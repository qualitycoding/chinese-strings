// FROZEN — DO NOT MODIFY. libFuzzer target (T-101).
#include "cs/StateCodec.h"
#include <cstdint>
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* d, std::size_t n) { (void) cs::decodeState(d, (int) n); return 0; }
