// Internal: allocation-free design routines and the real-time string loop shared by WaveguideString and the voices.
#pragma once
#include "cs/Dsp.h"
#include <cmath>
#include <cstdint>
namespace cs::detail {
inline constexpr int kMaxFirM = 48;
inline constexpr int kMaxSections = 3;           // second-order allpass sections
struct LossDesign { int M = 0; double c[kMaxFirM + 1] = {1.0}; };
struct DispDesign { int K = 0; double a1[kMaxSections] = {}, a2[kMaxSections] = {}; };
struct StringDesign { double hz = 0; LossDesign loss; DispDesign disp; double dispDelay = 0; };

double lossTarget(double s0, double s1, double lengthM, double f0, double f);
// Weighted least-squares zero-phase symmetric FIR (S-08/S-08b, C-068). Chooses the smallest M <= Mmax with max relative ln error <= tol.
void designLoss(double fs, double f0, double s0, double s1, double lengthM, int Mmax, double tol, LossDesign& out);
double lossMagnitude(const LossDesign& d, double omega);
// Second-order allpass dispersion (K sections) fitted by Levenberg-Marquardt to the stiff-string phase-delay profile (partials 1..10).
void designDispersion(double fs, double f0, double B, int order, DispDesign& out);
double dispersionPhaseDelay(const DispDesign& d, double omega);
double thiranPhaseDelay(double a, double omega);
// Nominal FractionalDelayLine delay whose total phase delay (integer part + Thiran allpass) at omega equals `target`.
double lineDelayFor(double target, double omega);
// Complete design for a string of `spec` sounding at hz (effective length / stiffness scaled from the open string).
// extraDelay = unit delays in the loop outside the line (1 for the feedback register).  Returns the nominal line delay in *lineDelay.
void designString(double fs, const StringSpec& spec, double hz, int extraDelay, StringDesign& out, double* lineDelay, bool allowDispersion = true, double lossShare = 1.0);

// Zero-phase symmetric FIR loss realised causally (delay M); used for the bridge segment of bowed strings.
struct FirLoss {
    LossDesign d; float hist[128] = {}; unsigned pos = 0;
    float process(float x) noexcept {
        hist[pos & 127u] = x; const int M = d.M; double acc = d.c[0] * (double) hist[(pos - (unsigned) M) & 127u];
        for (int k = 1; k <= M; ++k) acc += d.c[k] * ((double) hist[(pos - (unsigned) M + (unsigned) k) & 127u] + (double) hist[(pos - (unsigned) M - (unsigned) k) & 127u]);
        ++pos; return (float) acc;
    }
    void reset() noexcept { for (auto& h : hist) h = 0.0f; pos = 0; }
};

// Real-time loop: unit register -> fractional delay -> dispersion -> loss FIR (zero-phase, delayed by M).
class StringLoop {
public:
    void prepare(double fs, double lowestHz);
    void apply(const StringDesign& d, double lineDelay);   // no allocation
    void setLineDelay(double d);                            // retune without touching the filters
    void reset() noexcept;
    float tick(float inj) noexcept;
    float chain(float x) noexcept;                          // delay+dispersion+loss only (used by bowed loops)
    int maxDelay() const noexcept { return maxDelay_; }
    const StringDesign& design() const noexcept { return d_; }
private:
    cs::dsp::FractionalDelayLine line_;
    StringDesign d_;
    double s1_[kMaxSections] = {}, s2_[kMaxSections] = {};
    float hist_[128] = {};
    unsigned pos_ = 0;
    float yprev_ = 0.0f;
    int maxDelay_ = 0;
};
}
