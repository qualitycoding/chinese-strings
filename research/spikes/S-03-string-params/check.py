"""Spike S-03: self-consistency of published string parameters.
For an ideal string f0 = c / (2 Lv), c = sqrt(T / (rho*A)). Stiffness correction:
f0' = f0 * sqrt(1 + B), B = pi^3 E r^4 / (4 T Lv^2) (Fletcher & Rossing, eq. for stiff string, n=1; here used only as a size check).
Inputs are copied verbatim from the cited tables. Output: implied vibrating length Lv for the nominal pitch."""
import math
A4 = 440.0
def note(n): return A4 * 2 ** ((n - 69) / 12)
cases = [
 # (label, source, rho kg/m3, radius m, T N, E Pa, nominal Hz)
 ("erhu inner D4", "Samejima 2023 Table 1 (diam 0.440 mm)", 6577, 0.440e-3/2, 51.68, 200e9, note(62)),
 ("erhu outer A4", "Samejima 2023 Table 1 (diam 0.260 mm)", 8363, 0.260e-3/2, 51.50, 200e9, note(69)),
 ("yehu s1 F4",    "Zheng et al. 2026 Table 1 (r 0.55 mm)", 1250, 0.55e-3, 50.61, 1.0e10, note(65)),
 ("yehu s2 C5",    "Zheng et al. 2026 Table 1 (r 0.40 mm)", 1350, 0.40e-3, 64.29, 9.5e9, note(72)),
]
for lab, src, rho, r, T, E, f in cases:
    A = math.pi * r * r; c = math.sqrt(T / (rho * A)); Lv = c / (2 * f)
    B = math.pi ** 3 * E * r ** 4 / (4 * T * Lv ** 2)
    print(f"{lab:14s} f0={f:8.2f} Hz  c={c:7.1f} m/s  implied Lv={Lv*1000:6.1f} mm  B={B:.2e}  stiffness shift={600*math.log2(1+B):.2f} cents  [{src}]")
