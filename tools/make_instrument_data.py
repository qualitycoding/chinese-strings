#!/usr/bin/env python3
"""Writes data/instruments/<key>.json exactly per plan/INSTRUMENT_DATA.md (S-003). Re-running overwrites the files.
Cited values (erhu, yehu strings; C-032, C-034) are used verbatim; everything else is a design default (DD)."""
import json, math, os
OUT = os.path.join(os.path.dirname(__file__), "..", "data", "instruments")
BODY = json.load(open(os.path.join(os.path.dirname(__file__), "..", "data", "bodies", "yehu_body.json")))
hz = lambda m: 440.0 * 2 ** ((m - 69) / 12)
MAT = {"Steel": (7800.0, 200e9, 0.60, 2.0e-4), "Silk": (1250.0, 1.0e10, 0.9754, 0.0026),
       "Nylon": (1140.0, 4.0e9, 1.20, 4.0e-3), "SilkWoundSteel": (6000.0, 1.2e11, 0.80, 1.0e-3)}
ERHU_RHO = (6577.0, 8363.0)
def lerp(a, b, i, n): return a if n == 1 else a + (b - a) * i / (n - 1)
def tension(rho, r, L, f): return rho * math.pi * r * r * (2 * L * f) ** 2
def S(midi, r, L, mat, rho=None, E=None, s0=None, s1=None, T=None, courses=1, detune=0.0):
    d = MAT[mat]; rho = d[0] if rho is None else rho; E = d[1] if E is None else E
    s0 = d[2] if s0 is None else s0; s1 = d[3] if s1 is None else s1; f = hz(midi)
    return {"openHz": f, "vibratingLengthM": L, "tensionN": tension(rho, r, L, f) if T is None else T, "densityKgM3": rho,
            "radiusM": r, "youngsModulusPa": E, "sigma0": s0, "sigma1": s1, "material": mat, "courses": courses, "courseDetuneCents": detune}
ERHU_MODES = [(533.7, 0.05, 1.0), (848.2, 0.05, 1.0), (1091.7, 0.05, 1.0), (1165.5, 0.05, 1.0), (1428.2, 0.05, 1.0), (1476.6, 0.05, 1.0)]
GEN = {"bowed-wood": [(280, .08, 1), (460, .06, .8), (700, .05, .6), (1050, .04, .5), (1600, .03, .4), (2400, .02, .3)],
       "lute": [(100, .15, 1), (200, .10, .8), (390, .08, .6), (620, .06, .5), (900, .05, .4), (1400, .04, .3), (2200, .03, .2)],
       "membrane": [(300, .04, 1), (520, .03, .8), (830, .03, .6), (1200, .02, .5), (1700, .02, .4)],
       "zither": [(90, .20, 1), (150, .15, .8), (230, .12, .7), (340, .10, .6), (480, .08, .5), (700, .06, .4), (1000, .05, .3)],
       "harp": [(120, .20, 1), (200, .15, .8), (320, .12, .6), (500, .10, .5), (800, .08, .4), (1300, .06, .3)]}
def scale(modes, k): return [(f * k, t, g) for f, t, g in modes]
inv = [1.0 / m for m in (b["massKg"] for b in BODY["bridgeModes"])]
YEHU_MODES = [(b["freqHz"], b["t60s"], (1.0 / b["massKg"]) / max(inv)) for b in BODY["bridgeModes"]]
YANGQIN_MODES = [(100 + 80 * k, 0.3, 1.0 / (1 + 0.15 * k)) for k in range(8)]
def inst(key, name, fam, maxpoly, stops, lo, hi, cites, strings, frets, modes, radiation="none"):
    return {"key": key, "displayName": name, "family": fam, "maxPolyphony": maxpoly, "stopsAllStringsTogether": stops, "midiLow": lo, "midiHigh": hi,
            "citations": cites, "strings": strings, "fretSemitones": frets,
            "bodyModes": [{"freqHz": round(f, 6), "t60s": t, "gain": round(g, 9)} for f, t, g in modes], "radiation": radiation}
D = []
# ---- Bowed
er = [S(62, 0.220e-3, 0.387, "Steel", rho=6577.0, T=51.68), S(69, 0.130e-3, 0.387, "Steel", rho=8363.0, T=51.50)]
D.append(inst("erhu", "Erhu", "Bowed", 1, True, 62, 93, ["C-032", "C-033", "C-048", "D-019"], er, [], ERHU_MODES))
ye = [{**S(65, 0.55e-3, 0.295, "Silk", rho=1250.0, E=1.0e10, s0=0.9754, s1=0.0026, T=50.61)},
      {**S(72, 0.40e-3, 0.295, "Silk", rho=1350.0, E=9.5e9, s0=1.5844, s1=0.0032, T=64.29)}]
D.append(inst("yehu", "Yehu", "Bowed", 1, True, 65, 96, ["C-034", "C-010", "C-067", "S-07", "D-019"], ye, [], YEHU_MODES, "yehu"))
D.append(inst("gaohu", "Gaohu", "Bowed", 1, True, 67, 98, ["C-039", "D-019", "DD"], [S(67, 0.220e-3, 0.360, "Steel", rho=6577.0), S(74, 0.130e-3, 0.360, "Steel", rho=8363.0)], [], scale(ERHU_MODES, 1.15)))
D.append(inst("zhonghu", "Zhonghu", "Bowed", 1, True, 55, 86, ["C-039", "D-019", "DD"], [S(55, 0.275e-3, 0.450, "Steel", rho=6577.0), S(62, 0.175e-3, 0.450, "Steel", rho=8363.0)], [], scale(ERHU_MODES, 0.80)))
D.append(inst("banhu", "Banhu", "Bowed", 1, True, 74, 105, ["C-045", "C-034", "C-067", "D-019", "DD"], [S(74, 0.220e-3, 0.330, "Steel", rho=6577.0), S(81, 0.130e-3, 0.330, "Steel", rho=8363.0)], [], YEHU_MODES, "yehu"))
D.append(inst("jinghu", "Jinghu", "Bowed", 1, True, 74, 105, ["C-046", "D-019", "DD"], [S(74, 0.45e-3, 0.300, "Silk"), S(81, 0.35e-3, 0.300, "Silk")], [], scale(ERHU_MODES, 2.0)))
D.append(inst("sihu", "Sihu", "Bowed", 1, True, 50, 81, ["C-047", "D-019", "DD"],
              [S(50, 0.275e-3, 0.450, "Steel", rho=6577.0), S(50, 0.275e-3, 0.450, "Steel", rho=6577.0), S(57, 0.175e-3, 0.450, "Steel", rho=8363.0), S(57, 0.175e-3, 0.450, "Steel", rho=8363.0)], [], scale(ERHU_MODES, 0.80)))
D.append(inst("matouqin", "Matouqin", "Bowed", 1, False, 55, 84, ["C-019", "D-019", "DD"], [S(55, 0.90e-3, 0.600, "Nylon"), S(60, 0.75e-3, 0.600, "Nylon")], [], GEN["bowed-wood"]))
# ---- Plucked lutes
def lute(key, name, midis, r0, r1, L, mat, nfr, modes, cites, radii=None):
    n = len(midis); rr = radii or [lerp(r0, r1, i, n) * 1e-3 for i in range(n)]
    st = [S(m, r, L, mat) for m, r in zip(midis, rr)]
    return inst(key, name, "PluckedLute", 4 if n == 4 else n, False, min(midis), max(midis) + nfr, cites, st, list(range(1, nfr + 1)), modes)
D.append(lute("pipa", "Pipa", [45, 50, 52, 57], 0, 0, 0.660, "SilkWoundSteel", 30, GEN["lute"], ["C-014", "C-031", "D-019", "DD"], [0.45e-3, 0.38e-3, 0.33e-3, 0.25e-3]))
D.append(lute("liuqin", "Liuqin", [55, 62, 69, 76], 0.35, 0.20, 0.360, "SilkWoundSteel", 29, scale(GEN["lute"], 1.6), ["C-049", "D-019", "DD"]))
D.append(lute("yueqin", "Yueqin", [57, 57, 62, 62], 0, 0, 0.400, "SilkWoundSteel", 24, scale(GEN["lute"], 1.3), ["C-015", "D-019", "DD"], [0.35e-3, 0.35e-3, 0.28e-3, 0.28e-3]))
D.append(lute("xiaoruan", "Xiaoruan", [55, 62, 67, 74], 0.38, 0.22, 0.430, "SilkWoundSteel", 24, scale(GEN["lute"], 1.3), ["C-049", "D-019", "DD"]))
D.append(lute("zhongruan", "Zhongruan", [43, 50, 55, 62], 0.50, 0.28, 0.550, "SilkWoundSteel", 24, GEN["lute"] + [(110, 0.1, 1.0)], ["C-017", "C-049", "D-019", "DD"]))
D.append(lute("daruan", "Daruan", [38, 45, 50, 57], 0.65, 0.35, 0.750, "SilkWoundSteel", 24, scale(GEN["lute"], 0.7) + [(80, 0.1, 1.0)], ["C-049", "D-019", "DD"]))
sx = [S(38, 0.60e-3, 0.750, "Nylon"), S(43, 0.50e-3, 0.750, "Nylon"), S(50, 0.40e-3, 0.750, "Nylon")]
D.append(inst("sanxian", "Sanxian", "PluckedLute", 3, False, 38, 74, ["C-018", "C-041", "D-019", "DD"], sx, [], GEN["membrane"]))
# ---- Zithers / harp
gz = [38, 40, 42, 45, 47, 50, 52, 54, 57, 59, 62, 64, 66, 69, 71, 74, 76, 78, 81, 83, 86]
D.append(inst("guzheng", "Guzheng", "Zither", 21, False, 38, 86, ["C-012", "C-013", "D-019", "DD"], [S(m, lerp(0.90, 0.25, i, 21) * 1e-3, lerp(1.20, 0.30, i, 21), "SilkWoundSteel") for i, m in enumerate(gz)], [], GEN["zither"]))
gq = [36, 38, 41, 43, 45, 48, 50]
D.append(inst("guqin", "Guqin", "Zither", 7, False, 36, 50, ["C-011", "D-019", "DD"], [S(m, lerp(0.80, 0.35, i, 7) * 1e-3, 1.10, "Silk") for i, m in enumerate(gq)], [], scale(GEN["zither"], 0.8)))
kn = [36 + 12 * o + d for o in range(5) for d in (0, 2, 4, 5, 7, 9, 11)] + [96]
kh = []
for i, m in enumerate(kn):
    s = S(m, lerp(0.90, 0.25, i, 36) * 1e-3, lerp(1.30, 0.08, i, 36), "SilkWoundSteel"); kh += [dict(s), dict(s)]
D.append(inst("konghou", "Konghou", "Zither", 16, False, 36, 96, ["C-040", "C-065", "D-019", "DD"], kh, [], GEN["harp"]))
# ---- Struck
yq = []
for i, m in enumerate(range(43, 94)):
    c = 2 if m <= 59 else 3 if m <= 76 else 4
    yq.append(S(m, lerp(0.40, 0.18, i, 51) * 1e-3, lerp(0.90, 0.20, i, 51), "Steel", courses=c, detune=1.5))
D.append(inst("yangqin", "Yangqin", "Struck", 16, False, 43, 93, ["C-016", "C-028", "D-019", "DD"], yq, [], YANGQIN_MODES))
os.makedirs(OUT, exist_ok=True)
for d in D:
    json.dump(d, open(os.path.join(OUT, d["key"] + ".json"), "w"), indent=1)
print(len(D), "files written")
