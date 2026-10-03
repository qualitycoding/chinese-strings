// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
#include "cs/Dsp.h"
#include "cs/Instruments.h"
#include "../fixtures/Anchors.h"
#include "../support/Analysis.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <vector>
using namespace cs; using namespace cs::dsp;
static StringSpec yehuS1() { auto& a = anchors::kYehu[0]; return {anchors::midiHz(a.midi), a.Lv, a.T, a.rho, a.r, a.E, a.s0, a.s1, StringMaterial::Silk, 1, 0}; }
static StringSpec erhuOuter() { auto& a = anchors::kErhu[1]; return {440.0, a.Lv, a.T, a.rho, a.diamM / 2, a.E, 0.9754, 0.0026, StringMaterial::Steel, 1, 0}; }
static std::vector<float> pluckAndRender(WaveguideString& s, double fs, double secs) {
    s.pluck(0.13, 1.0); std::vector<float> y((std::size_t) (fs * secs)); for (auto& v : y) v = s.tick(0.0f); return y;
}

TEST_CASE("T-020 waveguide string pitch within +-3 cents (A-011) across fs and materials", "[acoustic][SC-02][C-030][C-037]") {
    // Tolerance +-3 cents = A-011 success criterion; analysis error < 0.1 cent (SUPPORT-1).
    for (double fs : {44100.0, 48000.0, 96000.0}) for (auto base : {yehuS1(), erhuOuter()}) for (double f : {65.41, 146.83, 293.66, 440.0, 880.0, 1760.0}) {
        WaveguideString s; s.prepare(fs, 30.0); s.configure(base); s.setFrequency(f);
        auto y = pluckAndRender(s, fs, 1.5);
        REQUIRE(cstest::allFinite(y));
        double est = cstest::peakFrequency(y, fs, (std::size_t) (0.1 * fs), (std::size_t) (1.2 * fs), f * 0.94, f * 1.06);
        INFO("fs=" << fs << " f=" << f << " est=" << est);
        REQUIRE(std::fabs(cstest::cents(est, f)) <= 3.0);
    }
}

TEST_CASE("T-021 partials follow the stiff-string law n f1 sqrt(1+B n^2)/sqrt(1+B) within 3 cents (n<=8)", "[acoustic][SC-05][C-037]") {
    const double fs = 48000; auto st = yehuS1(); const double B = inharmonicityB(st), f1 = st.openHz;
    WaveguideString s; s.prepare(fs, 30.0); s.configure(st); s.setFrequency(f1);
    auto y = pluckAndRender(s, fs, 2.0);
    for (int n = 1; n <= 8; ++n) {
        double target = n * f1 * std::sqrt(1 + B * n * n) / std::sqrt(1 + B);
        double est = cstest::peakFrequency(y, fs, (std::size_t) (0.05 * fs), (std::size_t) (1.0 * fs), target - 0.3 * f1, target + 0.3 * f1);
        INFO("n=" << n); REQUIRE(std::fabs(cstest::cents(est, target)) <= 3.0);
    }
}

TEST_CASE("T-022 partial decay matches the DAFx26 oracle (yehu F4) within [0.70, 1.10] x T60", "[acoustic][SC-03][C-063]") {
    // Asymmetric tolerance: bridge coupling may only add damping (round-4 R5 note); +10 % covers loss-filter fit (T-010: 5 %) + analysis (3 %).
    const double fs = 48000; auto st = yehuS1();
    WaveguideString s; s.prepare(fs, 30.0); s.configure(st); s.setFrequency(st.openHz);
    auto y = pluckAndRender(s, fs, 6.0); const double B = inharmonicityB(st);
    for (int n : {1, 2, 3, 4, 6}) {
        double fn = n * st.openHz * std::sqrt(1 + B * n * n) / std::sqrt(1 + B);
        double oracle = anchors::kYehuS1T60[n - 1];
        double t60 = cstest::componentT60(y, fs, fn, (std::size_t) (0.05 * fs), (std::size_t) (std::min(5.5, oracle * 0.8) * fs));
        INFO("n=" << n << " t60=" << t60 << " oracle=" << oracle);
        REQUIRE(t60 >= 0.70 * oracle); REQUIRE(t60 <= 1.10 * oracle);
    }
}

TEST_CASE("T-023 passivity: |H|<=1 for every instrument string; free decay is monotone and bounded", "[acoustic][SC-06][C-062][D-004a]") {
    for (double fs : {44100.0, 48000.0, 96000.0}) for (int i = 0; i < kNumInstruments; ++i) for (const auto& st : spec(static_cast<InstrumentId>(i)).strings) {
        WaveguideString s; s.prepare(fs, 20.0); s.configure(st); s.setFrequency(st.openHz);
        for (double m : s.lossCoefficientsMagnitude(4096)) REQUIRE(m <= 1.0 + 1e-9);
    }
    // 100 ms windows >> period; minimum decay of the loss law (sigma0 >= 0.9754/s) is ~9 %/window in RMS, so 1 % slack is conservative.
    const double fs = 48000; WaveguideString s; s.prepare(fs, 20.0); s.configure(yehuS1()); s.setFrequency(349.23);
    auto y = pluckAndRender(s, fs, 30.0); REQUIRE(cstest::allFinite(y));
    const std::size_t w = (std::size_t) (0.1 * fs); double prev = cstest::rms(y, 0, w); REQUIRE(prev > 0.0);
    for (std::size_t k = 1; k < 300; ++k) { double r = cstest::rms(y, k * w, w); REQUIRE(r <= prev * 1.01 + 1e-12); prev = r; }
    REQUIRE(cstest::rms(y, y.size() - w, w) < 1e-3 * cstest::rms(y, 0, w));
}
