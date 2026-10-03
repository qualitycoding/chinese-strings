#include "Voices.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
namespace cs::detail {
namespace { constexpr bool kBowedVoiceEnabled = false; }   // S-008 enables the bowed voice
bool usesBowedVoice(Family f);
std::shared_ptr<const Designs> designsFor(InstrumentId id, double fs) {
    static std::mutex mtx; static std::map<std::pair<int, long long>, std::shared_ptr<const Designs>> cache;
    const auto key = std::make_pair((int) id, std::llround(fs * 1000.0));
    { std::lock_guard<std::mutex> g(mtx); auto it = cache.find(key); if (it != cache.end()) return it->second; }
    const auto& sp = spec(id); auto ds = std::make_shared<Designs>(); ds->midiLow = sp.midiLow;
    ds->notes.resize((std::size_t) (sp.midiHigh - sp.midiLow + 1));
    for (int m = sp.midiLow; m <= sp.midiHigh; ++m) {
        NoteDesign& nd = ds->notes[(std::size_t) (m - sp.midiLow)];
        nd.hz12 = 440.0 * std::pow(2.0, (m - 69) / 12.0);
        const Fingering f = fingeringFor(id, nd.hz12); nd.stringIdx = f.stringIndex; nd.stop = f.stopPosition01;
        const StringSpec& st = sp.strings[(std::size_t) f.stringIndex];
        const bool fixedPitch = sp.family == Family::Zither || sp.family == Family::Struck;     // string sounds at its open pitch; the voice bends
        nd.designHz = std::min(fixedPitch ? st.openHz : nd.hz12, 0.45 * fs);
        if (usesBowedVoice(sp.family)) {
            const double P = fs / nd.designHz; nd.Db = std::max(2, (int) std::lround(kBowPosition * P));
            designString(fs, st, nd.designHz, 1 + nd.Db, nd.d, &nd.lineDelay);
        } else {
            designString(fs, st, nd.designHz, 1, nd.d, &nd.lineDelay);
        }
    }
    std::lock_guard<std::mutex> g(mtx); auto& slot = cache[key]; if (!slot) slot = ds; return slot;
}
bool usesBowedVoice(Family f) { return f == Family::Bowed && kBowedVoiceEnabled; }
std::unique_ptr<Voice> makeVoice(Family f) { (void) f; return makePluckedVoice(); }
}
