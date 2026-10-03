#include "Voices.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
namespace cs::detail {
namespace { constexpr bool kBowedVoiceEnabled = true; constexpr int kLockPartials = 7; }
bool usesBowedVoice(Family f);

// Oscillation frequency of a freshly bowed note, from the autocorrelation at ~4 periods (parabolic interpolation). Returns <= 0 if not periodic.
static double measureOscillation(const std::vector<float>& y, double fs, double expectedHz, std::size_t from, double* periodicity = nullptr) {
    const double P = fs / expectedHz; const std::size_t n = y.size() - from; if (n < (std::size_t) (9 * P)) return -1.0;
    double mean = 0; for (std::size_t i = 0; i < n; ++i) mean += y[from + i]; mean /= (double) n;
    const int span = 4, lo = (int) std::floor(span * P * 0.97), hi = (int) std::ceil(span * P * 1.03);
    auto ac = [&](int L) { double c = 0; for (std::size_t i = 0; i + (std::size_t) L < n; ++i) c += ((double) y[from + i] - mean) * ((double) y[from + i + (std::size_t) L] - mean); return c; };
    double e0 = 0; for (std::size_t i = 0; i < n; ++i) e0 += ((double) y[from + i] - mean) * ((double) y[from + i] - mean); if (!(e0 > 1e-12)) return -1.0;
    int best = lo; double bc = -1e300; std::vector<double> c((std::size_t) (hi - lo + 1));
    for (int L = lo; L <= hi; ++L) { c[(std::size_t) (L - lo)] = ac(L); if (c[(std::size_t) (L - lo)] > bc) { bc = c[(std::size_t) (L - lo)]; best = L; } }
    if (periodicity) *periodicity = bc / e0;
    if (bc / e0 < 0.3 || best <= lo || best >= hi) return -1.0;
    const double a = c[(std::size_t) (best - 1 - lo)], b = c[(std::size_t) (best - lo)], d = c[(std::size_t) (best + 1 - lo)], den = a - 2 * b + d;
    const double L = (double) best + (den != 0.0 ? 0.5 * (a - d) / den : 0.0); return span * fs / L;
}

// Default bridge-segment length: the bow stays at a fixed distance from the bridge (follows the OPEN string), limited to 0.3 P of the stopped note,
// then nudged so P/Db stays away from an integer (a bow at ~1/n of the sounding length puts the bridge-segment mode on harmonic n and breaks the motion).
static int defaultDb(const NoteDesign& nd, const StringSpec& st, double fs) {
    const double P = fs / (nd.hz12 > 0 ? nd.hz12 : 100.0);
    const int db0 = std::max(4, std::min((int) std::lround(kBowPosition * fs / st.openHz), (int) std::lround(0.3 * P))); int best = db0; double bestScore = -1.0;
    for (int cand = std::max(4, db0 - 3); cand <= std::min(db0 + 3, std::max(db0, (int) std::lround(0.3 * P))); ++cand) {
        const double q = P / cand, score = std::fabs(q - std::round(q)) - 0.03 * std::abs(cand - db0); if (score > bestScore) { bestScore = score; best = cand; }
    }
    return best;
}
// (Re)builds the whole bowed design of one note for bridge-segment length db. Resets lockFactor to its analytic starting value.
static void buildBowedNote(NoteDesign& nd, const StringSpec& st, double fs, int db, bool dispersion = true) {
    const double hz = std::min(nd.hz12, 0.45 * fs);
    // A bowed dispersive string mode-locks to ~ the mean of its stretched partial frequencies f_n/n, not to the phase-delay fundamental:
    // design the loop flat by that factor so the *oscillation* lands on the note (measured in the S-008 pitch sweep; refined by calibrateBowed).
    const double B = dispersion ? inharmonicityB(st) * (hz / st.openHz) * (hz / st.openHz) : 0.0; const int N = std::max(1, std::min(kLockPartials, (int) (0.45 * fs / hz)));
    double sum = 0; for (int n = 1; n <= N; ++n) sum += std::sqrt((1.0 + B * n * n) / (1.0 + B));
    nd.lockFactor = sum / N; nd.designHz = std::min(hz / nd.lockFactor, 0.45 * fs);
    const double P = fs / nd.designHz; nd.Db = db;
    // Loss is shared between the two segments in proportion to their length (otherwise the bridge segment is lossless while the bow sticks).
    const double share = (double) nd.Db / P, ratio = nd.designHz / st.openHz, Leff = st.vibratingLengthM / ratio;
    designLoss(fs, nd.designHz, st.sigma0 * share, st.sigma1 * share, Leff, std::min(6, nd.Db - 3), 0.03, nd.bridgeLoss);
    nd.dbLine = nd.Db - 1 - nd.bridgeLoss.M;
    designString(fs, st, nd.designHz, 1 + nd.Db, nd.d, &nd.lineDelay, dispersion, 1.0 - share);
}
// Closed-loop trim of the lock factor: simulate the real voice, measure where the settled oscillation lands, correct (S-008).
// Returns the remaining |error| in cents, or 1e9 if the note never became periodic.
static double calibrateNote(Voice& v, NoteDesign& nd, int m, double fs, std::vector<float>& y) {
    const double P = fs / nd.hz12; const std::size_t total = (std::size_t) std::max(1.2 * fs, 14.0 * P), skip = (std::size_t) std::max(0.6 * fs, 4.0 * P);   // the oscillation keeps drifting for ~0.5 s
    auto measure = [&](float pressure, double& pc) {                              // ratio of settled oscillation to the note frequency, or -1 if not periodic
        VoiceParams vp; vp.bowPressure = pressure; v.reset(); v.note = m; v.noteOn(m, nd.hz12, 100.0f / 127.0f, vp); y.assign(total, 0.0f);
        for (std::size_t i = 0; i < total; i += 256) v.render(y.data() + i, (int) std::min<std::size_t>(256, total - i), vp);
        pc = 0.0; const double f = measureOscillation(y, fs, nd.hz12, skip, &pc); return f <= 0.0 ? -1.0 : f / nd.hz12;
    };
    double r = 0.0, pc = 0.0;
    for (int it = 0; it < 7; ++it) {                                               // trim at the default pressure
        r = measure(0.5f, pc); if (r <= 0.0) return 1e9;
        const double c = std::fabs(std::log(r)) * 1731.234; if (c < 0.4 || it == 6) break;
        nd.lockFactor *= (it < 3 ? r : std::sqrt(r));
    }
    // Score = worst pitch error (cents) plus penalties for a messy (quasi-periodic) motion: clean Helmholtz motion has periodicity > 0.85.
    double score = std::fabs(std::log(r)) * 1731.234 + 40.0 * std::max(0.0, 0.85 - pc);
    for (const float pressure : {0.25f, 0.8f}) {                                   // the note must also stay periodic and close in pitch for lighter / heavier bowing
        double q = 0.0; const double rq = measure(pressure, q); if (rq <= 0.0) return 1e9;
        score = std::max(score, 0.5 * std::fabs(std::log(rq)) * 1731.234 + 20.0 * std::max(0.0, 0.6 - q));
    }
    return score;
}
static void calibrateBowed(Designs& ds, const InstrumentSpec& sp, double fs) {
    std::shared_ptr<const Designs> view(std::shared_ptr<void>(), &ds);          // non-owning alias of the table being built
    auto v = makeBowedVoice(); v->prepare(fs, sp, view); std::vector<float> y; std::vector<char> ok(ds.notes.size(), 0);
    for (int m = sp.midiLow; m <= sp.midiHigh; ++m) {
        NoteDesign& nd = ds.notes[(std::size_t) (m - sp.midiLow)]; if (nd.hz12 > 0.4 * fs) continue;
        const StringSpec& st = sp.strings[(std::size_t) nd.stringIdx];
        double res = calibrateNote(*v, nd, m, fs, y);
        if (res > 2.5) {                                                           // try other bow-to-bridge distances and keep the best (periodic, smallest error)
            const int db0 = nd.Db; NoteDesign best = nd; double bestRes = res; const double P = fs / nd.hz12; const int dbMax = std::max(4, (int) std::lround(0.3 * P));
            // Candidates: other bow distances with dispersion; then, if the note still will not lock (sharp in-band dispersion poles), without dispersion.
            for (int pass = 0; pass < 2 && bestRes > 2.5; ++pass) for (int k = (pass == 0 ? 1 : 0); k <= (pass == 0 ? 5 : 3) && bestRes > 2.5; ++k) for (int sgn : {1, -1}) {
                if (k == 0 && sgn < 0) continue;
                const int db = db0 + sgn * k; if (db < 4 || db > dbMax) continue;
                NoteDesign alt = nd; buildBowedNote(alt, st, fs, db, pass == 0); const double r2 = calibrateNote(*v, alt, m, fs, y);
                if (r2 < bestRes) { bestRes = r2; best = alt; if (bestRes <= 2.5) break; }
            }
            nd = best; res = bestRes; v->reset();
        }
        ok[(std::size_t) (m - sp.midiLow)] = res < 1e8 ? 1 : 0;      // keep the best candidate unless the note never produced a periodic tone at all
    }
    for (std::size_t i = 0; i < ds.notes.size(); ++i) {                         // never periodic: borrow the nearest calibrated note on the same string
        if (ok[i] || ds.notes[i].hz12 > 0.4 * fs) continue; std::size_t best = i; int bd = 1 << 30;
        for (std::size_t j = 0; j < ds.notes.size(); ++j) if (ok[j] && ds.notes[j].stringIdx == ds.notes[i].stringIdx && std::abs((int) j - (int) i) < bd) { bd = std::abs((int) j - (int) i); best = j; }
        if (best != i) ds.notes[i].lockFactor = ds.notes[best].lockFactor;
    }
}

std::shared_ptr<const Designs> designsFor(InstrumentId id, double fs) {
    static std::mutex mtx; static std::map<std::pair<int, long long>, std::shared_ptr<const Designs>> cache;
    const auto key = std::make_pair((int) id, std::llround(fs * 1000.0));
    { std::lock_guard<std::mutex> g(mtx); auto it = cache.find(key); if (it != cache.end()) return it->second; }
    const auto& sp = spec(id); auto ds = std::make_shared<Designs>(); ds->midiLow = sp.midiLow;
    ds->notes.resize((std::size_t) (sp.midiHigh - sp.midiLow + 1));
    for (int m = sp.midiLow; m <= sp.midiHigh; ++m) {
        NoteDesign& nd = ds->notes[(std::size_t) (m - sp.midiLow)];
        nd.hz12 = 440.0 * std::pow(2.0, (m - 69) / 12.0);
        const Fingering f = fingeringFor(id, nd.hz12); nd.stringIdx = f.stringIndex; nd.stop = f.stopPosition01;
        const StringSpec& st = sp.strings[(std::size_t) f.stringIndex];
        const bool fixedPitch = sp.family == Family::Zither || sp.family == Family::Struck;     // string sounds at its open pitch; the voice bends
        nd.designHz = std::min(fixedPitch ? st.openHz : nd.hz12, 0.45 * fs);
        if (usesBowedVoice(sp.family)) {
            buildBowedNote(nd, st, fs, defaultDb(nd, st, fs));
        } else {
            designString(fs, st, nd.designHz, 1, nd.d, &nd.lineDelay);
        }
    }
    if (usesBowedVoice(sp.family)) calibrateBowed(*ds, sp, fs);
    std::lock_guard<std::mutex> g(mtx); auto& slot = cache[key]; if (!slot) slot = ds; return slot;
}
bool usesBowedVoice(Family f) { return f == Family::Bowed && kBowedVoiceEnabled; }
std::unique_ptr<Voice> makeVoice(Family f) { return usesBowedVoice(f) ? makeBowedVoice() : makePluckedVoice(); }
}
