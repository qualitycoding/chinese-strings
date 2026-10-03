// FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
#include "EngineHarness.h"
#include "cs/Engine.h"
#include "cs/Fingering.h"
#include "../support/Analysis.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <random>
#include <stdexcept>
using namespace cs; using namespace harness;
static InstrumentId idx(int i) { return static_cast<InstrumentId>(i); }
static int midNote(const InstrumentSpec& s) { return (s.midiLow + s.midiHigh) / 2; }

TEST_CASE("T-030 every instrument sounds, stays finite and bounded, and decays after release", "[integration][SC-01][SC-06]") {
    for (int i = 0; i < kNumInstruments; ++i) {
        Engine e; e.prepare(48000, 128); e.setInstrument(idx(i)); REQUIRE(e.instrument() == idx(i));
        const auto& s = spec(idx(i)); int n = midNote(s);
        noteOn(e, 1, n, 100); std::vector<float> R; auto L = render(e, 48000, 1.0, 128, &R);
        INFO(s.key); REQUIRE(cstest::allFinite(L)); REQUIRE(cstest::allFinite(R));
        REQUIRE(cstest::rms(L, 0, L.size()) > 1e-4); REQUIRE(cstest::peakAbs(L) <= 2.0f);
        noteOff(e, 1, n); auto tail = render(e, 48000, 6.0, 128);
        REQUIRE(cstest::rms(tail, tail.size() - 4800, 4800) < 1e-5);
    }
}

TEST_CASE("T-031 every open string of every instrument sounds at its pitch within +-3 cents", "[integration][SC-02][A-011]") {
    for (int i = 0; i < kNumInstruments; ++i) {
        const auto& s = spec(idx(i));
        for (std::size_t k = 0; k < s.strings.size(); ++k) {
            double f = s.strings[k].openHz; int note = midiFromHz(f);
            if (std::fabs(1200.0 * std::log2(f / (440.0 * std::pow(2.0, (note - 69) / 12.0)))) > 0.01) continue; // only 12-TET open strings
            Engine e; e.prepare(48000, 128); e.setInstrument(idx(i)); noteOn(e, 1, note, 100);
            auto y = render(e, 48000, 1.5, 128);
            double est = cstest::peakFrequency(y, 48000, 24000, 48000, f * 0.94, f * 1.06);
            INFO(s.key << " string " << k << " f=" << f << " est=" << est); REQUIRE(std::fabs(cstest::cents(est, f)) <= 3.0);
        }
    }
}

TEST_CASE("T-032 sample rates 22.05-192 kHz and block sizes 1-1024: finite output, erhu A4 within 3 cents", "[integration][operational][SC-02][SC-16]") {
    for (double fs : {22050.0, 44100.0, 48000.0, 96000.0, 192000.0}) for (int block : {1, 64, 128, 512, 1024}) {
        Engine e; e.prepare(fs, 1024); e.setInstrument(InstrumentId::Erhu); noteOn(e, 1, 69, 100);
        auto y = render(e, fs, 1.2, block); REQUIRE(cstest::allFinite(y));
        double est = cstest::peakFrequency(y, fs, (std::size_t) (0.5 * fs), (std::size_t) (0.6 * fs), 410, 470);
        INFO("fs=" << fs << " block=" << block); REQUIRE(std::fabs(cstest::cents(est, 440.0)) <= 3.0);
    }
}

TEST_CASE("T-033 60 s seeded random automation: no NaN/Inf, |x| <= 8", "[integration][SC-06]") {
    std::mt19937 rng(42); std::uniform_real_distribution<float> u(0.f, 1.f);
    for (int i = 0; i < kNumInstruments; ++i) {
        Engine e; e.prepare(48000, 256); e.setInstrument(idx(i)); const auto& s = spec(idx(i));
        std::vector<float> L(256), R(256);
        for (int b = 0; b < 48000 * 60 / 256; ++b) {
            if (b % 40 == 0) { for (int p = 0; p < kNumParams; ++p) { auto r = paramRange(static_cast<ParamId>(p)); e.setParameter(static_cast<ParamId>(p), r.min + u(rng) * (r.max - r.min)); }
                               noteOn(e, 1, s.midiLow + (int) (u(rng) * (float) (s.midiHigh - s.midiLow)), 1 + (int) (u(rng) * 126)); }
            if (b % 40 == 20) noteOff(e, 1, s.midiLow + (int) (u(rng) * (float) (s.midiHigh - s.midiLow)));
            e.render(L.data(), R.data(), 256);
            for (int j = 0; j < 256; ++j) { REQUIRE(std::isfinite(L[(std::size_t) j])); REQUIRE(std::fabs(L[(std::size_t) j]) <= 8.0f); }
        }
    }
}

TEST_CASE("T-034 polyphony limits, free-polyphony override, bowed legato", "[integration][SC-07][A-010]") {
    for (int i = 0; i < kNumInstruments; ++i) {
        const auto& s = spec(idx(i)); Engine e; e.prepare(48000, 128); e.setInstrument(idx(i));
        for (int k = 0; k < s.maxPolyphony + 3; ++k) noteOn(e, 1, s.midiLow + k, 100);
        render(e, 48000, 0.05, 128); INFO(s.key); REQUIRE(e.activeVoiceCount() <= s.maxPolyphony); REQUIRE(e.activeVoiceCount() >= 1);
        Engine f; f.prepare(48000, 128); f.setInstrument(idx(i)); f.setParameter(ParamId::FreePolyphony, 1.0f);
        for (int k = 0; k < kFreePolyphonyVoices + 3; ++k) noteOn(f, 1, s.midiLow + k, 100);
        render(f, 48000, 0.05, 128); REQUIRE(f.activeVoiceCount() == std::min(kFreePolyphonyVoices, s.midiHigh - s.midiLow + 1));
    }
    Engine e; e.prepare(48000, 128); e.setInstrument(InstrumentId::Erhu);
    noteOn(e, 1, 69, 100); render(e, 48000, 0.3, 128); noteOn(e, 1, 71, 100); render(e, 48000, 0.3, 128);
    REQUIRE(e.activeVoiceCount() == 1); VoiceInfo v{}; REQUIRE(e.voiceInfo(&v, 1) == 1);
    REQUIRE(v.midiNote == 71); REQUIRE(std::fabs(cstest::cents(v.currentHz, 493.883)) <= 3.0);
}

TEST_CASE("T-035 keyswitches select techniques (D-017) without sounding; supported techniques render", "[integration][SC-08][D-017]") {
    for (int i = 0; i < kNumInstruments; ++i) for (int t = 0; t < kNumTechniques; ++t) {
        Engine e; e.prepare(48000, 128); e.setInstrument(idx(i));
        noteOn(e, 1, kKeyswitchLow + t, 100); render(e, 48000, 0.02, 128);
        REQUIRE(e.activeVoiceCount() == 0);
        auto tech = static_cast<Technique>(t); INFO(spec(idx(i)).key << " technique " << t);
        REQUIRE(e.technique() == (supportsTechnique(idx(i), tech) ? tech : Technique::Default));
        const auto& s = spec(idx(i)); noteOn(e, 1, midNote(s), 100); auto y = render(e, 48000, 0.5, 128);
        REQUIRE(cstest::allFinite(y)); REQUIRE(cstest::rms(y, 0, y.size()) > 1e-5);
    }
}

TEST_CASE("T-036 vibrato CCs: 50 cents depth at 5 Hz on erhu A4 -> 80..120 cents p-p, rate 4.5..5.5 Hz", "[integration][SC-08]") {
    // Depth tolerance +-20 % covers bowed-pitch jitter; rate +-0.5 Hz from zero-crossing count over 3 s.
    Engine e; e.prepare(48000, 128); e.setInstrument(InstrumentId::Erhu);
    e.setParameter(ParamId::VibratoDepth, 50.0f); e.setParameter(ParamId::VibratoRate, 5.0f);
    noteOn(e, 1, 69, 100); auto y = render(e, 48000, 4.0, 128);
    std::vector<float> seg(y.begin() + 48000, y.end());
    auto tr = cstest::pitchTrack(seg, 48000, 2048, 256, 380, 510);
    double mn = 1e9, mx = 0, mean = 0; for (double v : tr) { mn = std::min(mn, v); mx = std::max(mx, v); mean += v; } mean /= (double) tr.size();
    double pp = cstest::cents(mx, mn); REQUIRE(pp >= 80.0); REQUIRE(pp <= 120.0);
    std::vector<double> sm(tr.size(), mean);                       // 5-point moving average suppresses bow-noise crossings
    for (std::size_t k = 2; k + 2 < tr.size(); ++k) sm[k] = (tr[k - 2] + tr[k - 1] + tr[k] + tr[k + 1] + tr[k + 2]) / 5.0;
    int crossings = 0; for (std::size_t k = 3; k + 2 < sm.size(); ++k) if ((sm[k - 1] - mean) * (sm[k] - mean) < 0) ++crossings;
    double rate = crossings / 2.0 / ((double) (sm.size() - 5) * 256.0 / 48000.0); REQUIRE(rate >= 4.5); REQUIRE(rate <= 5.5);
}

TEST_CASE("T-037 pitch bend: range 2, full up-bend -> +200 cents within 3 cents", "[integration][SC-08]") {
    Engine e; e.prepare(48000, 128); e.setInstrument(InstrumentId::Erhu); e.setParameter(ParamId::PitchBendRange, 2.0f);
    noteOn(e, 1, 69, 100); bend(e, 1, 16383); auto y = render(e, 48000, 1.5, 128);
    double target = 440.0 * std::pow(2.0, 2.0 / 12.0) * std::pow(2.0, (8191.0 / 8192.0 - 1.0) * 2.0 / 12.0);
    REQUIRE(std::fabs(cstest::cents(cstest::peakFrequency(y, 48000, 24000, 48000, 450, 540), target)) <= 3.0);
}

TEST_CASE("T-038 MPE: per-channel bend moves only its own note", "[integration][SC-09][D-015]") {
    Engine e; e.prepare(48000, 128); e.setInstrument(InstrumentId::Guzheng); e.setMpeEnabled(true); e.setParameter(ParamId::PitchBendRange, 48.0f);
    noteOn(e, 2, 69, 100); noteOn(e, 3, 76, 100); render(e, 48000, 0.05, 128);
    bend(e, 2, 8192 + (int) std::lround(8192.0 / 48.0)); render(e, 48000, 0.1, 128);
    VoiceInfo v[4]{}; int n = e.voiceInfo(v, 4); REQUIRE(n == 2);
    for (int k = 0; k < n; ++k) {
        if (v[k].channel == 2) REQUIRE(std::fabs(cstest::cents(v[k].currentHz, 440.0 * std::pow(2.0, 1.0 / 12.0))) <= 3.0);
        else { REQUIRE(v[k].channel == 3); REQUIRE(std::fabs(cstest::cents(v[k].currentHz, 659.255)) <= 1.0); }
    }
}

TEST_CASE("T-039 sample-accurate note-on", "[integration][SC-08]") {
    Engine e; e.prepare(48000, 512); e.setInstrument(InstrumentId::Pipa);
    noteOn(e, 1, 57, 127, 300); std::vector<float> L(512), R(512); e.render(L.data(), R.data(), 512);
    int first = -1; for (int i = 0; i < 512; ++i) if (std::fabs(L[(std::size_t) i]) > 1e-7f) { first = i; break; }
    REQUIRE(first >= 300); REQUIRE(first < 364);
}

TEST_CASE("T-040 tuning tables apply to the engine (A4 = 432 Hz)", "[integration][SC-10]") {
    Engine e; e.prepare(48000, 128); e.setInstrument(InstrumentId::Erhu); e.setTuning(TuningTable::equal(432.0));
    noteOn(e, 1, 69, 100); auto y = render(e, 48000, 1.5, 128);
    REQUIRE(std::fabs(cstest::cents(cstest::peakFrequency(y, 48000, 24000, 48000, 400, 460), 432.0)) <= 3.0);
}

TEST_CASE("T-041 operational: invalid prepare arguments throw; unprepared engine renders silence", "[operational][SC-16]") {
    Engine e;
    REQUIRE_THROWS_AS(e.prepare(8000, 128), std::invalid_argument);
    REQUIRE_THROWS_AS(e.prepare(48000, 0), std::invalid_argument);
    REQUIRE_THROWS_AS(e.prepare(384000, 128), std::invalid_argument);
    Engine u; std::vector<float> L(64, 1.f), R(64, 1.f); noteOn(u, 1, 60, 100); u.render(L.data(), R.data(), 64);
    for (int i = 0; i < 64; ++i) { REQUIRE(L[(std::size_t) i] == 0.0f); REQUIRE(R[(std::size_t) i] == 0.0f); }
}

TEST_CASE("T-042 UI fingering map (D-021)", "[unit][SC-17][A-008]") {
    auto f = fingeringFor(InstrumentId::Erhu, 440.0); REQUIRE(f.stringIndex == 1); REQUIRE(f.stopPosition01 == 0.0);
    f = fingeringFor(InstrumentId::Erhu, 880.0); REQUIRE(f.stringIndex == 1); REQUIRE(std::fabs(f.stopPosition01 - 0.5) < 1e-9);
    f = fingeringFor(InstrumentId::Erhu, 329.628); REQUIRE(f.stringIndex == 0); REQUIRE(std::fabs(f.stopPosition01 - (1 - 293.665 / 329.628)) < 1e-4);
    const auto& g = spec(InstrumentId::Guzheng);
    for (int k = 0; k < (int) g.strings.size(); ++k) { auto z = fingeringFor(InstrumentId::Guzheng, g.strings[(std::size_t) k].openHz); REQUIRE(z.stringIndex == k); REQUIRE(z.stopPosition01 == 0.0); }
    auto z = fingeringFor(InstrumentId::Guzheng, 146.832 * std::pow(2.0, 0.4 / 12.0)); REQUIRE(z.stringIndex == 5);  // nearest to D3 (index 5)
}

TEST_CASE("T-043 operational: re-prepare mid-note (resume), instrument switch while sounding, destruction with active voices", "[operational][SC-16]") {
    for (int i = 0; i < kNumInstruments; ++i) {
        auto* e = new Engine(); e->prepare(48000, 256); e->setInstrument(idx(i)); const auto& s = spec(idx(i));
        noteOn(*e, 1, s.midiLow, 100); render(*e, 48000, 0.2, 256);
        e->prepare(96000, 512);                                   // host changes rate: all voices reset, no crash
        REQUIRE(e->activeVoiceCount() == 0);
        auto y = render(*e, 96000, 0.1, 512); REQUIRE(cstest::allFinite(y)); REQUIRE(cstest::peakAbs(y) == 0.0f);
        noteOn(*e, 1, s.midiLow, 100); render(*e, 96000, 0.1, 512);
        e->setInstrument(idx((i + 1) % kNumInstruments));         // switch while sounding: voices reset
        REQUIRE(e->activeVoiceCount() == 0);
        noteOn(*e, 1, spec(idx((i + 1) % kNumInstruments)).midiLow, 100); y = render(*e, 96000, 0.2, 512); REQUIRE(cstest::allFinite(y));
        REQUIRE_NOTHROW(delete e);                                // destruction with active voices
    }
}
