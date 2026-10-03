// Internal: per-note designs and the voice interface used by Engine.
#pragma once
#include "StringDesign.h"
#include "cs/Engine.h"
#include "cs/Fingering.h"
#include <memory>
#include <vector>
namespace cs::detail {
struct NoteDesign {
    int stringIdx = 0; double stop = 0, hz12 = 0, designHz = 0;
    StringDesign d; double lineDelay = 1.0; int Db = 0;     // Db: bridge-side loop delay (bowed voices)
};
struct Designs {
    int midiLow = 0; std::vector<NoteDesign> notes;
    const NoteDesign& at(int midi) const { return notes[(std::size_t) (midi - midiLow)]; }
};
// Cached per (instrument, sample rate); built on the control thread (allocates, takes a mutex).
std::shared_ptr<const Designs> designsFor(InstrumentId id, double fs);
inline constexpr double kBowPosition = 0.12;                // bow-to-bridge distance as fraction of the string (DD)

struct VoiceParams { float bowPressure = 0.5f, bowSpeed = 0.5f, pluckPosition = 0.12f, brightness = 0.5f, pressure = 0.0f; Technique tech = Technique::Default; };

class Voice {
public:
    virtual ~Voice() = default;
    virtual void prepare(double fs, const InstrumentSpec& spec, std::shared_ptr<const Designs> designs) = 0;
    virtual void noteOn(int note, double hz, float velocity, const VoiceParams& p) = 0;
    virtual bool supportsLegato() const { return false; }
    virtual void legato(int note, double hz, float velocity, const VoiceParams& p) { noteOn(note, hz, velocity, p); }
    virtual void release() = 0;
    virtual void setHz(double hz) = 0;
    virtual void render(float* out, int n, const VoiceParams& p) noexcept = 0;   // adds into out
    virtual void reset() noexcept = 0;
    bool active = false, releasing = false;
    int note = -1, channel = 1, stringIdx = 0; double stop = 0, hz = 0; std::uint64_t age = 0;
};
std::unique_ptr<Voice> makeVoice(Family f);
std::unique_ptr<Voice> makePluckedVoice();
}
