// Public interface (D-020) used by the UI highlighter (A-008, D-013).
#pragma once
#include "cs/Instruments.h"
namespace cs {
struct Fingering { int stringIndex; double stopPosition01; }; // stop 0 = open string; position as fraction of vibrating length from nut/qianjin
// Rule (D-021): bowed/lutes -> highest-pitched string whose open pitch <= target (lowest position);
// zithers/harp/yangqin -> string whose open pitch is nearest to target (stop 0; bends handled by voice).
// stopPosition01 = 1 - openHz/targetHz for stopped strings.
Fingering fingeringFor(InstrumentId id, double targetHz);
}
