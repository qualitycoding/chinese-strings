// cs_render — offline, JUCE-free renderer used for the human listening gates (plan S-008).
//   cs_render --instrument <key> --phrase <file.json> [--fs 48000] [--out <file.wav>] [--normalize <peak dBFS>] [--quiet]
// Phrase file: {"name": "...", "length": seconds (optional), "params": {"bowPressure": 0.5, ...},
//   "events": [{"t": s, "type": "noteOn"|"noteOff"|"bend"|"cc"|"param", "note": 69 | "S1+4", "vel": 100, "ch": 1, "value": ..., "cc": 1, "name": "vibratoDepth"}]}
// A note may be a MIDI number or "S<k>[+/-n]": the open pitch of string k (rounded to a semitone) plus n semitones; events naming a string
// the instrument does not have are skipped, so one phrase file serves every instrument of a family.
#include "Json.h"
#include "Wav.h"
#include "cs/Engine.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
using namespace cs;
namespace {
struct Ev { double t; int order; std::uint8_t b[3]; int size; };
int resolveNote(const csr::Json& j, const InstrumentSpec& sp, bool& ok) {
    ok = true; if (j.t == csr::Json::T::Num) return (int) std::lround(j.n);
    if (j.t == csr::Json::T::Str && j.s.size() >= 2 && j.s[0] == 'S') {
        char* e = nullptr; const long k = std::strtol(j.s.c_str() + 1, &e, 10); long off = 0; if (*e == '+' || *e == '-') off = std::strtol(e, nullptr, 10);
        if (k < 0 || k >= (long) sp.strings.size()) { ok = false; return 0; }
        return (int) std::lround(69.0 + 12.0 * std::log2(sp.strings[(std::size_t) k].openHz / 440.0)) + (int) off;
    }
    throw std::runtime_error("bad note value");
}
}
int main(int argc, char** argv) {
    std::string inst, phrase, out = "out.wav"; double fs = 48000.0, normDb = 1e9; bool quiet = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i]; auto next = [&]() -> std::string { if (i + 1 >= argc) { std::fprintf(stderr, "missing value for %s\n", a.c_str()); std::exit(2); } return argv[++i]; };
        if (a == "--instrument") inst = next(); else if (a == "--phrase") phrase = next(); else if (a == "--out") out = next();
        else if (a == "--fs") fs = std::atof(next().c_str()); else if (a == "--normalize") normDb = std::atof(next().c_str()); else if (a == "--quiet") quiet = true;
        else { std::fprintf(stderr, "unknown argument %s\n", a.c_str()); return 2; }
    }
    if (inst.empty() || phrase.empty()) { std::fprintf(stderr, "usage: cs_render --instrument <key> --phrase <file.json> [--fs 48000] [--out file.wav] [--normalize <dBFS>]\n"); return 2; }
    const auto id = instrumentFromKey(inst); if (!id) { std::fprintf(stderr, "unknown instrument '%s'\n", inst.c_str()); return 2; }
    try {
        std::ifstream f(phrase); if (!f) throw std::runtime_error("cannot open " + phrase);
        std::stringstream ss; ss << f.rdbuf(); const csr::Json doc = csr::JsonParser(ss.str()).parse(); const auto& sp = spec(*id);
        Engine e; e.setInstrument(*id); e.prepare(fs, 512);
        if (doc.has("params")) for (const auto& kv : doc.at("params").o) {
            bool found = false; for (int p = 0; p < kNumParams; ++p) if (kv.first == paramKey(static_cast<ParamId>(p))) { e.setParameter(static_cast<ParamId>(p), (float) kv.second.n); found = true; }
            if (!found) throw std::runtime_error("unknown parameter " + kv.first);
        }
        std::vector<Ev> evs; double last = 0; int order = 0;
        if (doc.has("events")) for (const auto& j : doc.at("events").a) {
            const double t = j.num("t", 0.0); const std::string type = j.str("type", ""); const int ch = (int) j.num("ch", 1) - 1; Ev ev{t, order++, {0, 0, 0}, 3};
            if (type == "noteOn" || type == "noteOff") {
                bool ok = true; const int n = resolveNote(j.at("note"), sp, ok); if (!ok) continue;
                ev.b[0] = (std::uint8_t) ((type == "noteOn" ? 0x90 : 0x80) | ch); ev.b[1] = (std::uint8_t) std::min(std::max(n, 0), 127); ev.b[2] = (std::uint8_t) (type == "noteOn" ? (int) j.num("vel", 100) : 64);
            } else if (type == "bend") { const int v = std::min(16383, std::max(0, (int) std::lround(8192.0 + 8192.0 * j.num("value", 0.0)))); ev.b[0] = (std::uint8_t) (0xE0 | ch); ev.b[1] = (std::uint8_t) (v & 0x7F); ev.b[2] = (std::uint8_t) (v >> 7); }
            else if (type == "cc") { ev.b[0] = (std::uint8_t) (0xB0 | ch); ev.b[1] = (std::uint8_t) j.num("cc", 1); ev.b[2] = (std::uint8_t) j.num("value", 0); }
            else if (type == "param") { ev.size = 0; }
            else throw std::runtime_error("unknown event type '" + type + "'");
            if (type == "param") { ev.b[0] = 0xFF; ev.b[1] = 0; }       // handled below by index
            evs.push_back(ev); last = std::max(last, t);
        }
        // parameter events carry their payload separately
        struct PEv { double t; ParamId p; float v; }; std::vector<PEv> pevs;
        if (doc.has("events")) for (const auto& j : doc.at("events").a) if (j.str("type", "") == "param") {
            const std::string nm = j.str("name", ""); bool found = false;
            for (int p = 0; p < kNumParams; ++p) if (nm == paramKey(static_cast<ParamId>(p))) { pevs.push_back({j.num("t", 0.0), static_cast<ParamId>(p), (float) j.num("value", 0.0)}); found = true; }
            if (!found) throw std::runtime_error("unknown parameter " + nm);
        }
        std::stable_sort(evs.begin(), evs.end(), [](const Ev& a, const Ev& b) { return a.t < b.t; });
        std::stable_sort(pevs.begin(), pevs.end(), [](const PEv& a, const PEv& b) { return a.t < b.t; });
        const double length = doc.num("length", last + 2.5); const std::size_t total = (std::size_t) std::llround(length * fs);
        std::vector<float> L(total, 0.0f), R(total, 0.0f); std::size_t ei = 0, pi = 0;
        for (std::size_t pos = 0; pos < total;) {
            const std::size_t n = std::min<std::size_t>(512, total - pos);
            while (pi < pevs.size() && (std::size_t) std::llround(pevs[pi].t * fs) < pos + n) { e.setParameter(pevs[pi].p, pevs[pi].v); ++pi; }       // parameter changes land on block boundaries
            while (ei < evs.size() && (std::size_t) std::llround(evs[ei].t * fs) < pos + n) {
                if (evs[ei].size > 0) { const std::size_t at = (std::size_t) std::llround(evs[ei].t * fs); e.handleMidi(evs[ei].b, 3, (int) (at > pos ? at - pos : 0)); } ++ei;
            }
            e.render(L.data() + pos, R.data() + pos, (int) n); pos += n;
        }
        double peak = 0, sq = 0; for (std::size_t i = 0; i < total; ++i) { peak = std::max(peak, (double) std::fabs(L[i])); sq += (double) L[i] * L[i]; }
        const double rms = total ? std::sqrt(sq / (double) total) : 0.0; double gain = 1.0;
        if (normDb < 1e8 && peak > 1e-9) { gain = std::pow(10.0, normDb / 20.0) / peak; for (auto& x : L) x *= (float) gain; for (auto& x : R) x *= (float) gain; }
        if (!csr::writeWav24(out, L, R, (int) fs)) throw std::runtime_error("cannot write " + out);
        if (!quiet) std::printf("%s %s: %.2f s, raw peak %.1f dBFS, raw rms %.1f dBFS, gain applied %.1f dB -> %s\n", inst.c_str(), phrase.c_str(), length, 20 * std::log10(std::max(peak, 1e-12)), 20 * std::log10(std::max(rms, 1e-12)), 20 * std::log10(gain), out.c_str());
        return 0;
    } catch (const std::exception& ex) { std::fprintf(stderr, "cs_render: %s\n", ex.what()); return 1; }
}
