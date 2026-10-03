"""Spike S-06: analytic decay oracle for strings whose loss model is the frequency-dependent
damping of Zheng et al. (DAFx26) eq. (2): rho*A*(u_tt + 2*s0*u_t - 2*s1*u_txx) + ... (stiff string).
For mode n (simply supported, length Lv), wavenumber beta_n = n*pi/Lv. The modal amplitude decays as
exp(-(s0 + s1*beta_n**2) t) (loss terms are diagonal in the sine basis), so T60_n = 3*ln(10)/(s0 + s1*beta_n**2).
Lv = 0.295 m is the vibrating length implied by S-03 (bridge at ~0.295 m from the loop)."""
import json, math
P = {"yehu_s1_F4": {"s0": 0.9754, "s1": 0.0026, "f0": 349.23},
     "yehu_s2_C5": {"s0": 1.5844, "s1": 0.0032, "f0": 523.25}}
Lv = 0.295
out = {}
for k, p in P.items():
    rows = []
    for n in range(1, 11):
        beta = n * math.pi / Lv
        sigma = p["s0"] + p["s1"] * beta * beta
        rows.append({"n": n, "f_ideal_hz": round(n * p["f0"], 2), "sigma_per_s": round(sigma, 4), "t60_s": round(3 * math.log(10) / sigma, 4)})
    out[k] = rows
print(json.dumps(out, indent=1))
