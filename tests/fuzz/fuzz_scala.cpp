// FROZEN — DO NOT MODIFY. libFuzzer target (T-100). Splits input at first 0x00: [scl]\0[kbm].
#include "cs/Tuning.h"
#include <cstdint>
#include <string_view>
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* d, std::size_t n) {
    std::string_view all(reinterpret_cast<const char*>(d), n); auto z = all.find('\0');
    (void) cs::loadScala(all.substr(0, z), z == std::string_view::npos ? std::string_view{} : all.substr(z + 1)); return 0;
}
