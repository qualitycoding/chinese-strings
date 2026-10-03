// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
#pragma once
#include "cs/Engine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>
namespace harness {
inline void noteOn(cs::Engine& e, int ch, int note, int vel, int off = 0) { std::uint8_t m[3] = {(std::uint8_t) (0x90 | (ch - 1)), (std::uint8_t) note, (std::uint8_t) vel}; e.handleMidi(m, 3, off); }
inline void noteOff(cs::Engine& e, int ch, int note, int off = 0) { std::uint8_t m[3] = {(std::uint8_t) (0x80 | (ch - 1)), (std::uint8_t) note, 0}; e.handleMidi(m, 3, off); }
inline void bend(cs::Engine& e, int ch, int value14, int off = 0) { std::uint8_t m[3] = {(std::uint8_t) (0xE0 | (ch - 1)), (std::uint8_t) (value14 & 0x7F), (std::uint8_t) ((value14 >> 7) & 0x7F)}; e.handleMidi(m, 3, off); }
// Renders `secs` seconds in blocks of `block`; returns left channel (right must equal or be finite; checked by tests that need it).
inline std::vector<float> render(cs::Engine& e, double fs, double secs, int block, std::vector<float>* right = nullptr) {
    std::size_t n = (std::size_t) (fs * secs); std::vector<float> L(n), R(n); std::vector<float> bl((std::size_t) block), br((std::size_t) block);
    for (std::size_t i = 0; i < n; i += (std::size_t) block) { int k = (int) std::min<std::size_t>((std::size_t) block, n - i); e.render(bl.data(), br.data(), k);
        for (int j = 0; j < k; ++j) { L[i + (std::size_t) j] = bl[(std::size_t) j]; R[i + (std::size_t) j] = br[(std::size_t) j]; } }
    if (right) *right = R; return L;
}
inline int midiFromHz(double hz) { return (int) std::lround(69.0 + 12.0 * std::log2(hz / 440.0)); }
}
