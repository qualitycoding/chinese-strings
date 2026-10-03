# Research Round 4 — 2026-10-03

Executed by: Opus 5.5, single context (A-016). Claims C-056..C-063 added.

## Leaves closed
- Q-P09 MPE → C-056 (JUCE source: MPE classes present; VST3 wrapper has no note-expression interface) → D-015.
- Q-P10 state format → C-057 (bounded XML read) → D-016.
- Q-P07 auval → C-061 → D-012b.
- Q-Q04 dependency scan → C-059, C-060 → D-014.
- Q-D10/Q-D15 DWG stability and bow passivity → C-062 (Smith PASP, Tier 1 textbook) → D-004a passivity conditions.
- Q-D07 reference values → policy: frozen acoustic tests verify the implementation against cited parameters and analytic consequences of them (pitch from tension/density/length, configured modal sets, decay oracle C-063), never against single contested physical values (see C-008/C-033).
- Windows toolchain → C-058 → D-012a.

## R3 Synthesis
- Every open leaf in QUESTIONS.md is now answered or decided.
- New load-bearing claims this round were all answers to previously open leaves; no new questions were raised.

## R5 Adversarial
- C-056: MPE via VST3 is host-dependent — the plan treats VST3 MPE as best effort with a human check (G-003a), automated MPE tests run in-process.
- C-063: the decay oracle assumes simply-supported modes over the vibrating length; the DAFx26 model couples strings to a bridge, which adds damping near bridge modes. Tolerance for decay tests is therefore asymmetric: measured T60 ≤ oracle T60 × 1.10 and ≥ oracle × 0.70 (extra bridge damping allowed, extra sustain not). Justification recorded in the test docstring.
- C-061: auval can fail on unsigned components on some macOS versions; CI builds are ad-hoc signed by CMake/Xcode by default. Decision rule: if auval reports "cannot open component", run `codesign --force -s - <component>` and retry once.
- C-058: VS 18 generator promoted from experimental (4.2-rc notes) to released; pinned 4.4.3 is well after.

## R6 Spikes
- **S-06** decay oracle (C-063).

## Saturation status
No new questions; all new claims closed existing leaves. One more adversarial-only round (round 5) is run to re-check single-source claims that would drive frozen tests.
