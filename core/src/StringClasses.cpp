// Public cs::dsp classes built on the internal design routines (S-005).
#include "StringDesign.h"
#include <algorithm>
#include <cstring>
namespace cs::dsp {
using namespace cs::detail;
namespace { constexpr double kPi = 3.14159265358979323846; }

// ---------------- LossFilter ----------------
struct LossFilter::Impl { double fs = 48000, f0 = 100, s0 = 0, s1 = 0, L = 1; LossDesign d; float hist[128] = {}; unsigned pos = 0; };
LossFilter::LossFilter() : impl_(new Impl) {}
LossFilter::~LossFilter() = default;
void LossFilter::design(double fs, double f0, double s0, double s1, double L) {
    impl_->fs = fs; impl_->f0 = f0; impl_->s0 = s0; impl_->s1 = s1; impl_->L = L;
    designLoss(fs, f0, s0, s1, L, kMaxFirM, 0.03, impl_->d); reset();
}
double LossFilter::magnitudeAt(double hz) const { return lossMagnitude(impl_->d, 2.0 * kPi * hz / impl_->fs); }
double LossFilter::targetMagnitudeAt(double hz) const { return lossTarget(impl_->s0, impl_->s1, impl_->L, impl_->f0, hz); }
float LossFilter::process(float x) noexcept {
    auto& I = *impl_; I.hist[I.pos & 127u] = x; const int M = I.d.M;
    double acc = I.d.c[0] * (double) I.hist[(I.pos - (unsigned) M) & 127u];
    for (int k = 1; k <= M; ++k) acc += I.d.c[k] * ((double) I.hist[(I.pos - (unsigned) M + (unsigned) k) & 127u] + (double) I.hist[(I.pos - (unsigned) M - (unsigned) k) & 127u]);
    ++I.pos; return (float) acc;
}
void LossFilter::reset() noexcept { std::memset(impl_->hist, 0, sizeof(impl_->hist)); impl_->pos = 0; }

// ---------------- DispersionFilter ----------------
struct DispersionFilter::Impl { double fs = 48000; DispDesign d; double s1[kMaxSections] = {}, s2[kMaxSections] = {}; };
DispersionFilter::DispersionFilter() : impl_(new Impl) {}
DispersionFilter::~DispersionFilter() = default;
void DispersionFilter::design(double fs, double f0, double B, int numSections) {
    impl_->fs = fs; designDispersion(fs, f0, B, numSections, impl_->d); reset();
}
double DispersionFilter::phaseDelaySamples(double hz) const { return dispersionPhaseDelay(impl_->d, 2.0 * kPi * hz / impl_->fs); }
std::vector<double> DispersionFilter::coefficients() const {
    std::vector<double> c; for (int k = 0; k < impl_->d.K; ++k) { c.push_back(impl_->d.a1[k] / (1.0 + impl_->d.a2[k])); c.push_back(impl_->d.a2[k]); } return c;
}
float DispersionFilter::process(float x) noexcept {
    auto& I = *impl_; double u = x;
    for (int k = 0; k < I.d.K; ++k) { const double a1 = I.d.a1[k], a2 = I.d.a2[k], y = a2 * u + I.s1[k]; I.s1[k] = a1 * u - a1 * y + I.s2[k]; I.s2[k] = u - a2 * y; u = y; }
    return (float) u;
}
void DispersionFilter::reset() noexcept { for (int k = 0; k < kMaxSections; ++k) impl_->s1[k] = impl_->s2[k] = 0.0; }

// ---------------- WaveguideString ----------------
struct WaveguideString::Impl {
    double fs = 48000, lowest = 30, hz = 100; StringSpec spec{}; bool configured = false, prepared = false;
    StringLoop loop; double pluckAmp = 0; int pluckT = 0, pluckDp = 0; bool plucking = false;
};
WaveguideString::WaveguideString() : impl_(new Impl) {}
WaveguideString::~WaveguideString() = default;
void WaveguideString::prepare(double fs, double lowestHz) { impl_->fs = fs; impl_->lowest = lowestHz; impl_->loop.prepare(fs, lowestHz); impl_->prepared = true; impl_->plucking = false; }
void WaveguideString::configure(const StringSpec& s) { impl_->spec = s; impl_->configured = true; }
void WaveguideString::setFrequency(double hz) {
    auto& I = *impl_; if (!I.prepared || !I.configured) return;
    hz = std::min(std::max(hz, I.lowest), 0.45 * I.fs); I.hz = hz;
    StringDesign d; double ld = 1.0; designString(I.fs, I.spec, hz, 1, d, &ld); I.loop.apply(d, ld);
}
void WaveguideString::pluck(double position01, double amplitude) {
    auto& I = *impl_; const double P = I.fs / I.hz;
    I.pluckDp = std::min(std::max((int) std::lround(position01 * P), 1), std::max(1, (int) P - 1)); I.pluckAmp = amplitude; I.pluckT = 0; I.plucking = true;
}
float WaveguideString::tick(float injected) noexcept {
    auto& I = *impl_; float e = 0.0f;
    if (I.plucking) { if (I.pluckT == 0) e += (float) I.pluckAmp; if (I.pluckT == I.pluckDp) { e -= (float) I.pluckAmp; I.plucking = false; } ++I.pluckT; }
    return I.loop.tick(injected + e);
}
std::vector<double> WaveguideString::lossCoefficientsMagnitude(int n) const {
    std::vector<double> m((std::size_t) n + 1); for (int i = 0; i <= n; ++i) m[(std::size_t) i] = lossMagnitude(impl_->loop.design().loss, kPi * (double) i / n); return m;
}
}
