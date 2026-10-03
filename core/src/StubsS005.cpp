// TEMPORARY (S-004 only): stubs for the S-005 classes; deleted in S-005.
#include "cs/Dsp.h"
#include "cs/Errors.h"
namespace cs::dsp {
struct LossFilter::Impl {}; struct DispersionFilter::Impl {}; struct WaveguideString::Impl {};
LossFilter::LossFilter() = default; LossFilter::~LossFilter() = default;
void LossFilter::design(double, double, double, double, double) { throw NotImplemented("LossFilter::design"); }
double LossFilter::magnitudeAt(double) const { throw NotImplemented("LossFilter::magnitudeAt"); }
double LossFilter::targetMagnitudeAt(double) const { throw NotImplemented("LossFilter::targetMagnitudeAt"); }
float LossFilter::process(float x) noexcept { return x; } void LossFilter::reset() noexcept {}
DispersionFilter::DispersionFilter() = default; DispersionFilter::~DispersionFilter() = default;
void DispersionFilter::design(double, double, double, int) { throw NotImplemented("DispersionFilter::design"); }
double DispersionFilter::phaseDelaySamples(double) const { throw NotImplemented("DispersionFilter::phaseDelaySamples"); }
std::vector<double> DispersionFilter::coefficients() const { throw NotImplemented("DispersionFilter::coefficients"); }
float DispersionFilter::process(float x) noexcept { return x; } void DispersionFilter::reset() noexcept {}
WaveguideString::WaveguideString() = default; WaveguideString::~WaveguideString() = default;
void WaveguideString::prepare(double, double) { throw NotImplemented("WaveguideString::prepare"); }
void WaveguideString::configure(const StringSpec&) { throw NotImplemented("WaveguideString::configure"); }
void WaveguideString::setFrequency(double) { throw NotImplemented("WaveguideString::setFrequency"); }
void WaveguideString::pluck(double, double) { throw NotImplemented("WaveguideString::pluck"); }
float WaveguideString::tick(float) noexcept { return 0.0f; }
std::vector<double> WaveguideString::lossCoefficientsMagnitude(int) const { throw NotImplemented("WaveguideString::lossCoefficientsMagnitude"); }
}
