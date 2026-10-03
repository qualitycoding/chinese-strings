// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256). Separate executable: replaces global operator new/delete.
#include "cs/Engine.h"
#include <catch2/catch_test_macros.hpp>
#include <atomic>
#include <cstdlib>
#include <new>
#include <vector>
static std::atomic<long> gAllocs{0}; static thread_local bool gGuard = false;
void* operator new(std::size_t n) { if (gGuard) gAllocs++; if (void* p = std::malloc(n ? n : 1)) return p; throw std::bad_alloc(); }
void* operator new[](std::size_t n) { if (gGuard) gAllocs++; if (void* p = std::malloc(n ? n : 1)) return p; throw std::bad_alloc(); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }
using namespace cs;
TEST_CASE("T-070 zero heap allocations on the audio path for every instrument", "[rt][SC-12][D-009][A-012]") {
    for (int i = 0; i < kNumInstruments; ++i) {
        Engine e; e.prepare(48000, 128); e.setInstrument(static_cast<InstrumentId>(i)); e.setMpeEnabled(true);
        std::vector<float> L(128), R(128); const auto& s = spec(static_cast<InstrumentId>(i));
        long before = gAllocs.load(); gGuard = true;
        for (int b = 0; b < 750; ++b) {           // 2 s
            if (b % 50 == 0) { std::uint8_t on[3] = {0x91, (std::uint8_t) (s.midiLow + (b / 50) % 12), 100}; e.handleMidi(on, 3, b % 128); }
            if (b % 50 == 25) { std::uint8_t ks[3] = {0x90, (std::uint8_t) (kKeyswitchLow + (b / 50) % kNumTechniques), 100}; e.handleMidi(ks, 3, 0); }
            if (b % 10 == 0) { e.setParameter(ParamId::Brightness, (float) (b % 100) / 100.f); std::uint8_t pb[3] = {0xE1, 0, (std::uint8_t) (b % 128)}; e.handleMidi(pb, 3, 7); }
            e.render(L.data(), R.data(), 128);
        }
        gGuard = false; INFO(s.key); REQUIRE(gAllocs.load() == before);
    }
}
