#include "cs/Fingering.h"
#include <cmath>
namespace cs {
// D-021: bowed / fretted lutes -> highest string whose open pitch <= target (lowest position), stop = 1 - open/target;
//        zithers / harp / yangqin -> nearest open string (log distance), stop 0.
Fingering fingeringFor(InstrumentId id, double targetHz) {
    const auto& s = spec(id); const int n = (int) s.strings.size();
    if (s.family == Family::Bowed || s.family == Family::PluckedLute) {
        int best = -1;
        for (int k = 0; k < n; ++k) if (s.strings[(std::size_t) k].openHz <= targetHz * (1.0 + 1e-9)) best = k;
        if (best < 0) return {0, 0.0};
        const double stop = 1.0 - s.strings[(std::size_t) best].openHz / targetHz;
        return {best, stop < 0.0 ? 0.0 : stop};
    }
    int best = 0; double bd = 1e300;
    for (int k = 0; k < n; ++k) { const double d = std::fabs(std::log(targetHz / s.strings[(std::size_t) k].openHz)); if (d < bd) { bd = d; best = k; } }
    return {best, 0.0};
}
}
