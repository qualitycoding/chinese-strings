#include "cs/Engine.h"
#include "cs/Instruments.h"
#include <cmath>
namespace cs {
double inharmonicityB(const StringSpec& s) {
    const double pi3 = M_PI * M_PI * M_PI;
    return pi3 * s.youngsModulusPa * std::pow(s.radiusM, 4) / (4.0 * s.tensionN * s.vibratingLengthM * s.vibratingLengthM);
}
bool supportsTechnique(InstrumentId id, Technique t) {
    if (t == Technique::Default) return true;
    switch (spec(id).family) {
        case Family::Bowed:       return t == Technique::Tremolo || t == Technique::Harmonic || t == Technique::Pizzicato || t == Technique::Glissando;
        case Family::PluckedLute: return t == Technique::Tremolo || t == Technique::Harmonic || t == Technique::Muted || t == Technique::Glissando;
        case Family::Zither:      return t == Technique::Tremolo || t == Technique::Harmonic || t == Technique::Muted || t == Technique::Glissando;
        case Family::Struck:      return t == Technique::Muted || t == Technique::Roll;
    }
    return false;
}
}
