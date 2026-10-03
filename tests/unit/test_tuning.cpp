// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
#include "cs/Tuning.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <string>
using namespace cs;
static const char* kEt12 = "! et12.scl\n12-tone equal temperament\n 12\n!\n100.0\n200.\n300.\n400.\n500.\n600.\n700.\n800.\n900.\n1000.\n1100.\n2/1\n";
static const char* kPenta = "! penta.scl\nPythagorean pentatonic (gong shang jue zhi yu)\n5\n9/8\n81/64\n3/2\n27/16\n2/1\n";

TEST_CASE("T-050 equal temperament table", "[unit][SC-10]") {
    auto t = TuningTable::equal(); REQUIRE(t.hz[69] == 440.0);
    REQUIRE(std::fabs(t.hz[60] - 261.6255653) < 1e-6);
    for (int n = 1; n < 128; ++n) REQUIRE(std::fabs(t.hz[(std::size_t) n] / t.hz[(std::size_t) n - 1] - std::pow(2.0, 1.0 / 12.0)) < 1e-12);
    REQUIRE(TuningTable::equal(432.0).hz[69] == 432.0);
}

TEST_CASE("T-051 Scala import: 12-TET equals equal(); pentatonic maps degree 0 to MIDI 60 at 261.6256 Hz", "[unit][SC-10][C-042][D-010]") {
    auto a = loadScala(kEt12, ""); REQUIRE(a.has_value()); auto e = TuningTable::equal();
    for (std::size_t n = 0; n < 128; ++n) REQUIRE(std::fabs(a->hz[n] / e.hz[n] - 1.0) < 1e-9);
    auto p = loadScala(kPenta, ""); REQUIRE(p.has_value());
    REQUIRE(std::fabs(p->hz[60] - 261.6255653) < 1e-6);
    REQUIRE(std::fabs(p->hz[61] / p->hz[60] - 9.0 / 8.0) < 1e-12);
    REQUIRE(std::fabs(p->hz[63] / p->hz[60] - 1.5) < 1e-12);
    REQUIRE(std::fabs(p->hz[65] / p->hz[60] - 2.0) < 1e-12);
    REQUIRE(std::fabs(p->hz[55] / p->hz[60] - 0.5) < 1e-12);
}

TEST_CASE("T-052 Scala import rejects malformed or oversized input without throwing", "[unit][security][SC-10][A-013][D-010]") {
    const std::string bad[] = {"", "only description\n", "desc\nx\n", "desc\n3\n100.0\n", "desc\n1\n-3/2\n", "desc\n1\n0/1\n", "desc\n1\n3/0\n",
                               "desc\n1\nabc\n", std::string("\xff\xfe\x00\x01", 4), "desc\n-1\n"};
    for (const auto& b : bad) { INFO(b); std::optional<TuningTable> r; REQUIRE_NOTHROW(r = loadScala(b, "")); REQUIRE_FALSE(r.has_value()); }
    std::string big = "desc\n1\n2/1\n! " + std::string(70000, 'x') + "\n";
    REQUIRE_FALSE(loadScala(big, "").has_value());
    std::string many = "desc\n1025\n"; for (int i = 1; i <= 1025; ++i) many += std::to_string(i) + ".0\n";
    REQUIRE_FALSE(loadScala(many, "").has_value());
    REQUIRE_FALSE(loadScala(kEt12, "garbage kbm").has_value());
}
