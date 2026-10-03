// Public interface (D-020, D-010)
#pragma once
#include <array>
#include <optional>
#include <string_view>
namespace cs {
struct TuningTable {
    std::array<double, 128> hz{};
    static TuningTable equal(double a4Hz = 440.0);
};
// Limits (D-010): each text <= 65536 bytes, scale <= 1024 notes. kbm may be empty (=> standard mapping, 60 = degree 0 at 261.6256 Hz).
std::optional<TuningTable> loadScala(std::string_view sclText, std::string_view kbmText);
}
