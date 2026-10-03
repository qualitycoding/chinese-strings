# Research Question Tree (R1)

Only the `software` profile is active, so there is one root branch (Engineering). It is split into
platform/tooling (E-P), DSP & instrument acoustics (E-D), and quality/security/ops (E-Q).
Each leaf names what it informs. Status: open | answered (C-###) | pruned.

## E-P Platform & tooling
| ID | Question | Informs | Status |
|---|---|---|---|
| Q-P01 | Which JUCE version to pin (8.0.x vs 9.0.x); licence; C++/CMake minimums? | D-001, plan/ENVIRONMENT.md | answered C-001..C-004 (decision pending R3) |
| Q-P02 | Is own-source Apache-2.0 compatible with distributing AGPLv3 JUCE binaries? VST3 SDK licence? | D-002, README | partial C-005; C-007 open |
| Q-P03 | Does JUCE 9 build headless on Linux CI (no X11) for DSP unit tests? Which modules? | D-005, T-unit | open (spike S-01) |
| Q-P04 | Unit test framework (Catch2 v3 vs GoogleTest) and CTest integration | D-005 | answered C-025 (decision pending) |
| Q-P05 | Plugin validation tool, version, headless flags | D-006, T-integration | answered C-006 (spike pending) |
| Q-P06 | GitHub Actions runner images & toolchains for Win x64 / macOS universal / Linux x64 | D-006, CI | open |
| Q-P07 | AU validation on macOS CI (auval) | D-006 | open |
| Q-P08 | Scala .scl/.kbm format specification | D-010, T-security (fuzz) | open |
| Q-P09 | MPE support in JUCE (MPESynthesiser / MPEInstrument) and VST3 note expression | D-012 | open |
| Q-P10 | Plugin state format & versioning (ValueTree XML vs binary) | D-009, T-security | open |

## E-D DSP & instrument acoustics
| ID | Question | Informs | Status |
|---|---|---|---|
| Q-D01 | Which instruments have Tier 1/2 acoustic data (A-002 retention rule)? | D-003 instrument list | partial C-008..C-019 |
| Q-D02 | Huqin family model: bow-friction law, membrane/soundbox model, real-time feasibility | D-004a | partial C-009, C-010, C-028 |
| Q-D03 | Plucked lutes (pipa, liuqin, yueqin, ruan, sanxian, qinqin): string, fret, plectrum/nail, body models | D-004b | partial C-014, C-015, C-017 |
| Q-D04 | Zithers (guzheng, guqin, konghou, se): string, bridge-side bending, cavity/plate models | D-004c | partial C-011..C-013 |
| Q-D05 | Yangqin: hammer–string contact, multi-course strings, bridge impedance | D-004d | partial C-016 |
| Q-D06 | Per-instrument organology: string count, tuning, scale length, string material/tension | D-003, T-pitch, T-layout | partial |
| Q-D07 | Quantitative reference values for tests (inharmonicity B, T60/decay, body modes, vibrato rate/depth) with locators | T-acoustic | open |
| Q-D08 | Licence-clean reference audio for deriving test fixtures | D-007 | partial C-020, C-021, C-026, C-027 |
| Q-D09 | Idiomatic techniques per instrument and their physical mechanism | A-007, D-011 | partial C-020 |
| Q-D10 | Numerical stability of FD/waveguide schemes at 44.1–192 kHz (CFL / passivity) | D-004*, T-stability | open |

## E-Q Quality, security, performance
| ID | Question | Informs | Status |
|---|---|---|---|
| Q-Q01 | Fuzzing harness for parsers on Linux CI (libFuzzer/clang) | T-security | open |
| Q-Q02 | Real-time safety checking (allocation/lock detection on audio thread) | T-perf, A-012 | open |
| Q-Q03 | Benchmark normalisation on shared CI runners vs. A-012 reference laptop | D-008, T-perf | open |
| Q-Q04 | Dependency vulnerability scanning for a C++/CMake (FetchContent) project | T-security | open |
| Q-Q05 | UI illustrations: licence-clean source (programmatic SVG vs. images) | D-013 | open |
