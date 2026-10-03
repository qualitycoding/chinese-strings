#include "cs/Dsp.h"
#include <cmath>
namespace cs::dsp {
// Derivation (plan C-062, corrected in S-008): friction f = F mu(|v|) sgn(v); wave impedance Z on EACH side of the bow point.
//   Outgoing waves = incoming wave from the opposite side + f/(2Z);  string velocity = (sum of incoming) + f/(2Z);
//   relative velocity v = vDelta+ - f/(2Z) with vDelta+ = vBow - (sum of incoming).   Let a = |vDelta+|, q = F/(2Z):
//   g(v) = v + q mu(v) = a.  Sticking (v = 0) when a <= q muS  ->  rho_hat -> 1 (returned as 1 - 2^-30 so that rho_hat < 1 always).
//   Slipping: smallest root v of g(v) = a in (0, a];  rho_hat = f/(2Z) / a = (a - v) / a.
double BowJunction::reflectionCoefficient(double vDeltaPlus, double bowForceN, double waveImpedance, const FrictionParams& p) {
    constexpr double kStick = 1.0 - 9.313225746154785e-10;           // 1 - 2^-30
    if (!(bowForceN > 0.0) || !(waveImpedance > 0.0)) return 0.0;
    const double a = std::fabs(vDeltaPlus), q = bowForceN / (2.0 * waveImpedance);
    if (!std::isfinite(a)) return 0.0;
    if (a <= q * p.muS) return kStick;
    auto g = [&](double v) { return v + q * (p.muD + (p.muS - p.muD) * p.v0 / (p.v0 + v)); };
    const int steps = 32; double lo = 0.0, hi = a;
    for (int i = 1; i <= steps; ++i) { const double v = a * (double) i / steps; if (g(v) >= a) { hi = v; break; } lo = v; }
    for (int it = 0; it < 64; ++it) { const double mid = 0.5 * (lo + hi); if (g(mid) >= a) hi = mid; else lo = mid; }
    const double v = 0.5 * (lo + hi), r = (a - v) / a;
    return r < 0.0 ? 0.0 : (r > kStick ? kStick : r);
}
HuntCrossleyContact::HuntCrossleyContact(double K, double a, double b) : K_(K), alpha_(a), beta_(b) {}
double HuntCrossleyContact::force(double x, double xdot) const {
    if (!(x > 0.0)) return 0.0;
    const double f = K_ * std::pow(x, alpha_) * (1.0 + beta_ * xdot);
    return f > 0.0 ? f : 0.0;
}
}
