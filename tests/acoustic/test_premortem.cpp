// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256). Added by pre-mortem round 1 (R-005, R-006, R-008).
#include "EngineHarness.h"
#include "cs/EngineHost.h"
#include "../support/Analysis.h"
#include <catch2/catch_test_macros.hpp>
#include <atomic>
#include <chrono>
#include <cmath>
#include <thread>
#include <vector>
using namespace cs; using namespace harness;

TEST_CASE("T-044 EngineHost: concurrent instrument switching, parameter writes and snapshots while rendering (run under TSAN in CI)", "[integration][concurrency][SC-19][D-024][R-005][R-008]") {
    EngineHost h; h.prepare(48000, 256);
    std::atomic<bool> stop{false}; std::atomic<long> blocks{0}; std::atomic<bool> bad{false};
    std::thread audio([&] {
        std::vector<float> L(256), R(256); int k = 0;
        while (!stop.load()) {
            if (k % 20 == 0) { std::uint8_t on[3] = {0x90, (std::uint8_t) (50 + k % 24), 100}; h.handleMidi(on, 3, 0); }
            h.render(L.data(), R.data(), 256);
            for (float v : L) if (!std::isfinite(v) || std::fabs(v) > 8.0f) bad = true;
            ++k; ++blocks;
        }
    });
    std::thread ui([&] { VoiceInfo v[16]; while (!stop.load()) { int n = h.voiceSnapshot(v, 16); if (n < 0 || n > 16) bad = true;
                                                               for (int i = 0; i < n; ++i) if (!(v[i].currentHz > 0.0) || v[i].stringIndex < 0) bad = true; } });
    for (int r = 0; r < 200; ++r) {
        h.requestInstrument(static_cast<InstrumentId>(r % kNumInstruments));
        h.setParameter(ParamId::Brightness, (float) (r % 10) / 10.f);
        if (r % 10 == 0) h.collectGarbage();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    h.requestInstrument(InstrumentId::Guqin);
    auto t0 = std::chrono::steady_clock::now();
    while (h.currentInstrument() != InstrumentId::Guqin && std::chrono::steady_clock::now() - t0 < std::chrono::seconds(5)) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    stop = true; audio.join(); ui.join(); h.collectGarbage();
    REQUIRE_FALSE(bad.load());
    REQUIRE(blocks.load() > 100);
    REQUIRE(h.currentInstrument() == InstrumentId::Guqin);
}

TEST_CASE("T-045 family excitation signature: bowed instruments sustain, all others decay (no placeholder voices)", "[integration][SC-01][R-006]") {
    // Ratio = RMS(2.5-3.0 s) / RMS(0.5-1.0 s) with the note held. Bowed (continuous bow) >= 0.8; plucked/zither/struck <= 0.8
    // (free decay: the smallest material sigma0 in INSTRUMENT_DATA.md, 0.60/s, gives exp(-1.2) = 0.30).
    for (int i = 0; i < kNumInstruments; ++i) {
        Engine e; e.prepare(48000, 128); auto id = static_cast<InstrumentId>(i); e.setInstrument(id);
        const auto& s = spec(id); noteOn(e, 1, midiFromHz(s.strings[0].openHz), 100);
        auto y = render(e, 48000, 3.0, 128);
        double ratio = cstest::rms(y, 120000, 24000) / cstest::rms(y, 24000, 24000);
        INFO(s.key << " ratio=" << ratio);
        if (s.family == Family::Bowed) REQUIRE(ratio >= 0.8); else REQUIRE(ratio <= 0.8);
    }
}
