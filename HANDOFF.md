# HANDOFF — chinese-strings implementation

## Purpose
Implement `chinese-strings`: a JUCE 9 plugin (VST3, AU on macOS, Standalone) that physically models 19 Chinese stringed instruments with digital waveguides, passing a frozen test suite, then stop at human gates. You (the implementer) need no access to the planner or the conversation that produced this plan.

## Active profiles
`software` only (`software.deploys = false`). Rules 7/8 and G-001 do not apply. See plan/PROFILE.md.

## Generation branch
`gen-20261003T134254Z-chinese-strings-vst` (this branch). S-001 creates `impl/v0` from its head.

## Reading order
1. HANDOFF.md (this file) 2. plan/PROFILE.md 3. plan/ASSUMPTIONS.md 4. plan/DECISIONS.md (decisions + decision rules DR-01..DR-13 + default rule) 5. plan/INSTRUMENT_DATA.md 6. plan/GATES.md 7. plan/PLAN.md 8. plan/ENVIRONMENT.md and plan/deps.lock.json 9. tests/TESTS.md, tests/RED_REPORT.md 10. plan/TRACEABILITY.md 11. research/ (background only; claims cited by ID in decisions).

## Environment setup
Literal commands: plan/ENVIRONMENT.md ("Setup commands"). Linux was verified in the planning sandbox (Ubuntu 24.04.4, GCC 13.3, CMake 4.4.3, Ninja 1.13.2). Windows/macOS are CI-only.

## Run the frozen suite
```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
ctest --test-dir build --output-on-failure            # all labels
ctest --test-dir build -L perf --output-on-failure     # T-080 (Release only)
```
At hand-off the expected result is the red state in tests/RED_REPORT.md (38 of 42 fail with cs::NotImplemented / missing NOTICE).

## Verify the freeze
`sha256sum -c tests/FROZEN_MANIFEST.sha256` (macOS: `shasum -a 256 -c tests/FROZEN_MANIFEST.sha256`). Every line must be `OK`. Run before every commit; CI also runs it.

## Steps at a glance
| Step | Title | Gate |
|---|---|---|
| S-001 | Bootstrap, environment and freeze verification | — |
| S-002 | Licence, NOTICE, repository hygiene | — |
| S-003 | Instrument data generator and 19 data files | — |
| S-004 | DSP blocks I | — |
| S-005 | DSP blocks II (loss, dispersion, waveguide string) | — |
| S-006 | Tuning and Scala import (+fuzz) | — |
| S-007 | Engine core | — |
| S-008 | Bowed family + render tool | G-003a |
| S-009 | Techniques, vibrato, bend, MPE, tuning | — |
| S-010 | Plucked lutes | G-003b |
| S-011 | Zithers and harp | G-003c |
| S-012 | Yangqin | G-003d |
| S-013 | Plugin state codec (+fuzz) | — |
| S-014 | JUCE plugin shell and UI | — |
| S-015 | Performance | — |
| S-016 | Continuous integration | G-004 |
| S-017 | Release candidate (PR only) | G-002 |

## Human gates
plan/GATES.md: G-003a..d (listening, after each family), G-004 (plugin acceptance in Reaper/Windows 11), G-002 (merge/tag/release — **you never merge, tag, release, sign or publish**).

## Halt / deviation protocol
- Unanticipated situation → plan/DECISIONS.md default rule.
- Reversible, in-scope deviations → append to `DEVIATIONS.md` (date, step, what, why) and continue.
- Anything touching frozen tests, security, data integrity, a public interface (D-020 signatures), or research integrity → write `BLOCKED.md` and halt.
- A frozen test you believe is wrong → `TEST_CHALLENGE.md` (DR-02) and halt. You may never modify, skip, mark expected-failure, or weaken a frozen test, fixture, tolerance or the manifest.
- Cited data values (non-DD rows of plan/INSTRUMENT_DATA.md) are never changed.

## Integrity (Rule 9, verbatim)
**Integrity** `[All]`: No step may fabricate, cherry-pick without disclosure, or manually alter data, test results, benchmarks, or figures. In addition:
* `[computational, publication]` Every reported number is generated from committed results, not transcribed by hand.
* `[publication]` Generative-AI images are never used as data figures. AI assistance is disclosed according to the venue's policy, as recorded in `plan/ASSUMPTIONS.md`.

(Only the `[All]` sentence applies to this plan's active profile; the bracketed lines are restated verbatim as required.)

## Operations
`plan/OPERATIONS.md` is N/A (`software.deploys = false`).
