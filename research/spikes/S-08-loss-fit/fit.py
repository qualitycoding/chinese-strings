"""Spike S-08: can (gain x one-pole low-pass) meet T-010's 5 % ln|H| tolerance for the yehu loss law,
or is a biquad needed? Fit minimises max relative error of ln|H| over harmonics 1..min(10, 0.4 fs/f0)."""
import numpy as np
from scipy.optimize import minimize
s0, s1, L = 0.9754, 0.0026, 0.295
def target(f, f0): c = 2 * L * f0; beta = 2 * np.pi * f / c; return np.exp(-(s0 + s1 * beta ** 2) / f0)
def onepole(p, w): g, a = p; return g * (1 - a) / np.abs(1 - a * np.exp(-1j * w))
worst = {}
for fs in (44100, 48000, 96000):
    for f0 in (110, 349.23, 880):
        n = np.arange(1, 11); n = n[n * f0 < 0.4 * fs]; f = n * f0; w = 2 * np.pi * f / fs
        lt = np.log(target(f, f0))
        def err(p):
            g, a = p
            if not (0 < g <= 1 and 0 <= a < 1): return 1e9
            return np.max(np.abs(np.log(onepole(p, w)) - lt) / np.abs(lt))
        best = min((minimize(err, [target(f0, f0), a0], method="Nelder-Mead", options={"xatol": 1e-12, "fatol": 1e-12, "maxiter": 20000}) for a0 in (1e-4, 1e-3, 1e-2, 0.1)), key=lambda r: r.fun)
        # passivity: max |H| on grid
        grid = np.linspace(0, np.pi, 4097); mx = onepole(best.x, grid).max()
        worst[(fs, f0)] = (best.fun, mx)
        print(f"fs={fs} f0={f0}: max rel ln-error={best.fun*100:.2f}%  g={best.x[0]:.6f} a={best.x[1]:.6f} max|H|={mx:.6f}")

print("--- pole-zero first-order shelf x gain: H = g*(1-a)/(1-b) * (1 - b z^-1)/(1 - a z^-1), 0<=b<a<1 ---")
def pz(p, w): g, a, b = p; z = np.exp(-1j * w); return g * (1 - a) / (1 - b) * np.abs(1 - b * z) / np.abs(1 - a * z)
for fs in (44100, 48000, 96000):
    for f0 in (110, 349.23, 880):
        n = np.arange(1, 11); n = n[n * f0 < 0.4 * fs]; f = n * f0; w = 2 * np.pi * f / fs; lt = np.log(target(f, f0))
        def err(p):
            g, a, b = p
            if not (0 < g <= 1 and 0 <= b < a < 1): return 1e9
            return np.max(np.abs(np.log(pz(p, w)) - lt) / np.abs(lt))
        best = min((minimize(err, [target(f0, f0), a0, a0 * 0.5], method="Nelder-Mead", options={"xatol": 1e-13, "fatol": 1e-13, "maxiter": 40000}) for a0 in (1e-3, 1e-2, 0.1, 0.5, 0.9)), key=lambda r: r.fun)
        grid = np.linspace(0, np.pi, 4097); mx = pz(best.x, grid).max()
        print(f"fs={fs} f0={f0}: max rel ln-error={best.fun*100:.2f}%  g={best.x[0]:.6f} a={best.x[1]:.6f} b={best.x[2]:.6f} max|H|={mx:.6f}")

print("--- cascade of two one-pole low-passes x gain (3 parameters) ---")
def two(p, w): g, a1, a2 = p; z = np.exp(-1j * w); return g * (1 - a1) * (1 - a2) / np.abs(1 - a1 * z) / np.abs(1 - a2 * z)
print("--- 5-tap symmetric linear-phase FIR (3 free coefficients), |H| = |c0 + 2 c1 cos w + 2 c2 cos 2w| ---")
def fir(p, w): c0, c1, c2 = p; return np.abs(c0 + 2 * c1 * np.cos(w) + 2 * c2 * np.cos(2 * w))
for name, fn, x0s, ok in (("two-pole", two, [[.99, .5, .1], [.99, .8, .3], [.99, .9, .01]], lambda p: 0 < p[0] <= 1 and 0 <= p[1] < 1 and 0 <= p[2] < 1),
                          ("fir5", fir, [[.6, .2, 0], [.5, .25, 0.0], [.8, .1, 0]], lambda p: True)):
    for fs in (44100, 48000, 96000):
        for f0 in (110, 349.23, 880):
            n = np.arange(1, 11); n = n[n * f0 < 0.4 * fs]; f = n * f0; w = 2 * np.pi * f / fs; lt = np.log(target(f, f0))
            grid = np.linspace(0, np.pi, 4097)
            def err(p):
                if not ok(p): return 1e9
                if fn(p, grid).max() > 1.0: return 1e9
                return np.max(np.abs(np.log(fn(p, w)) - lt) / np.abs(lt))
            best = min((minimize(err, x0, method="Nelder-Mead", options={"xatol": 1e-14, "fatol": 1e-14, "maxiter": 60000}) for x0 in x0s), key=lambda r: r.fun)
            print(f"{name} fs={fs} f0={f0}: max rel ln-error={best.fun*100:.2f}%  max|H|={fn(best.x, grid).max():.6f}")
