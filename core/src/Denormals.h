// Flush-to-zero / denormals-are-zero for the duration of a scope (D-024). Restores the previous state.
#pragma once
#include <cstdint>
#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86)
  #include <xmmintrin.h>
#endif
namespace cs::detail {
class ScopedFlushDenormals {
public:
    ScopedFlushDenormals() noexcept {
#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86)
        saved_ = _mm_getcsr(); _mm_setcsr(saved_ | 0x8040u);                 // FTZ (bit 15) | DAZ (bit 6)
#elif defined(__aarch64__)
        std::uint64_t v; asm volatile("mrs %0, fpcr" : "=r"(v)); saved_ = v; asm volatile("msr fpcr, %0" : : "r"(v | (1ull << 24)));   // FZ
#endif
    }
    ~ScopedFlushDenormals() {
#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86)
        _mm_setcsr((unsigned) saved_);
#elif defined(__aarch64__)
        asm volatile("msr fpcr, %0" : : "r"(saved_));
#endif
    }
    ScopedFlushDenormals(const ScopedFlushDenormals&) = delete; ScopedFlushDenormals& operator=(const ScopedFlushDenormals&) = delete;
private:
    std::uint64_t saved_ = 0;
};
}
