// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
// Test-side signal analysis. Validated by tests/support_selftest (must pass before any requirement test is trusted).
#pragma once
#include <cstddef>
#include <vector>
namespace cstest {
// Hann-windowed, zero-padded (2^18) FFT peak search in [fLo,fHi] with parabolic interpolation on log magnitude.
double peakFrequency(const std::vector<float>& x, double fs, std::size_t start, std::size_t len, double fLo, double fHi);
double cents(double f, double ref);
// Pitch track: frames of `frame` samples, hop `hop`, peak in [fLo,fHi]. Returns Hz per frame.
std::vector<double> pitchTrack(const std::vector<float>& x, double fs, std::size_t frame, std::size_t hop, double fLo, double fHi);
// T60 of the component at freqHz: complex heterodyne, 3x cascaded 10 ms moving-average envelope, least-squares slope of dB
// envelope over [start, start+len). Returns seconds (inf if slope >= 0).
double componentT60(const std::vector<float>& x, double fs, double freqHz, std::size_t start, std::size_t len);
double rms(const std::vector<float>& x, std::size_t start, std::size_t len);
bool allFinite(const std::vector<float>& x);
float peakAbs(const std::vector<float>& x);
}
