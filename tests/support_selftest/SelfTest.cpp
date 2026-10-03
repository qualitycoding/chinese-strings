// FROZEN — DO NOT MODIFY. Validates test infrastructure only (not a requirement test); must PASS at every stage.
#include "../support/Analysis.h"
#include <catch2/catch_test_macros.hpp>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <new>
#include <vector>
static std::atomic<long> gAllocs{0}; static thread_local bool gGuard = false;
void* operator new(std::size_t n) { if (gGuard) gAllocs++; if (void* p = std::malloc(n ? n : 1)) return p; throw std::bad_alloc(); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
static std::vector<float> decayingSines(double fs, double secs, std::vector<std::pair<double, double>> fT60) {
    std::vector<float> x((std::size_t) (fs * secs), 0.f);
    for (auto [f, t60] : fT60) { double a = 3.0 * std::log(10.0) / t60; for (std::size_t i = 0; i < x.size(); ++i) { double t = (double) i / fs; x[i] += (float) (0.3 * std::exp(-a * t) * std::sin(2 * M_PI * f * t)); } }
    return x;
}
TEST_CASE("SUPPORT-1 peakFrequency resolves a pure tone to < 0.1 cent", "[support]") {
    for (double fs : {44100.0, 48000.0, 96000.0}) for (double f : {65.41, 293.66, 440.0, 1760.0}) {
        auto x = decayingSines(fs, 1.0, {{f, 50.0}});
        REQUIRE(std::fabs(cstest::cents(cstest::peakFrequency(x, fs, 0, x.size(), f * 0.8, f * 1.2), f)) < 0.1);
    }
}
TEST_CASE("SUPPORT-2 componentT60 recovers T60 within 3 % for separated partials", "[support]") {
    const double fs = 48000; auto x = decayingSines(fs, 4.0, {{349.23, 5.44}, {698.46, 2.5}, {1396.9, 1.21}});
    REQUIRE(std::fabs(cstest::componentT60(x, fs, 349.23, 2400, 96000) / 5.44 - 1.0) < 0.03);
    REQUIRE(std::fabs(cstest::componentT60(x, fs, 698.46, 2400, 96000) / 2.5 - 1.0) < 0.03);
    REQUIRE(std::fabs(cstest::componentT60(x, fs, 1396.9, 2400, 48000) / 1.21 - 1.0) < 0.03);
}
TEST_CASE("SUPPORT-3 pitchTrack follows 5 Hz vibrato of +-50 cents", "[support]") {
    const double fs = 48000, f0 = 440; std::vector<float> x((std::size_t) fs * 2); double ph = 0;
    for (std::size_t i = 0; i < x.size(); ++i) { double t = (double) i / fs; double f = f0 * std::pow(2.0, 50.0 / 1200.0 * std::sin(2 * M_PI * 5 * t)); ph += 2 * M_PI * f / fs; x[i] = (float) (0.3 * std::sin(ph)); }
    auto tr = cstest::pitchTrack(x, fs, 2048, 256, 300, 600); double mn = 1e9, mx = 0; for (double v : tr) { mn = std::min(mn, v); mx = std::max(mx, v); }
    double pp = cstest::cents(mx, mn); REQUIRE(pp > 80.0); REQUIRE(pp < 110.0);
}
TEST_CASE("SUPPORT-4 allocation detector negative control fires", "[support]") {
    long before = gAllocs.load(); gGuard = true; auto* p = new std::vector<float>(64); gGuard = false; delete p;
    REQUIRE(gAllocs.load() > before);
}
