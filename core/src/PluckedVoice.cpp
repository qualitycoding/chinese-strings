// Plucked string voice: waveguide loop + comb-filtered impulse excitation (plan S-007 placeholder for every family; S-010..S-012 specialise it).
#include "Voices.h"
#include <algorithm>
#include <cmath>
namespace cs::detail {
namespace { constexpr double kPi = 3.14159265358979323846; }
class PluckedVoice final : public Voice {
public:
    void prepare(double fs_, const InstrumentSpec& sp, std::shared_ptr<const Designs> d) override {
        fs = fs_; spec = &sp; ds = std::move(d);
        double lowest = 1e9; for (const auto& s : sp.strings) lowest = std::min(lowest, s.openHz);
        loop.prepare(fs, 0.5 * lowest); relCoef = (float) std::exp(-1.0 / (0.12 * fs)); reset();
    }
    void noteOn(int n, double hz_, float vel, const VoiceParams& p) override {
        nd = &ds->at(n); stringIdx = nd->stringIdx; stop = nd->stop; reset();
        loop.apply(nd->d, nd->lineDelay); curHz = nd->designHz; setHz(hz_);
        const double P = fs / curHz; pDp = std::min(std::max((int) std::lround((double) p.pluckPosition * P), 1), std::max(1, (int) P - 1));
        amp = (float) (0.12 + 0.55 * std::pow((double) vel / 127.0, 1.3)); pT = 0; plucking = true;
        lpc = (float) (0.9 * std::pow(1.0 - (double) p.brightness, 1.5)); relGain = 1.0f; releasing = false; active = true;
    }
    void release() override { releasing = true; }
    void setHz(double h) override {
        hz = std::min(std::max(h, 8.0), 0.45 * fs);
        if (!nd) return;
        if (std::fabs(std::log(hz / curHz)) < 1e-6) return;
        const double T = fs / hz - 1.0 - nd->d.dispDelay - nd->d.loss.M;
        loop.setLineDelay(lineDelayFor(std::max(T, 1.0), 2.0 * kPi * hz / fs)); curHz = hz;
    }
    void render(float* out, int n, const VoiceParams&) noexcept override {
        for (int i = 0; i < n; ++i) {
            float e = 0.0f;
            if (plucking) { if (pT == 0) e += amp; if (pT == pDp) { e -= amp; plucking = false; } ++pT; }
            lp = (1.0f - lpc) * e + lpc * lp;
            out[i] += (loop.tick(lp) + 0.35f * lp) * relGain;   // direct pick-attack transient + loop output
            if (releasing) relGain *= relCoef;
        }
        if (releasing && relGain < 1e-4f) { active = false; releasing = false; }
    }
    void reset() noexcept override { loop.reset(); lp = 0; plucking = false; relGain = 1.0f; active = false; releasing = false; }
private:
    double fs = 48000, curHz = 100; const InstrumentSpec* spec = nullptr; std::shared_ptr<const Designs> ds; const NoteDesign* nd = nullptr;
    StringLoop loop; float amp = 0, lp = 0, lpc = 0, relGain = 1, relCoef = 0.99f; int pT = 0, pDp = 1; bool plucking = false;
};
std::unique_ptr<Voice> makePluckedVoice() { return std::make_unique<PluckedVoice>(); }
}
