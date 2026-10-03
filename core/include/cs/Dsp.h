// Public interface (D-020): DSP building blocks with testable contracts.
#pragma once
#include "cs/Instruments.h"
#include <cstddef>
#include <vector>
namespace cs::dsp {
// Per-round-trip loop loss. Target magnitude at frequency f (Hz):
//   |H(f)| = exp(-(sigma0 + sigma1 * (2*pi*f/c)^2) / f0),  c = 2 * L * f0   (wave speed)
class LossFilter {
public:
    void design(double sampleRate, double f0Hz, double sigma0, double sigma1, double lengthM);
    double magnitudeAt(double hz) const;       // of the realised filter
    double targetMagnitudeAt(double hz) const; // of the formula above
    float process(float x) noexcept;
    void reset() noexcept;
private: std::vector<double> c_; std::vector<double> z_; double fs_ = 0, f0_ = 0, s0_ = 0, s1_ = 0, c_wave_ = 0;
};
// Chain of first-order allpasses approximating stiff-string dispersion for coefficient B.
class DispersionFilter {
public:
    void design(double sampleRate, double f0Hz, double B, int numSections = 4);
    double phaseDelaySamples(double hz) const;
    std::vector<double> coefficients() const;  // each |a| < 1 required
    float process(float x) noexcept;
    void reset() noexcept;
private: std::vector<double> a_, x1_, y1_; double fs_ = 0;
};
class FractionalDelayLine {
public:
    void prepare(int maxDelaySamples);
    void setDelay(double samples);             // >= 1.0
    float process(float x) noexcept;
    void reset() noexcept;
private: std::vector<float> buf_; int w_ = 0; double d_ = 1; double a_ = 0; float x1_ = 0, y1_ = 0;
};
// Bow friction (Smith PASP / MSW): mu(v) = muD + (muS-muD) * v0 / (v0 + |v|)
struct FrictionParams { double muS = 0.8, muD = 0.3, v0 = 0.2; };
struct BowJunction {
    // rho_hat in [0,1) for every finite input (C-062); bowForceN >= 0, waveImpedance > 0.
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
    void prepare(double sampleRate, double lowestHz);
    void configure(const StringSpec& s);       // sets dispersion/loss from the spec
    void setFrequency(double hz);              // fundamental incl. dispersion compensation
    void pluck(double position01, double amplitude);  // passive pluck excitation (C-064)
    float tick(float injectedVelocity) noexcept;      // returns bridge force (arbitrary units)
    std::vector<double> lossCoefficientsMagnitude(int gridPoints) const; // |H| on [0, fs/2]
private: std::vector<float> state_;
};
}
