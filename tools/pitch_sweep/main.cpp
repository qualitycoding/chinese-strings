// pitch_sweep — plays every note of the bowed instruments (default parameters, velocity 100) and reports where the settled oscillation lands.
// Not part of the frozen suite; used for the G-003a report.   usage: pitch_sweep [fs]
#include "Analysis.h"
#include "EngineHarness.h"
#include "cs/Engine.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace cs; using namespace harness;
int main(int argc, char** argv) {
    const double fs = argc > 1 ? std::atof(argv[1]) : 48000.0;
    for (int i = 0; i < kNumInstruments; ++i) {
        const auto id = static_cast<InstrumentId>(i); const auto& s = spec(id); if (s.family != Family::Bowed) continue;
        double sum = 0, worst = 0; int cnt = 0, out3 = 0; std::string bad;
        for (int n = s.midiLow; n <= s.midiHigh; ++n) {
            const double f = 440.0 * std::pow(2.0, (n - 69) / 12.0); if (f > 0.4 * fs) break;
            Engine e; e.prepare(fs, 256); e.setInstrument(id); noteOn(e, 1, n, 100); const auto y = render(e, fs, 1.5, 256);
            const double c = cstest::cents(cstest::peakFrequency(y, fs, (std::size_t) (0.5 * fs), (std::size_t) (0.9 * fs), f * 0.9, f * 1.1), f);
            sum += c; ++cnt; if (std::fabs(c) > std::fabs(worst)) worst = c;
            if (std::fabs(c) > 3.0) { ++out3; char b[48]; std::snprintf(b, sizeof b, " %d(%+.0f)", n, c); bad += b; }
        }
        std::printf("%-9s notes %2d  mean %+6.2f c  worst %+7.2f c  outside +-3 c: %2d%s\n", s.key.data(), cnt, sum / cnt, worst, out3, bad.c_str());
    }
}
