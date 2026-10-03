// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
#include "cs/Engine.h"
#include "cs/StateCodec.h"
#include <catch2/catch_test_macros.hpp>
#include <random>
using namespace cs;
static PluginState sample() {
    PluginState s; s.instrumentKey = "guzheng";
    for (int p = 0; p < kNumParams; ++p) s.params[paramKey(static_cast<ParamId>(p))] = paramRange(static_cast<ParamId>(p)).def;
    s.sclText = "desc\n5\n9/8\n81/64\n3/2\n27/16\n2/1\n"; return s;
}
TEST_CASE("T-090 state round-trip", "[integration][SC-11][D-016]") {
    auto s = sample(); auto bytes = encodeState(s); REQUIRE(bytes.size() > 8);
    auto d = decodeState(bytes.data(), (int) bytes.size()); REQUIRE(d.has_value()); REQUIRE(*d == s);
}
TEST_CASE("T-091 state decoding rejects invalid input, clamps params, drops oversize/invalid tuning text", "[security][SC-11][A-013][D-016]") {
    REQUIRE_FALSE(decodeState(nullptr, 0).has_value());
    std::uint8_t junk[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; REQUIRE_FALSE(decodeState(junk, 16).has_value());
    auto s = sample(); s.instrumentKey = "not-an-instrument"; auto b = encodeState(s); REQUIRE_FALSE(decodeState(b.data(), (int) b.size()).has_value());
    s = sample(); s.version = kStateVersion + 1; b = encodeState(s); REQUIRE_FALSE(decodeState(b.data(), (int) b.size()).has_value());
    s = sample(); s.params["gain"] = 7.0f; s.params["vibratoDepth"] = -3.0f; s.params["unknownKey"] = 1.0f; b = encodeState(s);
    auto d = decodeState(b.data(), (int) b.size()); REQUIRE(d.has_value());
    REQUIRE(d->params.at("gain") == paramRange(ParamId::Gain).max); REQUIRE(d->params.at("vibratoDepth") == paramRange(ParamId::VibratoDepth).min);
    REQUIRE(d->params.count("unknownKey") == 0);
    s = sample(); s.sclText = std::string(70000, 'x'); b = encodeState(s); d = decodeState(b.data(), (int) b.size()); REQUIRE(d.has_value()); REQUIRE(d->sclText.empty());
    s = sample(); s.sclText = "desc\nnot-a-number\n"; b = encodeState(s); d = decodeState(b.data(), (int) b.size()); REQUIRE(d.has_value()); REQUIRE(d->sclText.empty());
}
TEST_CASE("T-092 every truncation and 2000 seeded byte-flips of a valid state decode without throwing", "[security][SC-11][A-013]") {
    auto b = encodeState(sample());
    for (std::size_t n = 0; n <= b.size(); ++n) REQUIRE_NOTHROW(decodeState(b.data(), (int) n));
    std::mt19937 rng(42);
    for (int k = 0; k < 2000; ++k) { auto c = b; c[rng() % c.size()] ^= (std::uint8_t) (1u << (rng() % 8)); REQUIRE_NOTHROW(decodeState(c.data(), (int) c.size())); }
}
