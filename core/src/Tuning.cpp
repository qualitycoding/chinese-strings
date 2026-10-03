#include "cs/Tuning.h"
#include "Tunings.h"      // surge-synthesizer/tuning-library @ 48422e2 (MIT), pinned in plan/deps.lock.json
#include <cmath>
#include <string>
namespace cs {
TuningTable TuningTable::equal(double a4Hz) {
    TuningTable t;
    for (int n = 0; n < 128; ++n) t.hz[(std::size_t) n] = n == 69 ? a4Hz : a4Hz * std::pow(2.0, (n - 69) / 12.0);
    return t;
}
// D-010 / D-010a: size limits, validation beyond the library, no exception ever escapes.
std::optional<TuningTable> loadScala(std::string_view sclText, std::string_view kbmText) {
    constexpr std::size_t kMaxBytes = 65536; constexpr int kMaxNotes = 1024;
    if (sclText.empty() || sclText.size() > kMaxBytes || kbmText.size() > kMaxBytes) return std::nullopt;
    try {
        const std::string scl(sclText), kbm(kbmText);
        const auto scale = Tunings::parseSCLData(scl);
        if (scale.count < 1 || scale.count > kMaxNotes || (int) scale.tones.size() != scale.count) return std::nullopt;
        for (const auto& tone : scale.tones) {
            if (!std::isfinite(tone.cents) || !std::isfinite(tone.floatValue) || !(tone.floatValue > 0.0)) return std::nullopt;
            if (tone.type == Tunings::Tone::kToneRatio && (tone.ratio_n <= 0 || tone.ratio_d <= 0)) return std::nullopt;
        }
        // Fuzzing finding (regression tests/fuzz/regressions/scala-ubsan-kbm-rotations-1): the library multiplies scale sizes by a
        // rotation count derived from the largest KBM key index, which overflows `int` for crafted maps. Bound every KBM field first.
        Tunings::KeyboardMapping km;
        if (!kbm.empty()) {
            km = Tunings::parseKBMData(kbm);
            auto in = [](int v, int lo, int hi) { return v >= lo && v <= hi; };
            if (!in(km.count, 0, kMaxNotes) || !in(km.firstMidi, 0, 127) || !in(km.lastMidi, 0, 127) || !in(km.middleNote, 0, 127) ||
                !in(km.tuningConstantNote, 0, 127) || !in(km.octaveDegrees, 0, kMaxNotes) || (int) km.keys.size() > kMaxNotes) return std::nullopt;
            if (!std::isfinite(km.tuningFrequency) || !(km.tuningFrequency > 0.0 && km.tuningFrequency < 1.0e5)) return std::nullopt;
            for (int key : km.keys) if (!in(key, -1, 8 * scale.count)) return std::nullopt;
        }
        const Tunings::Tuning tuning = kbm.empty() ? Tunings::Tuning(scale) : Tunings::Tuning(scale, km);
        TuningTable t;
        for (int n = 0; n < 128; ++n) {
            const double f = tuning.frequencyForMidiNote(n);
            if (!std::isfinite(f) || !(f > 0.0)) return std::nullopt;
            t.hz[(std::size_t) n] = f;
        }
        return t;
    } catch (...) {
        return std::nullopt;
    }
}
}
