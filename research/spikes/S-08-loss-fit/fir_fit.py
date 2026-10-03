"""Spike S-08b: zero-phase symmetric FIR loss filter (2M+1 taps) fitted by weighted linear least squares on |H| at
harmonics 1..min(20, 0.45 fs/f0) and 64 extra points up to fs/2 (passivity margin), weights ~ 1/(|Ht| |ln Ht|) so the
fit approximates relative ln|H| error; then scaled so max|H| <= 1 on a 4097-point grid. Reports the smallest M meeting
3 % (T-010 requires 5 %) for every fs/f0 case of T-010 plus extreme instrument cases (f0 30..4200 Hz)."""
import numpy as np
s0, s1, L = 0.9754, 0.0026, 0.295
def target(f, f0): c = 2 * L * f0; b = 2 * np.pi * f / c; return np.exp(-(s0 + s1 * b * b) / f0)
def design(fs, f0, M):
    n = np.arange(1, 21); n = n[n * f0 < 0.45 * fs]; f = n * f0
    fx = np.linspace(f[-1], fs / 2, 64)
    F = np.concatenate([f, fx]); W = np.concatenate([1 / (target(f, f0) * np.abs(np.log(target(f, f0)))), np.full(64, 1.0)])
    Ht = np.concatenate([target(f, f0), np.minimum(target(fx, f0), target(f[-1], f0))])
    w = 2 * np.pi * F / fs; A = np.stack([np.ones_like(w)] + [2 * np.cos(k * w) for k in range(1, M + 1)], 1)
    c, *_ = np.linalg.lstsq(A * W[:, None], Ht * W, rcond=None)
    grid = np.linspace(0, np.pi, 4097); Hg = np.abs(np.stack([np.ones_like(grid)] + [2 * np.cos(k * grid) for k in range(1, M + 1)], 1) @ c)
    c = c / max(1.0, Hg.max())
    nn = np.arange(1, 11); nn = nn[nn * f0 < 0.4 * fs]; ww = 2 * np.pi * nn * f0 / fs
    H = np.abs(np.stack([np.ones_like(ww)] + [2 * np.cos(k * ww) for k in range(1, M + 1)], 1) @ c)
    lt = np.log(target(nn * f0, f0)); return np.max(np.abs(np.log(H) - lt) / np.abs(lt)), Hg.max() / max(1.0, Hg.max())
for fs in (22050, 44100, 48000, 96000, 192000):
    for f0 in (30, 110, 349.23, 880, 2093, 4186):
        for M in range(1, 41):
            e, mx = design(fs, f0, M)
            if e < 0.03: break
        print(f"fs={fs:6d} f0={f0:7.2f}: M={M:2d} taps={2*M+1:2d} max rel ln-error={e*100:.2f}% max|H|<=1:{mx<=1+1e-12}")
