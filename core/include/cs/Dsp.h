// Public interface (D-020): DSP building blocks with testable contracts.
// (S-005 note: private state is held behind pimpl pointers; no public signature differs from the frozen interface — see DEVIATIONS.md.)
#pragma once
#include "cs/Instruments.h"
#include <cstddef>
#include <memory>
#include <vector>
namespace cs::dsp {
// Per-round-trip loop loss. Target magnitude at frequency f (Hz):
//   |H(f)| = exp(-(sigma0 + sigma1 * (2*pi*f/c)^2) / f0),  c = 2 * L * f0   (wave speed)
class LossFilter {
public:
    LossFilter();
    ~LossFilter();
    void design(double sampleRate, double f0Hz, double sigma0, double sigma1, double lengthM);
    double magnitudeAt(double hz) const;       // of the realised filter
    double targetMagnitudeAt(double hz) const; // of the formula above
    float process(float x) noexcept;
    void reset() noexcept;
private:
    struct Impl; std::unique_ptr<Impl> impl_;
};
// Chain of allpass sections approximating stiff-string dispersion for coefficient B.
// numSections = filter order; pairs of orders form second-order sections (odd orders are rounded up).
class DispersionFilter {
public:
    DispersionFilter();
    ~DispersionFilter();
    void design(double sampleRate, double f0Hz, double B, int numSections = 4);
    double phaseDelaySamples(double hz) const;
    std::vector<double> coefficients() const;  // lattice reflection coefficients, each |k| < 1 required
    float process(float x) noexcept;
    void reset() noexcept;
private:
    struct Impl; std::unique_ptr<Impl> impl_;
};
class FractionalDelayLine {
public:
    void prepare(int maxDelaySamples);
    void setDelay(double samples);             // >= 1.0
    float process(float x) noexcept;
    void reset() noexcept;
private: std::vector<float> buf_; std::size_t mask_ = 0; std::size_t w_ = 0; int n_ = 0; double d_ = 1, a_ = 0, xp_ = 0, yp_ = 0;
};
// Bow friction (Smith PASP / MSW): mu(v) = muD + (muS-muD) * v0 / (v0 + |v|)
struct FrictionParams { double muS = 0.8, muD = 0.3, v0 = 0.2; };
struct BowJunction {
    // rho_hat in [0,1) for every finite input (C-062); bowForceN >= 0, waveImpedance > 0.
    // Convention: outgoing waves = incoming wave from the opposite side + rho_hat * vDeltaPlus; rho_hat -> 1 when the bow sticks.
    static double reflectionCoefficient(double vDeltaPlus, double bowForceN, double waveImpedance, const FrictionParams& p);
};
// Hunt-Crossley contact (DAFx26 eq. 4-5): f = K [x]+^alpha * (1 + beta * dx/dt); 0 when x <= 0.
class HuntCrossleyContact {
public:
    HuntCrossleyContact(double K, double alpha, double beta);
    double force(double penetrationM, double penetrationVelocity) const;
private: double K_, alpha_, beta_;
};
class ModalBank {
public:
    void configure(double sampleRate, const std::vector<ModeSpec>& modes);
    float process(float x) noexcept;
    void reset() noexcept;
private: std::vector<double> b0_, a1_, a2_, z1_, z2_;
};
// Digital-waveguide string (D-004): fractional delay + dispersion + loss, passive terminations.
class WaveguideString {
public:
    WaveguideString();
    ~WaveguideString();
    void prepare(double sampleRate, double lowestHz);
    void configure(const StringSpec& s);       // sets dispersion/loss from the spec
    void setFrequency(double hz);              // fundamental incl. dispersion compensation
    void pluck(double position01, double amplitude);  // passive pluck excitation (C-064)
    float tick(float injectedVelocity) noexcept;      // returns bridge force (arbitrary units)
    std::vector<double> lossCoefficientsMagnitude(int gridPoints) const; // |H| on [0, fs/2]
private:
    struct Impl; std::unique_ptr<Impl> impl_;
};
}
