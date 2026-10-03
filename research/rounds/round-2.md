# Research Round 2 — 2026-10-03

Executed by: Opus 5.5, single context (A-016). Claims C-032..C-044 added; C-006, C-007, C-009, C-010, C-027 upgraded; C-008 downgraded.

## R2 Breadth (new leaves)
- Huqin variants' tunings (C-039; Tier 3 + one Tier 1 for erhu).
- Konghou organology (C-040); sanxian analogy via shamisen real-time FDTD model (C-041).
- Scala format (C-042) and an MIT parser library (C-043).
- Tier C instruments: no Tier 1/2 acoustic literature found (C-044). Queries: "Uyghur rawap OR dutar OR satar acoustic analysis physical modeling paper"; earlier round: none for se/zhu/yazheng.
- Licence compatibility (C-007 → corroborated by FSF and ASF).

## R4 Depth (full-text reads)
- Samejima 2023 (Acoust. Sci. Tech. 44(3):281-291, doi:10.1250/ast.44.281) read in full → C-009 verified; parameters → C-032; skin modes → C-033.
- Zheng, Darabundit & Scavone, DAFx26 pp. 347-354 (CC BY 4.0) read in full → C-010 verified; parameters → C-034; real-time factor → C-035; MIT reference code → C-036.

## R3 Synthesis & gaps
1. **Membrane-mode contradiction:** C-008 (Tier 3, "≈2 kHz lowest membrane mode") vs C-033 (FE model, first skin-dominated mode 534 Hz). Neither is a direct Tier 1 measurement of the same quantity. **Resolution:** no frozen test may assert an absolute membrane-mode frequency. Tests check that the implementation reproduces the *configured, cited* modal set (implementation verification), not a contested physical value.
2. **Performance vs. method (load-bearing):** the published real-time FD yehu costs ~15 % of an M1 core per voice (C-035). A-012 requires 16 voices of the heaviest instrument < 25 % of one core. An FD scheme cannot meet that with free polyphony. **Proposed D-004:** digital-waveguide strings with a passive elasto-plastic bow junction for all bowed instruments; the published FD model is a reference for parameters and qualitative behaviour only.
3. **Citation correctness:** the round-1 inferred citation for Woodhouse's thermal friction model was wrong (C-027 corrected from the DAFx26 reference list). Inferred bibliographic claims are not trusted; every citation in the final plan must come from a read source.
4. **Tuning sources are Tier 3** for gaohu/zhonghu/jinghu (C-039). **Proposed D-011:** tunings and fret maps are data tables with a cited default and user override, so a wrong default is Medium severity (documented workaround), not High.
5. **Remaining Tier B gaps:** sihu, gehu/diyingehu, zhuihu, qinqin, xiaoruan, banhu, jinghu, leiqin, liuqin need organology (string count, tuning, body type) before D-003 can retain them. → round 3.
6. **CI runner images** (Windows/macOS) and AU validation unverified (Q-P06, Q-P07). → round 3.
7. **Dispersion and pitch:** silk-string stiffness shifts pitch by up to 1.4 cents (C-037), half the ±3 cent budget. Pitch tuning must include dispersion compensation; the pitch test tolerance stays ±3 cents with the justification recorded.

## R5 Adversarial pass
- C-001/D-001: JUCE 9.0.3 is 5 days old. Mitigation: exact-tag pin + fallback rule to 8.0.15 (D-001).
- C-006: pluginval v1.0.4 is built against JUCE 8.0.3 (host side). S-02 shows it validates a JUCE 9.0.3 VST3 at strictness 5 and 10 → no incompatibility observed on Linux. Windows/macOS untested in sandbox → risk carried to Phase 4.
- C-014/C-031 pipa frets: still contradictory; handled by D-011 data tables.
- C-035 is single-source and on different hardware (M1) from A-012's x64 reference; only used to reject FD, which is the conservative direction.
- C-036 MIT code: compatible with Apache-2.0 source and AGPLv3 binary; attribution must be kept in NOTICE if any code is ported (D-002).
- C-043 tuning-library has release tags but the plan pins a commit SHA; a parser bug becomes our attack surface → fuzz target required (T-security).

## R6 Empirical verification
- **S-02**: JUCE 9.0.3 VST3 synth on Linux; first build failed (missing X11 extension headers), passed after JUCE-documented deps; pluginval v1.0.4 passes strictness 5 (`--skip-gui-tests`) and 10 under `xvfb-run`. → C-006, C-038 verified.
- **S-03**: published erhu and yehu string parameters are self-consistent (implied vibrating lengths 387 mm and 295 mm for both strings of each instrument). → C-032, C-034, C-037 verified.

## Saturation status
Not saturated (13 new load-bearing claims, one downgrade, one contradiction). Round 3 required.
