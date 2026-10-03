// Internal (not installed): data hooks shared between generated tables and the engine.
#pragma once
#include "cs/Instruments.h"
#include <vector>
namespace cs::detail {
struct RadiationSection { double a[3]; double b[2]; };   // H_k(z) = (b0 + b1 z^-1) / (a0 + a1 z^-1 + a2 z^-2)
struct RadiationSpec {
    double designSampleRate;
    double fir;                                            // direct term: H(z) = fir + sum_k H_k(z)
    std::vector<RadiationSection> sections;
};
const RadiationSpec* radiationFor(InstrumentId id);       // nullptr when the instrument has no radiation filter
}
