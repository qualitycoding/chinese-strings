// Bowed string voice (S-008): nut loop (fractional delay + dispersion + loss FIR) and bridge loop (integer delay) joined at a
// memoryless bow-string scattering junction (Smith, PASP; C-062). Loop closure: nut = -chain(v_nl-), bridge = -delay(v_br-).
#include "Voices.h"
#include <algorithm>
#include <cmath>
namespace cs::detail {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr int kTab = 256; constexpr double kVmax = 3.0;
}
class BowedVoice final : public Voice {
public:
    void prepare(double fs_, const InstrumentSpec& sp, std::shared_ptr<const Designs> d) override {
        fs = fs_; spec = &sp; ds = std::move(d);
        double lowest = 1e9; for (const auto& s : sp.strings) lowest = std::min(lowest, s.openHz);
        nut.prepare(fs, 0.5 * lowest);
        std::size_t n = 8; const std::size_t need = (std::size_t) (0.35 * fs / (0.5 * lowest)) + 16; while (n < need) n <<= 1;
        bridge.assign(n, 0.0f); bmask = n - 1; duckInc = (float) (1.0 / (0.025 * fs)); relCoef = (float) std::exp(-1.0 / (0.12 * fs)); reset();
    }
    bool supportsLegato() const override { return true; }
    void noteOn(int n, double hz_, float vel, const VoiceParams& p) override {
        reset(); start(n, hz_, vel, p, true); active = true;
    }
    void legato(int n, double hz_, float vel, const VoiceParams& p) override {
        const double soundingNow = hz * std::pow(2.0, glide / 1200.0); start(n, hz_, vel, p, false);
        glide = 1200.0 * std::log2(soundingNow / hz_); active = true; releasing = false; relGain = 1.0f; bowOn = true;
        duck = 0.0f;   // the loop taps change at a legato note change: hide the discontinuity under a 25 ms smoothstep (S-008)
    }
    void release() override { releasing = true; bowOn = false; }
    void setHz(double h) override { hz = std::min(std::max(h, 8.0), 0.45 * fs); retune(hz * std::pow(2.0, glide / 1200.0) / (nd ? nd->lockFactor : 1.0)); }
    void render(float* out, int n, const VoiceParams& p) noexcept override {
        updateBow(p);
        const float glideDecay = (float) std::exp(-(double) n / (0.020 * fs));
        for (int i = 0; i < n; ++i) {
            const float ramp = attack < 1.0f ? attack : 1.0f; if (attack < 1.0f) attack += attackInc;
            const float target = bowOn ? vbTarget * ramp * ramp * (3.0f - 2.0f * ramp) : 0.0f;
            vb += (target - vb) * (bowOn ? 0.2f : 0.002f);
            const float vnlPlus = -nut.chain(nutIn);
            bridge[bw & bmask] = brIn; const float vbrPlus = -brLoss.process(bridge[(bw - (std::size_t) dbLine) & bmask]); ++bw;
            const float vDelta = vb - (vnlPlus + vbrPlus);
            float x = (vDelta + (float) kVmax) * (float) (kTab / (2.0 * kVmax)); x = std::min(std::max(x, 0.0f), (float) kTab - 0.001f);
            const int k = (int) x; const float fr = x - (float) k; const float rho = tab[k] + fr * (tab[k + 1] - tab[k]);
            const float vbrMinus = vnlPlus + rho * vDelta, vnlMinus = vbrPlus + rho * vDelta;
            nutIn = vnlMinus; brIn = vbrMinus;
            float dg = 1.0f; if (duck < 1.0f) { dg = duck * duck * (3.0f - 2.0f * duck); duck += duckInc; }
            out[i] += kOut * vbrMinus * relGain * dg;
            if (releasing) relGain *= relCoef;
        }
        if (glide != 0.0) { glide *= (double) glideDecay; if (std::fabs(glide) < 1e-3) glide = 0.0; }
        if (releasing && relGain < 1e-4f) { active = false; releasing = false; }
    }
    void reset() noexcept override {
        nut.reset(); brLoss.reset(); std::fill(bridge.begin(), bridge.end(), 0.0f); bw = 0; nutIn = brIn = 0; vb = 0; attack = 1.0f; relGain = 1.0f;
        glide = 0.0; duck = 1.0f; bowOn = false; active = false; releasing = false; lastF = -1; lastZ = -1;
    }
private:
    void start(int n, double hz_, float vel, const VoiceParams& p, bool fresh) {
        nd = &ds->at(n); stringIdx = nd->stringIdx; stop = nd->stop; const StringSpec& st = spec->strings[(std::size_t) stringIdx];
        const double rhoL = st.densityKgM3 * kPi * st.radiusM * st.radiusM; Z = std::sqrt(st.tensionN * rhoL);
        db = nd->Db; dbLine = std::max(1, nd->dbLine); brLoss.d = nd->bridgeLoss; nut.apply(nd->d, nd->lineDelay); curHz = nd->designHz;
        const double velScale = 0.4 + 0.6 * (double) vel;
        vbTarget = (float) ((0.04 + 0.20 * (double) p.bowSpeed) * velScale); bowOn = true; betaEff = (double) db / (fs / nd->designHz);
        if (fresh) { attack = 0.0f; attackInc = (float) (1.0 / (0.030 * fs)); }
        hz = std::min(std::max(hz_, 8.0), 0.45 * fs); retune(hz / nd->lockFactor);
        pressureNow = p.bowPressure; speedNow = p.bowSpeed; velNow = vel; rebuild(p);
    }
    void retune(double h) noexcept {
        if (!nd) return; if (std::fabs(std::log(h / curHz)) < 1e-6) return;
        const double T = fs / h - 1.0 - db - nd->d.dispDelay - nd->d.loss.M;
        nut.setLineDelay(lineDelayFor(std::max(T, 1.0), 2.0 * kPi * h / fs)); curHz = h;
    }
    void updateBow(const VoiceParams& p) noexcept {
        if (std::fabs(p.bowPressure - pressureNow) > 0.01f || std::fabs(p.bowSpeed - speedNow) > 0.01f) {
            pressureNow = p.bowPressure; speedNow = p.bowSpeed;
            vbTarget = (float) ((0.04 + 0.20 * (double) p.bowSpeed) * (0.4 + 0.6 * (double) velNow)); rebuild(p);
        }
    }
    void rebuild(const VoiceParams& p) noexcept {
        // Bow force follows Schelleng's scaling with the note's actual bow-to-bridge ratio: F = (Z v_b / beta_eff) (kForceLo + kForceSpan pressure)   [DD]
        const double F = (Z * (double) vbTarget / betaEff) * (kForceLo + kForceSpan * (double) p.bowPressure);
        if (std::fabs(F - lastF) < 0.01 * lastF && std::fabs(Z - lastZ) < 1e-9) return; lastF = F; lastZ = Z;
        const dsp::FrictionParams fp;
        for (int k = 0; k <= kTab; ++k) tab[k] = (float) dsp::BowJunction::reflectionCoefficient(-kVmax + 2.0 * kVmax * k / kTab, F, Z, fp);
    }
    double fs = 48000, curHz = 100, Z = 0.2, betaEff = 0.12, glide = 0, lastF = -1, lastZ = -1; const InstrumentSpec* spec = nullptr; std::shared_ptr<const Designs> ds; const NoteDesign* nd = nullptr;
    StringLoop nut; FirLoss brLoss; std::vector<float> bridge; std::size_t bmask = 7, bw = 0; int db = 2, dbLine = 1;
    float duck = 1.0f, duckInc = 0.001f, nutIn = 0, brIn = 0, vb = 0, vbTarget = 0.2f, attack = 1, attackInc = 0.001f, relGain = 1, relCoef = 0.99f, pressureNow = 0.5f, speedNow = 0.5f, velNow = 0.8f;
    float tab[kTab + 2] = {}; bool bowOn = false; static constexpr float kOut = 0.6f; static constexpr double kForceLo = 0.15, kForceSpan = 1.35;
};
std::unique_ptr<Voice> makeBowedVoice() { return std::make_unique<BowedVoice>(); }
}
