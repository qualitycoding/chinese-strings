// Public interface (D-020). Real-time contract: render() and handleMidi() never allocate, lock or throw (D-009).
#pragma once
#include "cs/Instruments.h"
#include "cs/Tuning.h"
#include <cstdint>
#include <memory>
namespace cs {
enum class Technique { Default = 0, Tremolo, Harmonic, Muted, Pizzicato, Glissando, Roll };
inline constexpr int kNumTechniques = 7;
inline constexpr int kKeyswitchLow = 12;   // MIDI notes 12..18 select Technique 0..6 (D-017)
enum class ParamId { Gain, BowPressure, BowSpeed, PluckPosition, Brightness, VibratoRate, VibratoDepth, PitchBendRange, FreePolyphony };
inline constexpr int kFreePolyphonyVoices = 16;
// Parameter table (D-020): natural units, clamped to [min,max].
struct ParamRange { float min, max, def; };
ParamRange paramRange(ParamId p);            // Gain 0..1 (0.8); BowPressure 0..1 (0.5); BowSpeed 0..1 (0.5); PluckPosition 0.02..0.5 (0.12);
                                             // Brightness 0..1 (0.5); VibratoRate 0..12 Hz (5.5); VibratoDepth 0..100 cents (0);
                                             // PitchBendRange 0..48 semitones (2); FreePolyphony 0..1 (0)
const char* paramKey(ParamId p);             // "gain","bowPressure","bowSpeed","pluckPosition","brightness","vibratoRate","vibratoDepth","pitchBendRange","freePolyphony"
inline constexpr int kNumParams = 9;
struct VoiceInfo { int midiNote; int channel; int stringIndex; double stopPosition01; double currentHz; };
class Engine {
public:
    Engine();
    ~Engine();
    void prepare(double sampleRate, int maxBlockSize);          // may allocate; throws std::invalid_argument unless 22050<=sampleRate<=192000 and 1<=maxBlockSize<=8192
    void setInstrument(InstrumentId id);                        // message thread; may allocate
    InstrumentId instrument() const noexcept;
    void setParameter(ParamId p, float value) noexcept;         // normalised/natural units per D-020 table
    float parameter(ParamId p) const noexcept;
    void setTuning(const TuningTable& t) noexcept;
    void setMpeEnabled(bool on) noexcept;                       // lower zone, master ch 1, members 2-16
    void handleMidi(const std::uint8_t* bytes, int size, int sampleOffset) noexcept;
    void render(float* left, float* right, int numSamples) noexcept;
    Technique technique() const noexcept;
    int activeVoiceCount() const noexcept;
    int voiceInfo(VoiceInfo* out, int maxCount) const noexcept;
private:
    struct Impl; std::unique_ptr<Impl> impl_;
};
}
