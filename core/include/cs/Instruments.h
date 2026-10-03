// Public interface (D-020). Instrument catalogue generated at build time from data/instruments/*.json (D-018).
#pragma once
#include <optional>
#include <string_view>
#include <vector>
namespace cs {
enum class Family { Bowed, PluckedLute, Zither, Struck };
enum class InstrumentId : int {
    Erhu = 0, Yehu, Gaohu, Zhonghu, Banhu, Jinghu, Sihu, Matouqin,        // Bowed (8)
    Pipa, Liuqin, Yueqin, Xiaoruan, Zhongruan, Daruan, Sanxian,         // PluckedLute (7)
    Guzheng, Guqin, Konghou,                                            // Zither (3)
    Yangqin                                                             // Struck (1)
};
inline constexpr int kNumInstruments = 19;
enum class StringMaterial { Steel, Silk, Nylon, SilkWoundSteel };
struct StringSpec {
    double openHz;             // nominal open-string fundamental (ideal-string, before stiffness)
    double vibratingLengthM;   // nut/qianjin-to-bridge
    double tensionN;
    double densityKgM3;
    double radiusM;
    double youngsModulusPa;
    double sigma0;             // 1/s   (frequency-independent loss, DAFx26 eq. 2)
    double sigma1;             // m^2/s (frequency-dependent loss, DAFx26 eq. 2)
    StringMaterial material;
    int courses;               // strings per course (yangqin >1), else 1
    double courseDetuneCents;  // max detune between course members
};
struct ModeSpec { double freqHz; double t60s; double gain; };
struct InstrumentSpec {
    InstrumentId id;
    Family family;
    std::string_view key;                 // "erhu", "guzheng", ...
    std::string_view displayName;
    int maxPolyphony;                     // realistic polyphony (A-010)
    bool stopsAllStringsTogether;         // huqin: both strings stopped together (C-048)
    std::vector<StringSpec> strings;      // ordered low -> high open pitch
    std::vector<double> fretSemitones;    // empty = fretless; else semitone offsets of frets from open
    std::vector<ModeSpec> bodyModes;
    std::vector<ModeSpec> radiation;
    int midiLow, midiHigh;                // playable range
    std::vector<std::string_view> citations; // claim IDs (C-###) / decision IDs backing the data
};
const InstrumentSpec& spec(InstrumentId id);
std::optional<InstrumentId> instrumentFromKey(std::string_view key);
// Fletcher stiff-string coefficient: B = pi^3 E r^4 / (4 T L^2); partial n at n*f0*sqrt(1+B n^2).
double inharmonicityB(const StringSpec& s);
enum class Technique;
// D-017 support table; unsupported techniques fall back to Technique::Default in the engine.
bool supportsTechnique(InstrumentId id, Technique t);
}
