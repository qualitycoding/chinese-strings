// Public interface (D-020, D-024). Thread-safe wrapper used by the plugin processor.
// Threads: "audio" = the host's processing thread; "control" = any other thread (message thread, UI, host parameter thread).
#pragma once
#include "cs/Engine.h"
#include <memory>
namespace cs {
class EngineHost {
public:
    EngineHost();
    ~EngineHost();
    void prepare(double sampleRate, int maxBlockSize);         // control thread; never concurrent with render(); throws like Engine::prepare
    void requestInstrument(InstrumentId id);                   // control thread; builds a new configured Engine off the audio thread,
                                                               // published to the audio thread at the next block boundary (lock-free)
    InstrumentId currentInstrument() const noexcept;           // any thread: instrument of the engine the audio thread is using
    void setParameter(ParamId p, float value) noexcept;        // any thread (atomic; applied at the next block)
    void setMpeEnabled(bool on) noexcept;                      // any thread
    void handleMidi(const std::uint8_t* bytes, int size, int sampleOffset) noexcept; // audio thread
    void render(float* left, float* right, int numSamples) noexcept;                  // audio thread; never allocates, locks or frees
    int voiceSnapshot(VoiceInfo* out, int maxCount) const noexcept; // any thread; consistent snapshot published by render()
    void collectGarbage();                                     // control thread; frees engines retired by the audio thread
private:
    struct Impl; std::unique_ptr<Impl> impl_;
};
}
