#include "cs/Engine.h"
namespace cs {
ParamRange paramRange(ParamId p) {
    switch (p) {
        case ParamId::Gain:           return {0.0f, 1.0f, 0.8f};
        case ParamId::BowPressure:    return {0.0f, 1.0f, 0.5f};
        case ParamId::BowSpeed:       return {0.0f, 1.0f, 0.5f};
        case ParamId::PluckPosition:  return {0.02f, 0.5f, 0.12f};
        case ParamId::Brightness:     return {0.0f, 1.0f, 0.5f};
        case ParamId::VibratoRate:    return {0.0f, 12.0f, 5.5f};
        case ParamId::VibratoDepth:   return {0.0f, 100.0f, 0.0f};
        case ParamId::PitchBendRange: return {0.0f, 48.0f, 2.0f};
        case ParamId::FreePolyphony:  return {0.0f, 1.0f, 0.0f};
    }
    return {0.0f, 1.0f, 0.0f};
}
const char* paramKey(ParamId p) {
    switch (p) {
        case ParamId::Gain:           return "gain";
        case ParamId::BowPressure:    return "bowPressure";
        case ParamId::BowSpeed:       return "bowSpeed";
        case ParamId::PluckPosition:  return "pluckPosition";
        case ParamId::Brightness:     return "brightness";
        case ParamId::VibratoRate:    return "vibratoRate";
        case ParamId::VibratoDepth:   return "vibratoDepth";
        case ParamId::PitchBendRange: return "pitchBendRange";
        case ParamId::FreePolyphony:  return "freePolyphony";
    }
    return "";
}
}
