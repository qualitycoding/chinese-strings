#include "cs/Dsp.h"
#include <algorithm>
#include <cmath>
namespace cs::dsp {
void FractionalDelayLine::prepare(int maxDelaySamples) {
    std::size_t n = 4; const std::size_t need = (std::size_t) std::max(maxDelaySamples, 2) + 4;
    while (n < need) n <<= 1;
    buf_.assign(n, 0.0f); mask_ = n - 1; w_ = 0; xp_ = yp_ = 0; n_ = 0; d_ = 1; a_ = 0;
}
void FractionalDelayLine::setDelay(double samples) {
    const double maxD = (double) (mask_ > 4 ? mask_ - 3 : 1);
    d_ = std::min(std::max(samples, 1.0), maxD);
    n_ = (int) std::floor(d_ - 0.5);                // total = n_ + delta, delta in [0.5, 1.5)
    const double delta = d_ - n_;
    a_ = (1.0 - delta) / (1.0 + delta);              // first-order Thiran allpass
}
float FractionalDelayLine::process(float x) noexcept {
    buf_[w_ & mask_] = x;
    const double u = buf_[(w_ - (std::size_t) n_) & mask_];
    ++w_;
    const double y = a_ * u + xp_ - a_ * yp_;        // H(z) = (a + z^-1) / (1 + a z^-1)
    xp_ = u; yp_ = y;
    return (float) y;
}
void FractionalDelayLine::reset() noexcept { std::fill(buf_.begin(), buf_.end(), 0.0f); xp_ = yp_ = 0.0; }
}
