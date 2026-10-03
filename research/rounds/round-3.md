# Research Round 3 — 2026-10-03

Executed by: Opus 5.5, single context (A-016). Claims C-045..C-055 added; C-039 upgraded to corroborated with a Tier 1 source (JAMIS 2002).

## R2/R4 Breadth and depth
- Huqin organology from a Tier 1 organology paper (JAMIS 2002) plus museum catalogues (MIMEd, Horniman, NMM, Penn): banhu (C-045), jinghu (C-046), sihu (C-047), construction details shared by the family (C-048).
- Plucked-lute organology (C-049).
- JUCE 9 breaking changes read from the pinned tag (C-050).
- CI runner labels (C-051).

## R3 Synthesis
1. **Gaohu tuning contradiction resolved:** JAMIS states "a fourth or fifth higher" — both observed tunings are in practice. D-011 ships G4-D5 default plus A4-E5 alternative.
2. **Banhu ↔ yehu:** both are coconut-body, wooden-face fiddles (C-045). Banhu is parameterised from the measured yehu body (C-034) with its own tuning — an analogy with a strong structural basis.
3. **Final instrument list (D-003):** see DECISIONS.md. 19 instruments retained (corrected from an arithmetic error "22" in the first draft of this log); dropped: gehu/diyingehu (modern cello-like hybrids, no data), zhuihu, qinqin, leiqin (insufficient organology/acoustics), Tier C (C-044).
4. **Scope is large (22 instruments).** Plan must stage delivery by family with human listening gates per family (G-003a..d), so value is delivered early and the plan can stop cleanly.
5. **A-012 feasibility confirmed** for DWG (C-054: 16 voices ≈ 8 % of a 1-vCPU sandbox core).
6. **RT-safety test technique** verified (C-055).

## R5 Adversarial
- C-054 measures a stand-in voice, not the final model; the frozen performance test must measure the real heaviest instrument. The stand-in proves only order-of-magnitude headroom (≈3×).
- C-052 is single-source (one test label in a log). AU validation must not rely on it: CI also runs `auval -v aumu Cs01 Qcod` explicitly (decision rule if unavailable: halt macOS AU job, log).
- C-051: `macos-26` runners are arm64 only; universal binaries are produced by `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` and the x86_64 slice cannot be executed in CI on arm64 runners without Rosetta. Mitigation: a second macOS job on `macos-26-intel` runs pluginval on the x86_64 slice.
- Windows toolchain: windows-2025 now ships VS 2026; the CMake version needed to know the VS 2026 generator is unverified. Mitigation: CI installs CMake 4.4.3 from PyPI (as in the sandbox) and uses the Ninja generator inside a VS developer environment; decision rule in DECISIONS.md.
- OSV vulnerability scanning by git commit (planned for pinned FetchContent deps) is unverified in the sandbox (api.osv.dev not reachable from the sandbox allowlist) → carried as a risk to Phase 4.

## R6 Spikes
- **S-04** libFuzzer/ASan/UBSan over the Scala loader: 1,757,001 runs, clean (C-053).
- **S-05** DWG voice cost + allocation detector with negative control (C-054, C-055).

## Saturation status
Round 3 still added load-bearing claims (organology, CI, RT-safety). A 4th round is required, scoped to: adversarial re-check of every load-bearing claim below `corroborated`, VS 2026 + CMake generator, OSV commit query, and any remaining single-source claims that drive frozen tests.
