// 24-bit PCM stereo WAV writer.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
namespace csr {
inline bool writeWav24(const std::string& path, const std::vector<float>& l, const std::vector<float>& r, int sampleRate) {
    FILE* f = std::fopen(path.c_str(), "wb"); if (!f) return false;
    const std::uint32_t frames = (std::uint32_t) l.size(), dataBytes = frames * 2 * 3;
    auto w32 = [&](std::uint32_t v) { for (int i = 0; i < 4; ++i) std::fputc((int) ((v >> (8 * i)) & 0xFF), f); };
    auto w16 = [&](std::uint16_t v) { std::fputc(v & 0xFF, f); std::fputc(v >> 8, f); };
    std::fwrite("RIFF", 1, 4, f); w32(36 + dataBytes); std::fwrite("WAVEfmt ", 1, 8, f); w32(16); w16(1); w16(2); w32((std::uint32_t) sampleRate);
    w32((std::uint32_t) sampleRate * 6); w16(6); w16(24); std::fwrite("data", 1, 4, f); w32(dataBytes);
    auto s24 = [&](float x) { const double c = std::min(1.0, std::max(-1.0, (double) x)); const std::int32_t v = (std::int32_t) std::lround(c * 8388607.0); for (int i = 0; i < 3; ++i) std::fputc((v >> (8 * i)) & 0xFF, f); };
    for (std::uint32_t i = 0; i < frames; ++i) { s24(l[i]); s24(r[i]); }
    return std::fclose(f) == 0;
}
}
