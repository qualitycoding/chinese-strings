#include "cs/Dsp.h"
#include <algorithm>
#include <cmath>
namespace cs::dsp {
void ModalBank::configure(double fs, const std::vector<ModeSpec>& modes) {
    const std::size_t n = modes.size(); b0_.assign(n, 0.0); a1_.assign(n, 0.0); a2_.assign(n, 0.0); z1_.assign(n, 0.0); z2_.assign(n, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        const double r = std::pow(10.0, -3.0 / (fs * std::max(modes[i].t60s, 1e-4))), th = 2.0 * M_PI * modes[i].freqHz / fs;
        a1_[i] = -2.0 * r * std::cos(th); a2_[i] = r * r; b0_[i] = modes[i].gain * (1.0 - r);   // y = b0 x - a1 y1 - a2 y2
    }
}
float ModalBank::process(float x) noexcept {
    double sum = 0.0; const std::size_t n = b0_.size();
    for (std::size_t i = 0; i < n; ++i) {
        const double y = b0_[i] * (double) x - a1_[i] * z1_[i] - a2_[i] * z2_[i];
        z2_[i] = z1_[i]; z1_[i] = y; sum += y;
    }
    return (float) sum;
}
void ModalBank::reset() noexcept { std::fill(z1_.begin(), z1_.end(), 0.0); std::fill(z2_.begin(), z2_.end(), 0.0); }
}
