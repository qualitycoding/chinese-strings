# Instrument data rules (D-019) — authoritative inputs for `data/instruments/<key>.json`

Every JSON file is produced by step S-005 exactly from this table. "DD" = design default: an engineering choice
with no measurement behind it, tuned only at the family listening gate (G-003x) and logged in DEVIATIONS.md when changed.
Cited values (C-###) may not be changed except through TEST_CHALLENGE.md.

## Global rules
1. Tension is never chosen directly: `T = rho * pi * r^2 * (2 * Lv * f0)^2` (keeps T-003 exact) — **except erhu and yehu**, which use the cited T, rho, r, E exactly with the anchor Lv (0.387 m / 0.295 m); T-003 confirms they are within 0.5 % of the nominal pitch (S-03: +0.02 %, +0.01 %, +0.17 %, -0.30 %).
2. Material defaults (DD unless cited): Steel rho 7800, E 200e9, sigma0 0.60, sigma1 2.0e-4. Silk (C-034, yehu string 1): rho 1250, E 1.0e10, sigma0 0.9754, sigma1 0.0026. Nylon rho 1140, E 4.0e9, sigma0 1.20, sigma1 4.0e-3. SilkWoundSteel rho 6000, E 1.2e11, sigma0 0.80, sigma1 1.0e-3.
3. Interpolated rows ("a→b"): linear in string index from lowest (a) to highest (b) string.
4. Body modes: from the cited source where named; otherwise DD set "generic-<family>" below. Radiation: yehu sections (S-07) for yehu and banhu; all others an empty list (body output used directly).
5. Radiation sections were designed at 44100 Hz (yehudafx26 examples/exPhrases.m line 18). At any other fs, convert each section by impulse invariance: poles p -> exp(ln(p) * 44100 / fs), residues scaled by 44100 / fs; FIR term unchanged.
6. `midiLow` = lowest open string; `midiHigh` = bowed: highest open + 24; lutes: highest open + number of frets (fretless: + 24); zithers/harp/yangqin: highest open string.
7. `citations` lists every C-### used for the row, plus "D-019" and, for DD rows, "DD".

## Rows
| key | family | open strings (MIDI) | material | Lv (m) | radius (mm) | frets | maxPoly | stopsTogether | body |
|---|---|---|---|---|---|---|---|---|---|
| erhu | Bowed | 62, 69 (C-032) | Steel, rho 6577 / 8363, E 200e9 (C-032) | 0.387 (C-032, S-03) | 0.220 / 0.130 (C-032) | — | 1 | yes | C-033 skin modes 533.7, 848.2, 1091.7, 1165.5, 1428.2, 1476.6 Hz; T60 0.05 s (DD); gain 1 |
| yehu | Bowed | 65, 72 (C-034) | Silk; rho 1250 / 1350; E 1.0e10 / 9.5e9; sigma0 0.9754 / 1.5844; sigma1 0.0026 / 0.0032 (C-034) | 0.295 (S-03) | 0.55 / 0.40 (C-034) | — | 1 | yes | S-07 bridge modes (11) + S-07 radiation |
| gaohu | Bowed | 67, 74 (C-039) | Steel as erhu | 0.360 (DD) | 0.220 / 0.130 | — | 1 | yes | erhu modes x 1.15 freq (DD: "slightly smaller soundbox", C-039) |
| zhonghu | Bowed | 55, 62 (C-039) | Steel | 0.450 (DD) | 0.275 / 0.175 (DD) | — | 1 | yes | erhu modes x 0.80 freq (DD) |
| banhu | Bowed | 74, 81 (DD; fifth apart C-045; "octave above erhu", Tier 3) | Steel | 0.330 (DD) | 0.220 / 0.130 | — | 1 | yes | S-07 bridge modes + radiation (C-045 coconut/wood face) |
| jinghu | Bowed | 74, 81 (DD within C-046 "highest huqin", fifths) | Silk | 0.300 (DD) | 0.45 / 0.35 (DD) | — | 1 | yes | erhu modes x 2.0 freq (DD; resonator ~half erhu diameter, C-046) |
| sihu | Bowed | 50, 50, 57, 57 (DD; pairs C-047) | Steel | 0.450 (DD) | 0.275, 0.275, 0.175, 0.175 (DD) | — | 1 | yes | erhu modes x 0.80 freq (DD) |
| matouqin | Bowed | 55, 60 (C-019 g–c') | Nylon | 0.600 (DD) | 0.90 / 0.75 (DD) | — | 1 | no | generic-bowed-wood (DD) |
| pipa | PluckedLute | 45, 50, 52, 57 (DD; standard A-d-e-a) | SilkWoundSteel | 0.660 (DD) | 0.45, 0.38, 0.33, 0.25 (DD) | 30 chromatic (DD; C-014/C-031 conflict) | 4 | no | generic-lute (DD) |
| liuqin | PluckedLute | 55, 62, 69, 76 (DD; fifths, Tier 3) | SilkWoundSteel | 0.360 (DD) | 0.35→0.20 | 29 chromatic (C-049) | 4 | no | generic-lute x 1.6 freq (DD) |
| yueqin | PluckedLute | 57, 57, 62, 62 (DD) | SilkWoundSteel | 0.400 (DD) | 0.35, 0.35, 0.28, 0.28 (DD) | 24 chromatic (DD) | 4 | no | generic-lute x 1.3 freq (DD) |
| xiaoruan | PluckedLute | 55, 62, 67, 74 (DD; G-D-G-D) | SilkWoundSteel | 0.430 (DD) | 0.38→0.22 | 24 chromatic (C-049) | 4 | no | generic-lute x 1.3 freq (DD) |
| zhongruan | PluckedLute | 43, 50, 55, 62 (DD) | SilkWoundSteel | 0.550 (DD) | 0.50→0.28 | 24 chromatic (C-049) | 4 | no | generic-lute; Helmholtz mode 110 Hz, T60 0.1 s (DD; C-017 qualitative) |
| daruan | PluckedLute | 38, 45, 50, 57 (DD) | SilkWoundSteel | 0.750 (DD) | 0.65→0.35 | 24 chromatic (C-049) | 4 | no | generic-lute x 0.7 freq; Helmholtz 80 Hz (DD) |
| sanxian | PluckedLute | 38, 43, 50 (DD; 4th then 5th, C-018) | Nylon | 0.750 (DD) | 0.60, 0.50, 0.40 (DD) | — (fretless, C-018) | 3 | no | generic-membrane (DD; C-041 analogy) |
| guzheng | Zither | 38,40,42,45,47, 50,52,54,57,59, 62,64,66,69,71, 74,76,78,81,83, 86 (C-012) | SilkWoundSteel | 1.20→0.30 | 0.90→0.25 | — | 21 | no | generic-zither (DD; C-013 qualitative) |
| guqin | Zither | 36, 38, 41, 43, 45, 48, 50 (DD; zhengdiao C D F G A c d) | Silk | 1.10 all (DD) | 0.80→0.35 | — | 7 | no | generic-zither x 0.8 freq (DD; C-011 qualitative) |
| konghou | Zither | 36 diatonic C-major notes C2..C7 (MIDI 36..96), each duplicated (72 strings, pairs equal; C-040) | SilkWoundSteel | 1.30→0.08 | 0.90→0.25 | — | 16 | no | generic-harp (DD; C-065 analogy) |
| yangqin | Struck | chromatic 43..93 (DD), courses 2 (43–59), 3 (60–76), 4 (77–93) (DD; "two strings low, up to five high", Tier 3), courseDetuneCents 1.5 (DD), members spaced symmetrically about the nominal pitch | Steel | 0.90→0.20 | 0.40→0.18 | — | 16 | no | C-016 shaped: modes 100, 180, 260, 340, 420, 500, 580, 660 Hz T60 0.3 s (DD values within C-016's measured 100–700 Hz band) |

## Generic DD body sets (freq Hz : T60 s : gain)
- generic-bowed-wood: 280:0.08:1, 460:0.06:0.8, 700:0.05:0.6, 1050:0.04:0.5, 1600:0.03:0.4, 2400:0.02:0.3
- generic-lute: 100:0.15:1, 200:0.10:0.8, 390:0.08:0.6, 620:0.06:0.5, 900:0.05:0.4, 1400:0.04:0.3, 2200:0.03:0.2
- generic-membrane: 300:0.04:1, 520:0.03:0.8, 830:0.03:0.6, 1200:0.02:0.5, 1700:0.02:0.4
- generic-zither: 90:0.20:1, 150:0.15:0.8, 230:0.12:0.7, 340:0.10:0.6, 480:0.08:0.5, 700:0.06:0.4, 1000:0.05:0.3
- generic-harp: 120:0.20:1, 200:0.15:0.8, 320:0.12:0.6, 500:0.10:0.5, 800:0.08:0.4, 1300:0.06:0.3

## Technique support (D-017) — `supportsTechnique`
| family | Default | Tremolo | Harmonic | Muted | Pizzicato | Glissando | Roll |
|---|---|---|---|---|---|---|---|
| Bowed | yes | yes (bow tremolo) | yes | no | yes | yes (huayin slide) | no |
| PluckedLute | yes | yes (lunzhi / alternate picking) | yes | yes | no | yes (bend/slide) | no |
| Zither | yes | yes (yaozhi) | yes (fanyin) | yes | no | yes (guazou) | no |
| Struck | yes | no | no | yes | no | no | yes (lunyin) |
