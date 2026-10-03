#include "cs/Engine.h"
#include "Denormals.h"
#include "InstrumentInternal.h"
#include "Voices.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstring>
#include <stdexcept>
namespace cs {
using namespace detail;
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr int kQueue = 512, kCtrlBlock = 32;
constexpr double kBodyMix = 0.35;                     // body-mode colouring added to the dry bridge signal (DD)
constexpr double kRadiationWet = 0.6;                 // yehu/banhu: share of the measured radiation response in the output; the rest is the dry body signal (DD; S-008)
constexpr double kRadiationTrim = 0.8;                // level trim for radiated instruments (DD; S-008: keeps yehu/banhu peaks <= -3 dBFS)
struct Event { int sample; std::uint8_t status, d1, d2; };

// Parallel second-order radiation sections (S-07), re-derived for the engine sample rate by impulse invariance.
struct Radiation {
    struct Sec { double b0, b1, a1, a2, z1 = 0, z2 = 0; };
    bool on = false; double fir = 0, norm = 1; std::vector<Sec> secs;
    void configure(const RadiationSpec* rs, double fs, double loHz) {
        secs.clear(); on = rs != nullptr; if (!on) return;
        const double k = rs->designSampleRate / fs; fir = rs->fir;
        for (const auto& s : rs->sections) {
            using C = std::complex<double>; const double a1 = s.a[1], a2 = s.a[2];
            const C disc = std::sqrt(C(a1 * a1 - 4.0 * a2, 0.0)); const C p1 = (-a1 + disc) / 2.0, p2 = (-a1 - disc) / 2.0;
            const C r1 = (s.b[0] * p1 + s.b[1]) / (p1 - p2), r2 = (s.b[0] * p2 + s.b[1]) / (p2 - p1);
            const C q1 = std::exp(k * std::log(p1)), q2 = std::exp(k * std::log(p2)); const C g1 = k * r1, g2 = k * r2;
            Sec o; o.a1 = (-(q1 + q2)).real(); o.a2 = (q1 * q2).real(); o.b0 = (g1 + g2).real(); o.b1 = (-(g1 * q2 + g2 * q1)).real(); secs.push_back(o);
        }
        double acc = 0; int cnt = 0;                         // normalise the mean power over the instrument's playing range .. 6 kHz (log grid) to unity: the measured yehu response
        for (int i = 0; i < 200; ++i) {                      // spans ~47 dB, so a peak-based norm would leave ordinary notes ~35 dB down (S-008 finding)
            const double f = loHz * std::pow(6000.0 / loHz, i / 199.0); if (f > 0.45 * fs) break;
            const std::complex<double> z1 = std::polar(1.0, -2.0 * kPi * f / fs), z2 = z1 * z1; std::complex<double> h = fir;
            for (const auto& s : secs) h += (s.b0 + s.b1 * z1) / (1.0 + s.a1 * z1 + s.a2 * z2);
            acc += std::norm(h); ++cnt;
        }
        norm = cnt > 0 && acc > 1e-18 ? 1.0 / std::sqrt(acc / cnt) : 1.0;
    }
    float process(float x) noexcept {
        double y = fir * (double) x;
        for (auto& s : secs) { const double o = s.b0 * x + s.z1; s.z1 = s.b1 * x - s.a1 * o + s.z2; s.z2 = -s.a2 * o; y += o; }
        return (float) (y * norm);
    }
    void reset() noexcept { for (auto& s : secs) s.z1 = s.z2 = 0.0; }
};
}  // namespace

struct Engine::Impl {
    double fs = 0; int maxBlock = 0; bool prepared = false;
    InstrumentId inst = InstrumentId::Erhu; const InstrumentSpec* sp = &spec(InstrumentId::Erhu);
    float params[kNumParams]; TuningTable tuning = TuningTable::equal(); bool mpe = false; Technique tech = Technique::Default;
    std::shared_ptr<const Designs> designs; std::vector<std::unique_ptr<Voice>> voices;
    dsp::ModalBank body; Radiation rad; std::vector<float> mono;
    Event q[kQueue]; int qn = 0; std::uint64_t ageCtr = 0; float bend[17] = {}; float chanPress[17] = {}; float cc1 = 1.0f;
    double lfoPhase = 0, gainNow = 0.8; VoiceParams vp;

    Impl() { for (int p = 0; p < kNumParams; ++p) params[p] = paramRange(static_cast<ParamId>(p)).def; gainNow = params[(int) ParamId::Gain]; }
    void build() {
        designs = designsFor(inst, fs); voices.clear();
        const int nv = std::max(kFreePolyphonyVoices, sp->maxPolyphony);
        for (int i = 0; i < nv; ++i) { auto v = makeVoice(sp->family); v->prepare(fs, *sp, designs); voices.push_back(std::move(v)); }
        body.configure(fs, sp->bodyModes); rad.configure(radiationFor(inst), fs, 440.0 * std::pow(2.0, (sp->midiLow - 69) / 12.0));
        mono.assign((std::size_t) std::max(maxBlock, kCtrlBlock) + 8, 0.0f); qn = 0; tech = Technique::Default; lfoPhase = 0;
        std::memset(bend, 0, sizeof(bend)); std::memset(chanPress, 0, sizeof(chanPress)); cc1 = 1.0f; gainNow = params[(int) ParamId::Gain];
    }
    void resetAll() noexcept { for (auto& v : voices) v->reset(); body.reset(); rad.reset(); }
    void syncParams() noexcept {
        vp.bowPressure = params[(int) ParamId::BowPressure]; vp.bowSpeed = params[(int) ParamId::BowSpeed];
        vp.pluckPosition = params[(int) ParamId::PluckPosition]; vp.brightness = params[(int) ParamId::Brightness]; vp.tech = tech;
    }
    int voiceLimit() const noexcept { return params[(int) ParamId::FreePolyphony] >= 0.5f ? kFreePolyphonyVoices : std::min(sp->maxPolyphony, (int) voices.size()); }
    double targetHz(const Voice& v) const noexcept {
        float b = bend[std::min(std::max(v.channel, 1), 16)]; if (mpe && v.channel >= 2) b += bend[1];
        const double semis = (double) b * params[(int) ParamId::PitchBendRange];
        const double vib = (double) params[(int) ParamId::VibratoDepth] * cc1 * std::sin(lfoPhase);
        return tuning.hz[(std::size_t) v.note] * std::pow(2.0, semis / 12.0 + vib / 1200.0);
    }
    int allocate(int limit) noexcept {
        int act = 0; for (auto& v : voices) act += v->active ? 1 : 0;
        if (act < limit) { for (std::size_t i = 0; i < voices.size(); ++i) if (!voices[i]->active) return (int) i; }
        int pick = 0; bool pickRel = false; std::uint64_t pickAge = ~0ull; bool have = false;
        for (std::size_t i = 0; i < voices.size(); ++i) {
            const auto& v = *voices[i]; if (!v.active) continue;
            if (!have || (v.releasing && !pickRel) || (v.releasing == pickRel && v.age < pickAge)) { pick = (int) i; pickRel = v.releasing; pickAge = v.age; have = true; }
        }
        return pick;
    }
    void noteOn(int ch, int note, int vel) noexcept {
        if (note >= kKeyswitchLow && note < kKeyswitchLow + kNumTechniques) { const auto t = static_cast<Technique>(note - kKeyswitchLow); tech = supportsTechnique(inst, t) ? t : Technique::Default; return; }
        if (note < sp->midiLow || note > sp->midiHigh) return;
        syncParams();
        if (voiceLimit() == 1 && !voices.empty() && voices[0]->supportsLegato() && voices[0]->active && !voices[0]->releasing) {   // mono legato (D-022)
            Voice& v = *voices[0]; v.note = note; v.channel = ch; v.age = ++ageCtr; v.legato(note, targetHz(v), (float) vel / 127.0f, vp); return;
        }
        const int idx = allocate(std::max(1, voiceLimit())); Voice& v = *voices[(std::size_t) idx];
        v.reset(); v.note = note; v.channel = ch; v.age = ++ageCtr; v.active = true; v.releasing = false;
        v.noteOn(note, targetHz(v), (float) vel / 127.0f, vp); v.active = true;
    }
    void noteOff(int ch, int note) noexcept {
        for (auto& v : voices) if (v->active && !v->releasing && v->note == note && (!mpe || v->channel == ch)) { v->release(); v->releasing = true; return; }
    }
    void apply(const Event& e) noexcept {
        const int st = e.status & 0xF0, ch = (e.status & 0x0F) + 1;
        switch (st) {
            case 0x90: if (e.d2 > 0) noteOn(ch, e.d1, e.d2); else noteOff(ch, e.d1); break;
            case 0x80: noteOff(ch, e.d1); break;
            case 0xE0: bend[ch] = ((float) (e.d1 | (e.d2 << 7)) - 8192.0f) / 8192.0f; break;
            case 0xB0: if (e.d1 == 1) cc1 = (float) e.d2 / 127.0f; break;
            case 0xD0: chanPress[ch] = (float) e.d1 / 127.0f; break;
            default: break;
        }
    }
    void renderSegment(float* l, float* r, int len) noexcept {
        syncParams();
        for (auto& v : voices) if (v->active) v->setHz(targetHz(*v));
        float* m = mono.data(); std::memset(m, 0, sizeof(float) * (std::size_t) len);
        for (auto& v : voices) if (v->active) v->render(m, len, vp);
        const double gt = params[(int) ParamId::Gain], gc = 1.0 - std::exp(-1.0 / (0.01 * fs)); bool bad = false;
        for (int i = 0; i < len; ++i) {
            float x = m[i]; float y = x + (float) kBodyMix * body.process(x); if (rad.on) y = (float) (kRadiationTrim * ((1.0 - kRadiationWet) * (double) y + kRadiationWet * (double) rad.process(y)));
            gainNow += (gt - gainNow) * gc; float o = (float) (gainNow * y);
            if (!std::isfinite(o)) { bad = true; o = 0.0f; }
            const float a = std::fabs(o); if (a > 2.0f) o = std::copysign(2.0f + std::tanh(a - 2.0f), o);
            l[i] = o; r[i] = o;
        }
        for (auto& v : voices) v->active = v->active;      // (voices deactivate themselves when their tails end)
        lfoPhase += 2.0 * kPi * (double) params[(int) ParamId::VibratoRate] * len / fs; if (lfoPhase > 2.0 * kPi) lfoPhase = std::fmod(lfoPhase, 2.0 * kPi);
        if (bad) resetAll();
    }
};

Engine::Engine() : impl_(new Impl) {}
Engine::~Engine() = default;
void Engine::prepare(double sampleRate, int maxBlockSize) {
    if (!(sampleRate >= 22050.0 && sampleRate <= 192000.0) || maxBlockSize < 1 || maxBlockSize > 8192) throw std::invalid_argument("Engine::prepare: sampleRate must be in [22050, 192000] and maxBlockSize in [1, 8192]");
    impl_->fs = sampleRate; impl_->maxBlock = maxBlockSize; impl_->build(); impl_->prepared = true;
}
void Engine::setInstrument(InstrumentId id) { impl_->inst = id; impl_->sp = &spec(id); if (impl_->prepared) impl_->build(); else impl_->tech = Technique::Default; }
InstrumentId Engine::instrument() const noexcept { return impl_->inst; }
void Engine::setParameter(ParamId p, float value) noexcept {
    const int i = (int) p; if (i < 0 || i >= kNumParams) return; const auto r = paramRange(p);
    impl_->params[i] = std::isfinite(value) ? std::min(std::max(value, r.min), r.max) : r.def;
}
float Engine::parameter(ParamId p) const noexcept { const int i = (int) p; return i >= 0 && i < kNumParams ? impl_->params[i] : 0.0f; }
void Engine::setTuning(const TuningTable& t) noexcept { impl_->tuning = t; }
void Engine::setMpeEnabled(bool on) noexcept { impl_->mpe = on; }
void Engine::handleMidi(const std::uint8_t* b, int size, int off) noexcept {
    if (!impl_->prepared || b == nullptr || size < 2) return;
    const std::uint8_t st = b[0]; const int kind = st & 0xF0; if (kind < 0x80 || kind > 0xE0) return;
    if ((kind == 0xC0 || kind == 0xD0) ? size < 2 : size < 3) return;
    Event e{std::max(off, 0), st, (std::uint8_t) (b[1] & 0x7F), (std::uint8_t) (size > 2 ? (b[2] & 0x7F) : 0)};
    auto& I = *impl_;
    if (I.qn == kQueue) { std::memmove(&I.q[0], &I.q[1], sizeof(Event) * (kQueue - 1)); --I.qn; }       // overflow drops the oldest
    int pos = I.qn; while (pos > 0 && I.q[pos - 1].sample > e.sample) { I.q[pos] = I.q[pos - 1]; --pos; } I.q[pos] = e; ++I.qn;
}
void Engine::render(float* left, float* right, int numSamples) noexcept {
    if (numSamples <= 0) return;
    auto& I = *impl_;
    if (!I.prepared || numSamples > I.maxBlock) { std::memset(left, 0, sizeof(float) * (std::size_t) numSamples); std::memset(right, 0, sizeof(float) * (std::size_t) numSamples); I.qn = 0; return; }
    ScopedFlushDenormals ftz; int pos = 0, ei = 0;
    while (pos < numSamples) {
        while (ei < I.qn && I.q[ei].sample <= pos) { I.apply(I.q[ei]); ++ei; }
        int next = std::min(numSamples, pos + kCtrlBlock); if (ei < I.qn) next = std::min(next, std::max(I.q[ei].sample, pos + 1));
        I.renderSegment(left + pos, right + pos, next - pos); pos = next;
    }
    int keep = 0; for (int k = ei; k < I.qn; ++k) { I.q[keep] = I.q[k]; I.q[keep].sample = std::max(0, I.q[keep].sample - numSamples); ++keep; } I.qn = keep;
}
Technique Engine::technique() const noexcept { return impl_->tech; }
int Engine::activeVoiceCount() const noexcept { int n = 0; for (const auto& v : impl_->voices) n += v->active ? 1 : 0; return n; }
int Engine::voiceInfo(VoiceInfo* out, int maxCount) const noexcept {
    int n = 0; if (!out) return 0;
    for (const auto& v : impl_->voices) { if (!v->active || n >= maxCount) continue; out[n++] = VoiceInfo{v->note, v->channel, v->stringIdx, v->stop, v->hz}; }
    return n;
}
}
