// Thread-safe host wrapper (D-024): off-thread instrument construction + atomic publish, parameter atomics, seqlock voice snapshot.
#include "cs/EngineHost.h"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
namespace cs {
namespace { constexpr int kRetire = 64, kSnap = 16; }
struct EngineHost::Impl {
    double fs = 48000; int block = 512; bool prepared = false;
    std::atomic<Engine*> current{nullptr}, pending{nullptr}; std::atomic<Engine*> retired[kRetire];
    std::atomic<int> curInst{0}; std::atomic<float> params[kNumParams]; std::atomic<bool> mpe{false};
    float applied[kNumParams]; bool mpeApplied = false;                       // audio thread only
    std::atomic<std::uint64_t> seq{0}; std::atomic<int> snapCount{0}; std::atomic<std::uint64_t> words[kSnap][4];
    Impl() {
        for (auto& r : retired) r.store(nullptr);
        for (int p = 0; p < kNumParams; ++p) { params[p].store(paramRange(static_cast<ParamId>(p)).def); applied[p] = params[p].load(); }
        for (auto& w : words) for (auto& x : w) x.store(0);
    }
    Engine* build(InstrumentId id) {
        auto* e = new Engine(); e->setInstrument(id); e->prepare(fs, block);
        for (int p = 0; p < kNumParams; ++p) e->setParameter(static_cast<ParamId>(p), params[p].load(std::memory_order_relaxed));
        e->setMpeEnabled(mpe.load(std::memory_order_relaxed)); return e;
    }
    void gc() { for (auto& r : retired) delete r.exchange(nullptr); }
};
EngineHost::EngineHost() : impl_(new Impl) {}
EngineHost::~EngineHost() { auto& I = *impl_; delete I.current.exchange(nullptr); delete I.pending.exchange(nullptr); I.gc(); }
void EngineHost::prepare(double sampleRate, int maxBlockSize) {
    auto& I = *impl_; I.fs = sampleRate; I.block = maxBlockSize;
    const auto id = static_cast<InstrumentId>(I.curInst.load());
    Engine* e = I.build(id);                                   // throws std::invalid_argument for bad arguments, before anything changes
    delete I.pending.exchange(nullptr); delete I.current.exchange(e); I.gc(); I.prepared = true;
    for (int p = 0; p < kNumParams; ++p) I.applied[p] = I.params[p].load(); I.mpeApplied = I.mpe.load();
}
void EngineHost::requestInstrument(InstrumentId id) {
    auto& I = *impl_; if (!I.prepared) { I.curInst.store((int) id); return; }
    I.gc(); Engine* e = I.build(id); delete I.pending.exchange(e);
}
InstrumentId EngineHost::currentInstrument() const noexcept { return static_cast<InstrumentId>(impl_->curInst.load()); }
void EngineHost::setParameter(ParamId p, float value) noexcept {
    const int i = (int) p; if (i < 0 || i >= kNumParams) return; const auto r = paramRange(p);
    impl_->params[i].store(std::isfinite(value) ? std::min(std::max(value, r.min), r.max) : r.def, std::memory_order_relaxed);
}
void EngineHost::setMpeEnabled(bool on) noexcept { impl_->mpe.store(on, std::memory_order_relaxed); }
void EngineHost::handleMidi(const std::uint8_t* b, int size, int off) noexcept { if (Engine* e = impl_->current.load(std::memory_order_acquire)) e->handleMidi(b, size, off); }
void EngineHost::render(float* l, float* r, int n) noexcept {
    auto& I = *impl_; if (n <= 0) return;
    if (I.pending.load(std::memory_order_relaxed) != nullptr) {
        if (Engine* p = I.pending.exchange(nullptr, std::memory_order_acq_rel)) {
            Engine* old = I.current.load(std::memory_order_relaxed); bool retiredOk = false;
            for (auto& slot : I.retired) { Engine* expected = nullptr; if (slot.compare_exchange_strong(expected, old)) { retiredOk = true; break; } }
            if (retiredOk) {
                I.current.store(p, std::memory_order_release); I.curInst.store((int) p->instrument(), std::memory_order_release);
                for (int k = 0; k < kNumParams; ++k) I.applied[k] = std::nanf(""); I.mpeApplied = !I.mpe.load();     // force re-apply to the new engine
            }                                                                                                       // (ring full is impossible: every request collects first)
        }
    }
    Engine* e = I.current.load(std::memory_order_acquire);
    if (!e) { std::memset(l, 0, sizeof(float) * (std::size_t) n); std::memset(r, 0, sizeof(float) * (std::size_t) n); return; }
    for (int k = 0; k < kNumParams; ++k) { const float v = I.params[k].load(std::memory_order_relaxed); if (!(v == I.applied[k])) { e->setParameter(static_cast<ParamId>(k), v); I.applied[k] = v; } }
    const bool m = I.mpe.load(std::memory_order_relaxed); if (m != I.mpeApplied) { e->setMpeEnabled(m); I.mpeApplied = m; }
    e->render(l, r, n);
    VoiceInfo tmp[kSnap]; const int cnt = e->voiceInfo(tmp, kSnap);
    I.seq.fetch_add(1, std::memory_order_acq_rel);
    for (int k = 0; k < cnt; ++k) {
        std::uint64_t w2, w3; std::memcpy(&w2, &tmp[k].stopPosition01, 8); std::memcpy(&w3, &tmp[k].currentHz, 8);
        I.words[k][0].store((std::uint64_t) (std::uint32_t) tmp[k].midiNote | ((std::uint64_t) (std::uint32_t) tmp[k].channel << 32), std::memory_order_relaxed);
        I.words[k][1].store((std::uint64_t) (std::uint32_t) tmp[k].stringIndex, std::memory_order_relaxed);
        I.words[k][2].store(w2, std::memory_order_relaxed); I.words[k][3].store(w3, std::memory_order_relaxed);
    }
    I.snapCount.store(cnt, std::memory_order_relaxed); I.seq.fetch_add(1, std::memory_order_release);
}
int EngineHost::voiceSnapshot(VoiceInfo* out, int maxCount) const noexcept {
    auto& I = *impl_; if (!out || maxCount <= 0) return 0; VoiceInfo tmp[kSnap]; int cnt = 0;
    for (int tries = 0; tries < 100000; ++tries) {
        const std::uint64_t s1 = I.seq.load(std::memory_order_acquire); if (s1 & 1u) continue;
        cnt = std::min(I.snapCount.load(std::memory_order_relaxed), kSnap);
        for (int k = 0; k < cnt; ++k) {
            const std::uint64_t w0 = I.words[k][0].load(std::memory_order_relaxed), w1 = I.words[k][1].load(std::memory_order_relaxed), w2 = I.words[k][2].load(std::memory_order_relaxed), w3 = I.words[k][3].load(std::memory_order_relaxed);
            tmp[k].midiNote = (int) (std::uint32_t) w0; tmp[k].channel = (int) (std::uint32_t) (w0 >> 32); tmp[k].stringIndex = (int) (std::uint32_t) w1;
            std::memcpy(&tmp[k].stopPosition01, &w2, 8); std::memcpy(&tmp[k].currentHz, &w3, 8);
        }
        std::atomic_thread_fence(std::memory_order_acquire);
        if (I.seq.load(std::memory_order_relaxed) == s1) { const int n = std::min(cnt, maxCount); for (int k = 0; k < n; ++k) out[k] = tmp[k]; return n; }
    }
    return 0;
}
void EngineHost::collectGarbage() { impl_->gc(); }
}
