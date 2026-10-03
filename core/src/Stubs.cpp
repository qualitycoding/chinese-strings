// STUBS — replaced during implementation (plan/PLAN.md). Every entry point throws cs::NotImplemented.
#include "cs/Dsp.h"
#include "cs/Engine.h"
#include "cs/EngineHost.h"
#include "cs/Errors.h"
#include "cs/Fingering.h"
#include "cs/Tuning.h"
#define CS_NI(name) throw ::cs::NotImplemented(name)
namespace cs {
const InstrumentSpec& spec(InstrumentId) { CS_NI("spec"); }
std::optional<InstrumentId> instrumentFromKey(std::string_view) { CS_NI("instrumentFromKey"); }
double inharmonicityB(const StringSpec&) { CS_NI("inharmonicityB"); }
bool supportsTechnique(InstrumentId, Technique) { CS_NI("supportsTechnique"); }
ParamRange paramRange(ParamId) { CS_NI("paramRange"); }
const char* paramKey(ParamId) { CS_NI("paramKey"); }
TuningTable TuningTable::equal(double) { CS_NI("TuningTable::equal"); }
std::optional<TuningTable> loadScala(std::string_view, std::string_view) { CS_NI("loadScala"); }
Fingering fingeringFor(InstrumentId, double) { CS_NI("fingeringFor"); }
struct Engine::Impl {};
struct EngineHost::Impl {};
EngineHost::EngineHost() { CS_NI("EngineHost::EngineHost"); }
EngineHost::~EngineHost() = default;
void EngineHost::prepare(double, int) { CS_NI("EngineHost::prepare"); }
void EngineHost::requestInstrument(InstrumentId) { CS_NI("EngineHost::requestInstrument"); }
InstrumentId EngineHost::currentInstrument() const noexcept { return InstrumentId::Erhu; }
void EngineHost::setParameter(ParamId, float) noexcept {}
void EngineHost::setMpeEnabled(bool) noexcept {}
void EngineHost::handleMidi(const std::uint8_t*, int, int) noexcept {}
void EngineHost::render(float*, float*, int) noexcept {}
int EngineHost::voiceSnapshot(VoiceInfo*, int) const noexcept { return 0; }
void EngineHost::collectGarbage() { CS_NI("EngineHost::collectGarbage"); }
Engine::Engine() { CS_NI("Engine::Engine"); }
Engine::~Engine() = default;
void Engine::prepare(double, int) { CS_NI("Engine::prepare"); }
void Engine::setInstrument(InstrumentId) { CS_NI("Engine::setInstrument"); }
InstrumentId Engine::instrument() const noexcept { return InstrumentId::Erhu; }
void Engine::setParameter(ParamId, float) noexcept {}
float Engine::parameter(ParamId) const noexcept { return 0.0f; }
void Engine::setTuning(const TuningTable&) noexcept {}
void Engine::setMpeEnabled(bool) noexcept {}
void Engine::handleMidi(const std::uint8_t*, int, int) noexcept {}
void Engine::render(float*, float*, int) noexcept {}
Technique Engine::technique() const noexcept { return Technique::Default; }
int Engine::activeVoiceCount() const noexcept { return 0; }
int Engine::voiceInfo(VoiceInfo*, int) const noexcept { return 0; }
}
namespace cs::dsp {
void LossFilter::design(double, double, double, double, double) { CS_NI("LossFilter::design"); }
double LossFilter::magnitudeAt(double) const { CS_NI("LossFilter::magnitudeAt"); }
double LossFilter::targetMagnitudeAt(double) const { CS_NI("LossFilter::targetMagnitudeAt"); }
float LossFilter::process(float x) noexcept { return x; }
void LossFilter::reset() noexcept {}
void DispersionFilter::design(double, double, double, int) { CS_NI("DispersionFilter::design"); }
double DispersionFilter::phaseDelaySamples(double) const { CS_NI("DispersionFilter::phaseDelaySamples"); }
std::vector<double> DispersionFilter::coefficients() const { CS_NI("DispersionFilter::coefficients"); }
float DispersionFilter::process(float x) noexcept { return x; }
void DispersionFilter::reset() noexcept {}
void FractionalDelayLine::prepare(int) { CS_NI("FractionalDelayLine::prepare"); }
void FractionalDelayLine::setDelay(double) { CS_NI("FractionalDelayLine::setDelay"); }
float FractionalDelayLine::process(float x) noexcept { return x; }
void FractionalDelayLine::reset() noexcept {}
double BowJunction::reflectionCoefficient(double, double, double, const FrictionParams&) { CS_NI("BowJunction::reflectionCoefficient"); }
HuntCrossleyContact::HuntCrossleyContact(double K, double a, double b) : K_(K), alpha_(a), beta_(b) { CS_NI("HuntCrossleyContact"); }
double HuntCrossleyContact::force(double, double) const { CS_NI("HuntCrossleyContact::force"); }
void ModalBank::configure(double, const std::vector<ModeSpec>&) { CS_NI("ModalBank::configure"); }
float ModalBank::process(float x) noexcept { return x; }
void ModalBank::reset() noexcept {}
void WaveguideString::prepare(double, double) { CS_NI("WaveguideString::prepare"); }
void WaveguideString::configure(const StringSpec&) { CS_NI("WaveguideString::configure"); }
void WaveguideString::setFrequency(double) { CS_NI("WaveguideString::setFrequency"); }
void WaveguideString::pluck(double, double) { CS_NI("WaveguideString::pluck"); }
float WaveguideString::tick(float) noexcept { return 0.0f; }
std::vector<double> WaveguideString::lossCoefficientsMagnitude(int) const { CS_NI("WaveguideString::lossCoefficientsMagnitude"); }
}
