# Assumptions (A-###)

All A-001..A-017 were presented to the human in the Phase 0 intake batch on 2026-10-03.
The human replied "Defaults"; every proposed default is therefore adopted as stated.

## Intake restatement (0.3.1)
- **Goal:** Produce a plugin (`chinese-strings`) that physically models every Chinese stringed instrument for which adequate acoustic reference material exists.
- **In scope:** Instruments per A-002 (subject to evidence), physical-model DSP engine, JUCE plugin (VST3/AU/Standalone), instrument-illustration UI, articulations, Scala tuning, CI builds, automated test suite, human listening gates.
- **Out of scope:** Sample libraries; CLAP; public release/publishing (gated, G-002); mobile/iOS AUv3; AAX; network features; telemetry; committing copyrighted recordings.
- **Success criteria (measurable):** see SC-### in plan/TRACEABILITY.md, derived from A-011, A-012, A-013.
- **Constraints:** C++20; JUCE 8.0.x pinned; CMake ≥ 3.25; Windows x64, macOS universal (arm64+x86_64), Linux x64; Apache-2.0 own source, AGPLv3 binary distribution terms.

## Software intake (0.3.2)
- **Target users:** composers/producers using a DAW; the project owner (Windows 11 + Reaper for listening gates, A-017).
- **Platforms:** Windows x64 (MSVC), macOS universal (Xcode/AppleClang), Linux x64 (GCC ≥ 12).
- **Deployment target:** none (`software.deploys = false`); CI artifacts only.
- **Performance targets:** A-012.
- **Threat model / data sensitivity:** A-013. No personal data handled. Data sensitivity: public.
- **Data handling & retention:** plugin state stored only in host project files; user preset files in the OS user data directory; nothing transmitted.
- **Release channel & versioning:** SemVer `0.y.z` during development; tags/releases only after G-002.
- **Maintenance expectations:** single maintainer; CI must stay green on all three OSes; dependencies pinned and updated deliberately.

## Assumptions

| ID | Topic | Decision (adopted default) | Consequence |
|---|---|---|---|
| A-001 | Profiles | `software` only, `software.deploys=false`. | No math/figure/manuscript artifacts; DSP accuracy tested as software. |
| A-002 | Instrument scope | Tier A: erhu, gaohu, zhonghu, banhu, jinghu, pipa, liuqin, yueqin, zhongruan, daruan, sanxian, guzheng, guqin, yangqin, konghou. Tier B: matouqin, leiqin, sihu, gehu/diyingehu, zhuihu, qinqin, xiaoruan. Tier C: rawap, dutar, satar, dombra, se, zhu, yazheng. An instrument is retained only if research finds Tier 1/2 acoustic data (or Tier 1/2 organological data sufficient to parameterise a model by analogy, recorded as a decision); drops are logged in DECISIONS.md. | Final instrument list is a research output, fixed before freeze. |
| A-003 | Synthesis | Physical modelling, no samples: digital waveguide strings + modal body/resonator; friction-curve bowed-string model for huqin; plectrum/nail contact for plucked; nonlinear hammer for yangqin; membrane soundboard for skin-faced instruments. | Realism depends on model parameters from literature. |
| A-004 | Plugin shape | One plugin, instrument selector, one instrument per instance. | Single binary; shared engine. |
| A-005 | Framework/formats | C++20, JUCE 8.0.x (exact tag pinned in ENVIRONMENT.md), CMake; VST3 + Standalone on Win/macOS/Linux, AU on macOS; no CLAP. | |
| A-006 | Licensing | Own source Apache-2.0; distributed binaries under AGPLv3 (JUCE 8 open-source licence); stated in README. Research verifies compatibility and alternatives. | If research shows incompatibility, recorded as D-### and the profile of the decision re-run. |
| A-007 | Techniques | Per-instrument idiomatic core techniques via keyswitches + MIDI CC; MPE per-note pitch & pressure supported. | |
| A-008 | UI | Instrument illustration highlighting string and stopping position/bridge-side position per played note, with parameter panels. | Mirrors saxophone-vst / clarinet-vst. |
| A-009 | Tuning | Standard per-instrument tunings and string layouts; Scala .scl/.kbm import in scope. | Scala parser is attack surface (A-013). |
| A-010 | Polyphony | Realistic per instrument (bowed mono+legato; plucked ≤ string count; zithers/yangqin polyphonic) with a "free polyphony" override. | |
| A-011 | Realism criteria | Objective tests vs. published measurements (pitch ±3 cents; inharmonicity, body resonances, decay times within cited tolerances) plus human listening gates G-003a..d (one per family) before UI polish. No copyrighted recordings committed. | |
| A-012 | Performance | 16 voices of heaviest instrument @48 kHz/128-sample buffer < 25 % of one core on 2020-era x64 laptop reference; zero added latency; no allocation/locks on audio thread. | CI enforces a normalised benchmark (see D-### later). |
| A-013 | Threat model | Offline; no network, no telemetry. Attack surface = preset/state/Scala parsing → validation + fuzz tests. | |
| A-014 | Release | Plan ends with CI artifacts. Any GitHub Release/tag gated by G-002. | |
| A-015 | Repo bootstrap | Minimal `main` (LICENSE, README stub) pushed; all plan artifacts on the generation branch. | Done: main @ initial commit. |
| A-016 | Execution environment | Planning runs in claude.ai chat with a single model (Opus 5.5); no subagents. All tiered and fresh-context roles are executed sequentially by the same model and logged as tier substitutions. | Reduced independence of adversarial review, cold-read and pre-mortem; mitigated by explicit role separation and written checklists. |
| A-017 | Human test setup | Windows 11 + Reaper for human listening gates. | G-003a..d evidence bundles are rendered WAVs; the Reaper check happens at G-004 (plan clarification, Phase 3). |
| A-018 | AI disclosure | Not applicable (no publication); README states the plan was AI-generated. | |
