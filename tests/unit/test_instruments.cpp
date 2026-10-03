// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
#include "cs/Engine.h"
#include "cs/Instruments.h"
#include "../fixtures/Anchors.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <set>
#include <string>
using namespace cs;
static InstrumentId idx(int i) { return static_cast<InstrumentId>(i); }
static double idealHz(const StringSpec& s) { return std::sqrt(s.tensionN / (s.densityKgM3 * M_PI * s.radiusM * s.radiusM)) / (2.0 * s.vibratingLengthM); }

TEST_CASE("T-001 catalogue: 19 instruments, keys round-trip, family counts, structural sanity", "[unit][SC-01][D-003]") {
    std::set<std::string> keys; int fam[4] = {0, 0, 0, 0};
    for (int i = 0; i < kNumInstruments; ++i) {
        const auto& s = spec(idx(i));
        REQUIRE(s.id == idx(i));
        REQUIRE(keys.insert(std::string(s.key)).second);
        REQUIRE(instrumentFromKey(s.key) == idx(i));
        fam[(int) s.family]++;
        REQUIRE(!s.strings.empty());
        for (std::size_t k = 1; k < s.strings.size(); ++k) REQUIRE(s.strings[k].openHz >= s.strings[k - 1].openHz);
        REQUIRE(!s.citations.empty());
        REQUIRE(s.midiLow < s.midiHigh);
        int strings = 0; for (auto& st : s.strings) strings += st.courses;
        REQUIRE(s.maxPolyphony >= 1);
        REQUIRE(s.maxPolyphony <= strings);
        REQUIRE(supportsTechnique(idx(i), Technique::Default));
    }
    REQUIRE(fam[(int) Family::Bowed] == 8); REQUIRE(fam[(int) Family::PluckedLute] == 7);
    REQUIRE(fam[(int) Family::Zither] == 3); REQUIRE(fam[(int) Family::Struck] == 1);
    REQUIRE_FALSE(instrumentFromKey("gehu").has_value());   // dropped (D-003)
    REQUIRE_FALSE(instrumentFromKey("").has_value());
}

TEST_CASE("T-002 cited anchors: tunings, string counts, huqin construction", "[unit][SC-02][C-012][C-032][C-034][C-039][C-040][C-047][C-048][C-049]") {
    auto openMidiNear = [](double hz, int midi) { return std::fabs(1200.0 * std::log2(hz / anchors::midiHz(midi))) < 0.01; };
    const auto& erhu = spec(InstrumentId::Erhu); REQUIRE(erhu.strings.size() == 2);
    for (int k = 0; k < 2; ++k) REQUIRE(openMidiNear(erhu.strings[k].openHz, anchors::kErhu[k].midi));
    const auto& yehu = spec(InstrumentId::Yehu); REQUIRE(yehu.strings.size() == 2);
    for (int k = 0; k < 2; ++k) REQUIRE(openMidiNear(yehu.strings[k].openHz, anchors::kYehu[k].midi));
    for (int k = 0; k < 2; ++k) REQUIRE(openMidiNear(spec(InstrumentId::Gaohu).strings[k].openHz, anchors::kGaohu[k]));
    for (int k = 0; k < 2; ++k) REQUIRE(openMidiNear(spec(InstrumentId::Zhonghu).strings[k].openHz, anchors::kZhonghu[k]));
    const auto& gz = spec(InstrumentId::Guzheng); REQUIRE(gz.strings.size() == 21);
    for (int k = 0; k < 21; ++k) REQUIRE(openMidiNear(gz.strings[k].openHz, anchors::kGuzheng[k]));
    for (auto id : {InstrumentId::Banhu, InstrumentId::Jinghu}) {
        const auto& s = spec(id); REQUIRE(s.strings.size() == 2);
        REQUIRE(std::fabs(1200.0 * std::log2(s.strings[1].openHz / s.strings[0].openHz) - 700.0) < 0.01);
    }
    REQUIRE(spec(InstrumentId::Jinghu).strings[0].openHz > erhu.strings[1].openHz);   // highest huqin
    const auto& sihu = spec(InstrumentId::Sihu); REQUIRE(sihu.strings.size() == 4);
    REQUIRE(sihu.strings[0].openHz == sihu.strings[1].openHz); REQUIRE(sihu.strings[2].openHz == sihu.strings[3].openHz);
    for (auto id : {InstrumentId::Pipa, InstrumentId::Liuqin, InstrumentId::Xiaoruan, InstrumentId::Zhongruan, InstrumentId::Daruan})
        REQUIRE(spec(id).strings.size() == 4);
    REQUIRE(spec(InstrumentId::Sanxian).strings.size() == 3);
    REQUIRE(spec(InstrumentId::Sanxian).fretSemitones.empty());
    REQUIRE(spec(InstrumentId::Guqin).strings.size() == 7);
    const auto& kh = spec(InstrumentId::Konghou); auto n = kh.strings.size();
    REQUIRE(n % 2 == 0); REQUIRE(n >= 72); REQUIRE(n <= 76);
    for (std::size_t k = 0; k < n; k += 2) REQUIRE(kh.strings[k].openHz == kh.strings[k + 1].openHz);
    REQUIRE(spec(InstrumentId::Liuqin).fretSemitones.size() == 29);
    for (auto id : {InstrumentId::Xiaoruan, InstrumentId::Zhongruan, InstrumentId::Daruan}) REQUIRE(spec(id).fretSemitones.size() == 24);
    for (int i = 0; i < 8; ++i) {               // bowed family
        const auto& s = spec(idx(i));
        REQUIRE(s.family == Family::Bowed);
        REQUIRE(s.maxPolyphony == 1);
        REQUIRE(s.fretSemitones.empty());
        if (idx(i) != InstrumentId::Matouqin) REQUIRE(s.stopsAllStringsTogether);
    }
}

TEST_CASE("T-003 physical self-consistency of string data; erhu & yehu equal cited values", "[unit][SC-02][C-032][C-034]") {
    for (int i = 0; i < kNumInstruments; ++i)
        for (const auto& st : spec(idx(i)).strings) {
            REQUIRE(st.vibratingLengthM > 0); REQUIRE(st.tensionN > 0); REQUIRE(st.densityKgM3 > 0); REQUIRE(st.radiusM > 0);
            REQUIRE(st.sigma0 >= 0); REQUIRE(st.sigma1 >= 0); REQUIRE(st.courses >= 1);
            REQUIRE(std::fabs(idealHz(st) / st.openHz - 1.0) <= 0.005);   // 0.5 %: data must be physically consistent
        }
    const auto& e = spec(InstrumentId::Erhu);
    for (int k = 0; k < 2; ++k) {
        REQUIRE(std::fabs(e.strings[k].radiusM - anchors::kErhu[k].diamM / 2) < 1e-12);
        REQUIRE(std::fabs(e.strings[k].tensionN - anchors::kErhu[k].T) < 1e-9);
        REQUIRE(std::fabs(e.strings[k].densityKgM3 - anchors::kErhu[k].rho) < 1e-9);
        REQUIRE(std::fabs(e.strings[k].youngsModulusPa - anchors::kErhu[k].E) < 1.0);
        REQUIRE(std::fabs(e.strings[k].vibratingLengthM - anchors::kErhu[k].Lv) < 5e-4);
        REQUIRE(e.strings[k].material == StringMaterial::Steel);
    }
    const auto& y = spec(InstrumentId::Yehu);
    for (int k = 0; k < 2; ++k) {
        REQUIRE(std::fabs(y.strings[k].radiusM - anchors::kYehu[k].r) < 1e-12);
        REQUIRE(std::fabs(y.strings[k].tensionN - anchors::kYehu[k].T) < 1e-9);
        REQUIRE(std::fabs(y.strings[k].densityKgM3 - anchors::kYehu[k].rho) < 1e-9);
        REQUIRE(std::fabs(y.strings[k].youngsModulusPa - anchors::kYehu[k].E) < 1.0);
        REQUIRE(std::fabs(y.strings[k].sigma0 - anchors::kYehu[k].s0) < 1e-12);
        REQUIRE(std::fabs(y.strings[k].sigma1 - anchors::kYehu[k].s1) < 1e-12);
        REQUIRE(std::fabs(y.strings[k].vibratingLengthM - anchors::kYehu[k].Lv) < 5e-4);
        REQUIRE(y.strings[k].material == StringMaterial::Silk);
    }
}

TEST_CASE("T-004 inharmonicity coefficient formula (Fletcher; S-03)", "[unit][C-037]") {
    StringSpec s{349.23, 0.295, 50.61, 1250, 0.55e-3, 1.0e10, 0, 0, StringMaterial::Silk, 1, 0};
    double expect = std::pow(M_PI, 3) * s.youngsModulusPa * std::pow(s.radiusM, 4) / (4 * s.tensionN * s.vibratingLengthM * s.vibratingLengthM);
    REQUIRE(std::fabs(inharmonicityB(s) / expect - 1.0) < 1e-12);
    REQUIRE(std::fabs(inharmonicityB(s) / 1.60e-3 - 1.0) < 0.01);
}

TEST_CASE("T-005 parameter table (D-020)", "[unit][SC-11]") {
    const char* keys[kNumParams] = {"gain","bowPressure","bowSpeed","pluckPosition","brightness","vibratoRate","vibratoDepth","pitchBendRange","freePolyphony"};
    for (int p = 0; p < kNumParams; ++p) {
        auto r = paramRange(static_cast<ParamId>(p));
        REQUIRE(r.min < r.max); REQUIRE(r.def >= r.min); REQUIRE(r.def <= r.max);
        REQUIRE(std::string(paramKey(static_cast<ParamId>(p))) == keys[p]);
    }
    REQUIRE(paramRange(ParamId::PitchBendRange).max == 48.0f);
    REQUIRE(paramRange(ParamId::VibratoDepth).max == 100.0f);
}
