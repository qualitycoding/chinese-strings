// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256). Release builds only (CTest label "perf").
#include "cs/Engine.h"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstdio>
#include <vector>
using namespace cs;
static double calibrationSeconds() {      // identical workload to spike S-05
    auto c0 = std::chrono::steady_clock::now(); volatile double acc = 0;
    for (long k = 0; k < 200000000L; ++k) acc = acc + 1e-9 * (double) (k & 1023);
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - c0).count();
}
TEST_CASE("T-080 16 free-polyphony voices of the heaviest instrument: normalised load <= 0.45", "[perf][SC-13][A-012][D-008]") {
    // 0.45 x calibration (~0.55 s on the S-05 reference sandbox core) ~= 25 % of one core (A-012).
    const double calib = calibrationSeconds(); double worst = 0; int worstId = -1;
    for (int i = 0; i < kNumInstruments; ++i) {
        Engine e; e.prepare(48000, 128); e.setInstrument(static_cast<InstrumentId>(i)); e.setParameter(ParamId::FreePolyphony, 1.0f);
        const auto& s = spec(static_cast<InstrumentId>(i));
        for (int k = 0; k < kFreePolyphonyVoices; ++k) { std::uint8_t on[3] = {0x90, (std::uint8_t) (s.midiLow + k), 100}; e.handleMidi(on, 3, 0); }
        std::vector<float> L(128), R(128);
        auto t0 = std::chrono::steady_clock::now();
        for (int b = 0; b < 48000 * 10 / 128; ++b) e.render(L.data(), R.data(), 128);
        double load = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count() / 10.0 / calib;
        if (load > worst) { worst = load; worstId = i; }
    }
    std::printf("T-080 calibration_s=%.3f worst_normalised_load=%.3f instrument=%d\n", calib, worst, worstId);
    REQUIRE(worst <= 0.45);
}
