// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
#include "cs/Dsp.h"
#include "../support/Analysis.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <vector>
using namespace cs; using namespace cs::dsp;

TEST_CASE("T-010 LossFilter matches the DAFx26 loss law and is passive", "[unit][SC-03][SC-06][C-063][D-004a]") {
    // Tolerance: |ln|H| - ln|H_target|| <= 5 % of |ln|H_target||  => T60 error <= 5 % (T60 = -3 ln10 / (f0 ln|H|)).
    for (double fs : {44100.0, 48000.0, 96000.0}) {
        for (double f0 : {110.0, 349.23, 880.0}) {
            LossFilter lf; lf.design(fs, f0, 0.9754, 0.0026, 0.295);
            const double c = 2 * 0.295 * f0;
            for (int n = 1; n <= 10 && n * f0 < 0.4 * fs; ++n) {
                double f = n * f0, beta = 2 * M_PI * f / c;
                double target = std::exp(-(0.9754 + 0.0026 * beta * beta) / f0);
                REQUIRE(std::fabs(lf.targetMagnitudeAt(f) / target - 1.0) < 1e-12);
                REQUIRE(std::fabs(std::log(lf.magnitudeAt(f)) - std::log(target)) <= 0.05 * std::fabs(std::log(target)));
            }
            for (int k = 0; k <= 4096; ++k) REQUIRE(lf.magnitudeAt(0.5 * fs * k / 4096.0) <= 1.0 + 1e-9);
        }
    }
}

TEST_CASE("T-011 DispersionFilter coefficients are stable and delay falls with frequency for B>0", "[unit][SC-06][D-004a]") {
    DispersionFilter d; d.design(48000, 349.23, 1.6e-3, 4);
    auto a = d.coefficients(); REQUIRE(a.size() == 4);
    for (double v : a) REQUIRE(std::fabs(v) < 1.0);
    double prev = d.phaseDelaySamples(349.23);
    for (int n = 2; n <= 10; ++n) { double cur = d.phaseDelaySamples(349.23 * n); REQUIRE(cur <= prev + 1e-9); prev = cur; }
    DispersionFilter z; z.design(48000, 349.23, 0.0, 4);
    for (double v : z.coefficients()) REQUIRE(std::fabs(v) < 1.0);
}

TEST_CASE("T-012 FractionalDelayLine realises the requested delay (phase at 100 Hz) within 0.01 samples", "[unit][SC-02]") {
    const double fs = 48000, f = 100;
    for (double D : {1.3, 10.5, 100.25, 733.9}) {
        FractionalDelayLine dl; dl.prepare(2048); dl.setDelay(D);
        std::vector<float> in(48000), out(48000);
        for (std::size_t i = 0; i < in.size(); ++i) { in[i] = (float) std::sin(2 * M_PI * f * (double) i / fs); out[i] = dl.process(in[i]); }
        // cross-correlation phase estimate over the steady part
        double sc = 0, ss = 0; for (std::size_t i = 4800; i < in.size(); ++i) { double t = (double) i / fs; sc += out[i] * std::cos(2 * M_PI * f * t); ss += out[i] * std::sin(2 * M_PI * f * t); }
        double phase = std::atan2(-sc, ss);            // out = sin(wt - phase)
        double delay = phase / (2 * M_PI * f) * fs; if (delay < 0) delay += fs / f;
        REQUIRE(std::fabs(delay - std::fmod(D, fs / f)) < 0.01);
    }
}

TEST_CASE("T-013 bow junction reflection coefficient lies in [0,1) and grows with bow force", "[unit][SC-06][C-062]") {
    FrictionParams p;
    for (double z : {0.1, 1.0, 10.0}) for (double v = -5.0; v <= 5.0; v += 0.01) {
        REQUIRE(BowJunction::reflectionCoefficient(v, 0.0, z, p) == 0.0);
        double prev = 0.0;
        for (double F : {0.05, 0.2, 0.5, 1.0, 2.0, 5.0}) {
            double r = BowJunction::reflectionCoefficient(v, F, z, p);
            REQUIRE(r >= 0.0); REQUIRE(r < 1.0); REQUIRE(r >= prev - 1e-12); prev = r;
        }
    }
}

TEST_CASE("T-014 Hunt-Crossley contact law (DAFx26 eq. 4-5)", "[unit][SC-06][C-028]") {
    HuntCrossleyContact c(5e5, 2.3, 20.0);
    REQUIRE(c.force(0.0, 1.0) == 0.0); REQUIRE(c.force(-1e-4, 1.0) == 0.0);
    for (double x : {1e-6, 1e-5, 1e-4}) {
        REQUIRE(std::fabs(c.force(x, 0.0) / (5e5 * std::pow(x, 2.3)) - 1.0) < 1e-12);
        REQUIRE(std::fabs(c.force(x, 0.01) / (5e5 * std::pow(x, 2.3) * (1 + 20.0 * 0.01)) - 1.0) < 1e-12);
    }
}

TEST_CASE("T-015 ModalBank reproduces configured mode frequencies (0.5 %) and T60 (10 %)", "[unit][SC-04]") {
    const double fs = 48000; std::vector<ModeSpec> m = {{200, 1.0, 1.0}, {523, 0.5, 1.0}, {1500, 0.2, 1.0}};
    ModalBank b; b.configure(fs, m);
    std::vector<float> y((std::size_t) fs * 4); y[0] = b.process(1.0f); for (std::size_t i = 1; i < y.size(); ++i) y[i] = b.process(0.0f);
    REQUIRE(cstest::allFinite(y));
    for (auto& md : m) {
        REQUIRE(std::fabs(cstest::peakFrequency(y, fs, 0, (std::size_t) (fs * md.t60s * 0.5), md.freqHz * 0.9, md.freqHz * 1.1) / md.freqHz - 1.0) < 0.005);
        REQUIRE(std::fabs(cstest::componentT60(y, fs, md.freqHz, 240, (std::size_t) (fs * md.t60s * 0.5)) / md.t60s - 1.0) < 0.10);
    }
}
