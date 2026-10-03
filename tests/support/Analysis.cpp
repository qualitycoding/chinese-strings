// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
#include "Analysis.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
namespace cstest {
namespace {
constexpr double kPi = 3.14159265358979323846;
void fft(std::vector<std::complex<double>>& a) {
    const std::size_t n = a.size();
    for (std::size_t i = 1, j = 0; i < n; ++i) { std::size_t bit = n >> 1; for (; j & bit; bit >>= 1) j ^= bit; j ^= bit; if (i < j) std::swap(a[i], a[j]); }
    for (std::size_t len = 2; len <= n; len <<= 1) {
        const double ang = -2.0 * kPi / (double) len; const std::complex<double> wl(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len) { std::complex<double> w(1.0, 0.0);
            for (std::size_t k = 0; k < len / 2; ++k) { auto u = a[i + k], v = a[i + k + len / 2] * w; a[i + k] = u + v; a[i + k + len / 2] = u - v; w *= wl; } }
    }
}
}
double peakFrequency(const std::vector<float>& x, double fs, std::size_t start, std::size_t len, double fLo, double fHi) {
    const std::size_t N = 1u << 18; len = std::min({len, x.size() > start ? x.size() - start : 0, N});
    std::vector<std::complex<double>> a(N, {0.0, 0.0});
    for (std::size_t i = 0; i < len; ++i) { double w = 0.5 - 0.5 * std::cos(2.0 * kPi * (double) i / (double) (len - 1)); a[i] = { (double) x[start + i] * w, 0.0 }; }
    fft(a);
    std::size_t lo = (std::size_t) std::max(1.0, std::floor(fLo * N / fs)), hi = (std::size_t) std::min((double) N / 2 - 2, std::ceil(fHi * N / fs));
    std::size_t k = lo; double best = -1;
    for (std::size_t i = lo; i <= hi; ++i) { double m = std::abs(a[i]); if (m > best) { best = m; k = i; } }
    double l = std::log(std::abs(a[k - 1]) + 1e-300), c = std::log(std::abs(a[k]) + 1e-300), r = std::log(std::abs(a[k + 1]) + 1e-300);
    double den = l - 2 * c + r; double p = den != 0 ? 0.5 * (l - r) / den : 0.0;
    return ((double) k + p) * fs / (double) N;
}
double cents(double f, double ref) { return 1200.0 * std::log2(f / ref); }
std::vector<double> pitchTrack(const std::vector<float>& x, double fs, std::size_t frame, std::size_t hop, double fLo, double fHi) {
    std::vector<double> out; for (std::size_t s = 0; s + frame <= x.size(); s += hop) out.push_back(peakFrequency(x, fs, s, frame, fLo, fHi)); return out;
}
double componentT60(const std::vector<float>& x, double fs, double freqHz, std::size_t start, std::size_t len) {
    len = std::min(len, x.size() > start ? x.size() - start : 0);
    const std::size_t win = std::max<std::size_t>(1, (std::size_t) (0.010 * fs));
    std::vector<std::complex<double>> h(len);
    for (std::size_t i = 0; i < len; ++i) { double ph = -2.0 * kPi * freqHz * (double) (start + i) / fs; h[i] = (double) x[start + i] * std::complex<double>(std::cos(ph), std::sin(ph)); }
    // Low-pass = three cascaded 10 ms moving averages (sinc^3; <= -60 dB at >= 300 Hz offset) to reject neighbouring partials.
    for (int pass = 0; pass < 3; ++pass) {
        std::vector<std::complex<double>> o(len); std::complex<double> acc = 0;
        for (std::size_t i = 0; i < len; ++i) { acc += h[i]; if (i >= win) acc -= h[i - win]; o[i] = acc / (double) win; }
        h.swap(o);
    }
    std::vector<double> tdb, ydb;
    for (std::size_t i = 3 * win; i < len; i += win) { double m = std::abs(h[i]); if (m > 0) { tdb.push_back((double) (start + i) / fs); ydb.push_back(20.0 * std::log10(m)); } }
    if (tdb.size() < 3) return std::numeric_limits<double>::quiet_NaN();
    double mt = 0, my = 0; for (std::size_t i = 0; i < tdb.size(); ++i) { mt += tdb[i]; my += ydb[i]; } mt /= (double) tdb.size(); my /= (double) tdb.size();
    double sxy = 0, sxx = 0; for (std::size_t i = 0; i < tdb.size(); ++i) { sxy += (tdb[i] - mt) * (ydb[i] - my); sxx += (tdb[i] - mt) * (tdb[i] - mt); }
    double slope = sxy / sxx; return slope < 0 ? -60.0 / slope : std::numeric_limits<double>::infinity();
}
double rms(const std::vector<float>& x, std::size_t start, std::size_t len) {
    double s = 0; std::size_t n = 0; for (std::size_t i = start; i < std::min(x.size(), start + len); ++i, ++n) s += (double) x[i] * x[i]; return n ? std::sqrt(s / (double) n) : 0.0;
}
bool allFinite(const std::vector<float>& x) { return std::all_of(x.begin(), x.end(), [](float v) { return std::isfinite(v); }); }
float peakAbs(const std::vector<float>& x) { float m = 0; for (float v : x) m = std::max(m, std::fabs(v)); return m; }
}
