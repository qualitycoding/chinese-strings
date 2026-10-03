// STUBS — replaced during implementation (plan/PLAN.md). Every entry point throws cs::NotImplemented.
#include "cs/Dsp.h"
#include "cs/Engine.h"
#include "cs/EngineHost.h"
#include "cs/Errors.h"
#include "cs/Fingering.h"
#include "cs/Tuning.h"
#define CS_NI(name) throw ::cs::NotImplemented(name)
namespace cs {
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
}
