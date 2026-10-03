#include "cs/Dsp.h"
#include <cmath>
namespace cs::dsp {
// Derivation (see plan C-062): friction f = F mu(|v|) sgn(v); string impedance Z per side.
//   v + f/Z = vDelta+   (v = bow-string relative velocity),  outgoing waves gain f/(2Z) = rho_hat * vDelta+.
// With a = |vDelta+|:  g(v) = v + (F/Z) mu(v) = a.  Sticking (v = 0) when a <= (F/Z) muS  -> rho_hat = 1/2.
// Slipping: smallest root v of g(v) = a in (0, a];  rho_hat = (a - v) / (2 a)  in [0, 1/2].
double BowJunction::reflectionCoefficient(double vDeltaPlus, double bowForceN, double waveImpedance, const FrictionParams& p) {
    if (!(bowForceN > 0.0) || !(waveImpedance > 0.0)) return 0.0;
    const double a = std::fabs(vDeltaPlus), q = bowForceN / waveImpedance;
    if (!std::isfinite(a)) return 0.0;
    if (a <= q * p.muS) return 0.5;
    auto g = [&](double v) { return v + q * (p.muD + (p.muS - p.muD) * p.v0 / (p.v0 + v)); };
    const int steps = 32; double lo = 0.0, hi = a;
    for (int i = 1; i <= steps; ++i) { const double v = a * (double) i / steps; if (g(v) >= a) { hi = v; break; } lo = v; }
    for (int it = 0; it < 64; ++it) { const double mid = 0.5 * (lo + hi); if (g(mid) >= a) hi = mid; else lo = mid; }
    const double v = 0.5 * (lo + hi), r = (a - v) / (2.0 * a);
    return r < 0.0 ? 0.0 : (r > 0.5 ? 0.5 : r);
}
HuntCrossleyContact::HuntCrossleyContact(double K, double a, double b) : K_(K), alpha_(a), beta_(b) {}
double HuntCrossleyContact::force(double x, double xdot) const {
    if (!(x > 0.0)) return 0.0;
    const double f = K_ * std::pow(x, alpha_) * (1.0 + beta_ * xdot);
    return f > 0.0 ? f : 0.0;
}
}
