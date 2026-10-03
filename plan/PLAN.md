# Execution plan — chinese-strings

(Status header is written in Phase 5.)

## Conventions used by every step
- **Repo root** = a clone of `https://github.com/qualitycoding/chinese-strings`. **Implementation branch** `impl/v0`, created in S-001 from the head of the generation branch named in HANDOFF.md. All implementer commits go to `impl/v0`; nothing is pushed to `main`.
- **Build (Linux, Release):** `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build -j`
- **Run tests by ID:** `ctest --test-dir build --output-on-failure -R "<regex>"` where test names begin with their ID (e.g. `^T-02[0-3]`).
- **Freeze check (before every commit):** `sha256sum -c tests/FROZEN_MANIFEST.sha256` must print only `OK` lines.
- **Implementer checkpoint:** `.impl/state.json` = `{"step":"S-###","status":"started|done","commit":"<sha>","updated_at":"<UTC>"}`; write `started` before a step's first action and `done` after its Done-when passes; commit it with the step's last commit (message `S-###: <title>`).
- **Idempotency:** every step can be re-run from its first action; generated files are overwritten, never appended. Partial completion is detected by `.impl/state.json` = `started` for that step: re-run the step from action 1.
- **Retry policy (default, unless the step overrides):** fix-and-rerun the failing command up to 5 attempts; then apply DR-02 (DECISIONS.md, "Decision rules").
- **DD** = design default (plan/INSTRUMENT_DATA.md). Only DD rows may be tuned, only at a G-003x response, logged per DR-10.

---

### S-001 Bootstrap, environment and freeze verification
- Tier: Sonnet
- Profile: software
- Depends on: none
- Inputs: HANDOFF.md, plan/ENVIRONMENT.md, tests/FROZEN_MANIFEST.sha256, tests/RED_REPORT.md
- Actions:
  1. `git clone https://github.com/qualitycoding/chinese-strings && cd chinese-strings && git fetch origin && git checkout <generation branch from HANDOFF.md> && git checkout -b impl/v0`
  2. Run the Linux setup commands in plan/ENVIRONMENT.md verbatim.
  3. `sha256sum -c tests/FROZEN_MANIFEST.sha256`
  4. Build (Conventions) and run `ctest --test-dir build --output-on-failure > red.log 2>&1 || true`.
  5. `mkdir -p .impl` and write `.impl/state.json` for S-001; commit `S-001: bootstrap`; `git push -u origin impl/v0`.
- Outputs: branch `impl/v0`; `.impl/state.json`
- Evidence produced: none
- Done when: manifest check prints 23 `OK` lines; `red.log` shows `38 tests failed out of 42`; the 4 SUPPORT-* tests pass.
- Checkpoint: `.impl/state.json` step S-001 done
- On failure: setup command fails → DR-01; manifest mismatch → halt, write BLOCKED.md (frozen files altered); red counts differ → halt, BLOCKED.md with red.log attached.
- Gate: none
- Relevant decisions/claims: D-001, D-005, C-038, C-060

### S-002 Licence, NOTICE and repository hygiene
- Tier: Haiku
- Profile: software
- Depends on: S-001
- Inputs: plan/deps.lock.json, D-002, research/spikes/S-07-yehu-body/LICENSE.yehudafx26
- Actions:
  1. Create `NOTICE` listing, one per line with licence: "JUCE 9.0.3 — AGPLv3 (binaries of this plugin are distributed under AGPLv3)", "Steinberg VST3 SDK (bundled in JUCE) — MIT", "Catch2 v3.16.0 — BSL-1.0 (tests only)", "surge-synthesizer tuning-library 48422e2 — MIT", "yehudafx26 284e5be — MIT (bridge-mode and radiation data, Copyright (c) 2026 Champ C. Darabundit and Zhen Zhang)", "pluginval v1.0.4 — GPLv3 (CI tool, not distributed)".
  2. Copy `research/spikes/S-07-yehu-body/LICENSE.yehudafx26` to `third_party/yehudafx26/LICENSE`.
  3. Replace README.md "Licence" paragraph with: "Source code: Apache-2.0 (LICENSE). Distributed plugin binaries link JUCE under the AGPLv3; they are therefore distributed under AGPLv3 terms. See NOTICE." Add a "Dropped instruments" section listing the D-003 drop list and "Known limitations" (empty heading).
  4. Create `.gitignore` with `build*/`, `renders/*.wav`, `.DS_Store`.
- Outputs: NOTICE, third_party/yehudafx26/LICENSE, README.md, .gitignore
- Evidence produced: T-113
- Done when: `python3 tools/check_notice.py` prints `T-113 PASS`.
- Checkpoint: S-002 done
- On failure: default retry.
- Gate: none
- Relevant decisions/claims: D-002, C-005, C-007, C-036, C-067

### S-003 Instrument data generator and 19 data files
- Tier: Sonnet
- Profile: software
- Depends on: S-002
- Inputs: plan/INSTRUMENT_DATA.md, core/include/cs/Instruments.h, research/spikes/S-07-yehu-body/yehu_body.json
- Actions:
  1. Copy `research/spikes/S-07-yehu-body/yehu_body.json` to `data/bodies/yehu_body.json`.
  2. Write `data/instruments/<key>.json` for all 19 keys exactly per INSTRUMENT_DATA.md rows and global rules (schema: `{"key","displayName","family","maxPolyphony","stopsAllStringsTogether","midiLow","midiHigh","citations":[...],"strings":[{"openHz","vibratingLengthM","tensionN","densityKgM3","radiusM","youngsModulusPa","sigma0","sigma1","material","courses","courseDetuneCents"}],"fretSemitones":[...],"bodyModes":[{"freqHz","t60s","gain"}],"radiation":"none"|"yehu"}`). `openHz` = 440·2^((midi−69)/12) computed in double precision; tension per global rule 1.
  3. Write `tools/gen_instruments.py` (stdlib only): reads all JSON, validates schema and the T-001..T-003 invariants, emits `generated/InstrumentTables.cpp` defining `cs::spec`, `cs::instrumentFromKey` (and the yehu radiation table as a static array).
  4. In root CMakeLists.txt add `add_custom_command` producing `${CMAKE_BINARY_DIR}/generated/InstrumentTables.cpp` from the script + `data/**/*.json` (DEPENDS on each file) and add it to `cs_core`; remove `spec`/`instrumentFromKey` stubs from core/src/Stubs.cpp; implement `inharmonicityB` and `supportsTechnique` (table in INSTRUMENT_DATA.md) in `core/src/Instruments.cpp`.
- Outputs: data/instruments/*.json (19), data/bodies/yehu_body.json, tools/gen_instruments.py, core/src/Instruments.cpp, CMakeLists.txt, core/src/Stubs.cpp
- Evidence produced: T-001, T-002, T-003, T-004
- Done when: `ctest -R "^T-00[1-4]"` all pass; manifest check OK.
- Checkpoint: S-003 done
- On failure: T-002/T-003 failure on a cited row → the data file is wrong, fix data (never the test); if a cited value itself appears wrong → DR-02 (TEST_CHALLENGE.md).
- Gate: none
- Relevant decisions/claims: D-003, D-011, D-018, D-019, C-012, C-032, C-034, C-039..C-049, C-067

### S-004 DSP blocks I: fractional delay, bow junction, contact, modal bank
- Tier: Opus
- Profile: software
- Depends on: S-003
- Inputs: core/include/cs/Dsp.h, C-062 (Smith PASP bow junction), C-028 (DAFx26 eq. 4–5)
- Actions:
  1. `core/src/FractionalDelay.cpp`: integer delay line + first-order Thiran allpass for the fractional part, delay range [1, maxDelay].
  2. `core/src/BowJunction.cpp`: friction coefficient mu(v) = muD + (muS−muD)·v0/(v0+|v|); solve the Smith/MSW load-line for v_delta (R_b(v)·v = R_s·(v+ − v)) by bracketed Newton (≤ 20 iterations, tolerance 1e-9); return rho_hat = r/(1+r), r = 0.25·R_b/R_s; return exactly 0 when bowForce = 0.
  3. `core/src/Contact.cpp`: Hunt–Crossley as documented in Dsp.h.
  4. `core/src/ModalBank.cpp`: one two-pole resonator per mode, pole radius exp(−3 ln10/(T60·fs)), angle 2π f/fs, b0 = gain·(1−r).
  5. Remove the corresponding stubs.
- Outputs: the four .cpp files; core/src/Stubs.cpp; CMakeLists.txt (sources)
- Evidence produced: T-012, T-013, T-014, T-015
- Done when: `ctest -R "^T-01[2-5]"` pass.
- Checkpoint: S-004 done
- On failure: default retry, then DR-02.
- Gate: none
- Relevant decisions/claims: D-004, D-004a, C-028, C-062

### S-005 DSP blocks II: loss filter, dispersion filter, waveguide string
- Tier: Opus
- Profile: software
- Depends on: S-004
- Inputs: Dsp.h, C-063 (loss law), C-037 (stiffness), C-064 (passive pluck junction), C-068, research/spikes/S-08-loss-fit/fir_fit.py, research/spikes/S-01-juce9-headless/main.cpp (fractional tuning reference)
- Actions:
  1. `LossFilter`: zero-phase symmetric FIR with 2M+1 taps, designed exactly as `research/spikes/S-08-loss-fit/fir_fit.py` `design()` (weighted linear least squares on |H| at harmonics 1..min(20, 0.45 fs/f0) plus 64 points to fs/2, weights 1/(|Ht|·|ln Ht|), then scaled so max|H| ≤ 1 on a 4097-point grid); M = smallest value in 1..48 giving ≤ 3 % max relative ln error over harmonics 1..min(10, 0.4 fs/f0) (C-068: one-pole designs cannot meet T-010). The filter's constant M-sample delay is part of the loop delay. `magnitudeAt` evaluates |c0 + 2 Σ ck cos(k w)|.
  2. `DispersionFilter`: N first-order allpass sections with equal coefficient a chosen to minimise the squared phase-delay error versus the stiff-string target over partials 1..10 (Rauhala/Välimäki-style design by 1-D golden-section search on a ∈ (−0.95, 0)); B = 0 → a = 0.
  3. `WaveguideString`: single-loop DWG (delay line + dispersion + loss + fractional delay); `setFrequency` sets the total loop delay to fs/f1 minus the dispersion phase delay at f1 and the loss filter delay M (pitch includes stiffness compensation); the loss filter is designed for the effective length L·openHz/f; `pluck` uses the passive pluck scattering junction (spring finger, k = 2000 N/m DD) applied for 2 ms; `lossCoefficientsMagnitude(n)` returns |H| of the realised loop filters on n+1 points over [0, fs/2].
  4. Remove stubs.
- Outputs: core/src/LossFilter.cpp, DispersionFilter.cpp, WaveguideString.cpp; Stubs.cpp; CMakeLists.txt
- Evidence produced: T-010, T-011, T-020, T-021, T-022, T-023
- Done when: `ctest -R "^T-01[01]|^T-02[0-3]"` pass.
- Checkpoint: S-005 done
- On failure: T-022 below 0.70× → too much damping in loop filters (check fractional-delay/dispersion loss); above 1.10× → loss filter under-damps; never change tolerances; after the retry budget → DR-02.
- Gate: none
- Relevant decisions/claims: D-004, D-004a, C-030, C-037, C-063, C-064

### S-006 Tuning tables and Scala import (+ fuzz)
- Tier: Sonnet
- Profile: software
- Depends on: S-003
- Inputs: core/include/cs/Tuning.h, D-010, D-010a, C-042, C-043
- Actions:
  1. Add `FetchContent_MakeAvailable(tuning_library)` and link its include dir privately to cs_core.
  2. `core/src/Tuning.cpp`: `equal(a4)`; `loadScala`: size limits, call `Tunings::parseSCLData` / `parseKBMData` inside `try{}catch(...)`, apply D-010a validation, build `Tunings::Tuning` (default KBM when kbm empty), copy `frequencyForMidiNote(0..127)`.
  3. Build fuzzers (TESTS.md commands) and run `fuzz_scala` for 300 s.
- Outputs: core/src/Tuning.cpp; CMakeLists.txt
- Evidence produced: T-050, T-051, T-052, T-100
- Done when: `ctest -R "^T-05"` pass; `fuzz_scala -max_total_time=300 -seed=1 tests/fuzz/corpus_scala` exits 0 and writes no `crash-*` file.
- Checkpoint: S-006 done
- On failure: a fuzz crash → fix loader, add the crash input to `tests/fuzz/regressions/` (new, not frozen), rerun.
- Gate: none
- Relevant decisions/claims: D-010, D-010a, C-042, C-043, C-053

### S-007 Engine core
- Tier: Opus
- Profile: software
- Depends on: S-005, S-006
- Inputs: Engine.h, Fingering.h, D-009, D-015, D-020, D-021, D-022
- Actions:
  1. `core/src/Engine.cpp` (pimpl): `prepare` validates per Engine.h (throws std::invalid_argument), pre-allocates `kFreePolyphonyVoices` voices sized for the lowest instrument string at that fs, resets all voices; `setInstrument` resets voices and reconfigures strings/body/radiation (radiation converted per INSTRUMENT_DATA.md rule 5); render before prepare writes zeros.
  2. MIDI: note on/off (velocity 0 = off), pitch bend (D-020 normalisation), CC1 value v scales VibratoDepth by v/127 (v defaults to 127 until the first CC1 is received; reset to 127 by prepare), CC11 = expression (bow pressure / velocity scaling), channel pressure → bow pressure (bowed) or brightness (others); sample-accurate event scheduling inside `render` via a fixed-size event queue (capacity 512, overflow drops oldest).
  3. Notes outside [midiLow, midiHigh] of the current instrument and keyswitch notes (12..18) never start voices. Voice allocation: realistic (`maxPolyphony`, oldest-note stealing) or free (16); bowed mono legato with 60 ms glide (D-022); string choice via `fingeringFor`; zithers play the nearest string and bend by the remaining interval.
  4. Note-off damping per D-022; parameters with 10 ms smoothing; `paramRange`/`paramKey` per Engine.h table; `fingeringFor` per D-021.
  5. Output: body modal bank → radiation (if any) → gain; stereo = identical L/R in v0.
  5a. Until S-008/S-010/S-011/S-012 replace them, every family uses a **placeholder voice**: `WaveguideString` + `pluck(PluckPosition)` with the instrument's string data and body (sustained families simply ring). The placeholder is deleted family by family as real voices land.
  6. Remove remaining engine stubs.
- Outputs: core/src/Engine.cpp, core/src/Fingering.cpp; Stubs.cpp
- Evidence produced: T-005, T-030, T-031, T-032, T-033, T-034, T-041, T-042, T-043, T-070
- Done when: `ctest -R "^T-005|^T-03[0-4]|^T-04[1-3]|^T-070"` pass for all 19 instruments with placeholder voices.
- Checkpoint: S-007 done
- On failure: T-070 allocations → move allocation to prepare/setInstrument; default retry then DR-02.
- Gate: none
- Relevant decisions/claims: D-009, D-015, D-020, D-021, D-022, C-055, C-056

### S-008 Bowed family (8 instruments) and render tool
- Tier: Opus
- Profile: software
- Depends on: S-007
- Inputs: C-010, C-027, C-034, C-048, C-062, data/bodies/yehu_body.json
- Actions:
  1. Bowed voice: two DWG segments (nut↔bow, bow↔bridge) joined by `BowJunction`; bow velocity = 0.05 + 0.45·BowSpeed m/s; bow force = 0.05 + 1.95·BowPressure N (DD); bow position 0.12 of vibrating length from the bridge (DD); both strings of `stopsAllStringsTogether` instruments stopped at the same position; noise suppressor = extra damping of the body mode nearest the string's open frequency (DD factor 0.5 on T60).
  2. Pizzicato/harmonic/tremolo/glissando per INSTRUMENT_DATA.md technique table (bowed row).
  3. `tools/cs_render/` console target (JUCE-free): `cs_render --instrument <key> --phrase <file.json> --fs 48000 --out <file.wav>` (24-bit WAV writer in-tree); phrase files `tools/phrases/bowed_*.json` (open strings, scale, legato, vibrato).
  4. Render the G-003a bundle to `renders/G-003a/`; write report.md.
- Outputs: core/src/BowedVoice.cpp, tools/cs_render/*, tools/phrases/*.json, renders/G-003a/report.md
- Evidence produced: T-030 and T-031 for bowed instruments
- Done when: `ctest -R "^T-0([0-2]|3[0-4]|4[1-3])|^T-070"` passes (regression across all instruments, bowed now real); G-003a response received.
- Checkpoint: S-008 done; record the G-003a response verbatim in `.impl/notes.md`
- On failure: bowed pitch outside ±3 cents → adjust bow position/force DD values (DR-10) before touching algorithms; DR-02 after budget.
- Gate: G-003a
- Relevant decisions/claims: D-004a, D-017, D-019, D-022, C-009, C-010, C-034, C-062

### S-009 Techniques, vibrato, pitch bend, MPE, tuning in engine
- Tier: Sonnet
- Profile: software
- Depends on: S-008
- Inputs: D-015, D-017, Engine.h
- Actions:
  1. Keyswitches 12..18 → technique (falls back to Default where unsupported); keyswitch notes never start voices.
  2. Vibrato LFO (sine, rate/depth params, CC1 scales depth); pitch bend per D-020; MPE lower zone (master ch 1, members 2–16) using juce-free reimplementation of zone logic: per-channel bend/pressure applies to the voice started on that channel.
  3. `setTuning` replaces the equal-temperament table used for note→Hz.
- Outputs: core/src/Engine.cpp (extended), core/src/Techniques.cpp
- Evidence produced: T-035, T-036, T-037, T-038, T-039, T-040
- Done when: `ctest -R "^T-03[5-9]|^T-040"` pass.
- Checkpoint: S-009 done
- On failure: default.
- Gate: none
- Relevant decisions/claims: D-015, D-017, C-056, C-066

### S-010 Plucked lutes (7 instruments)
- Tier: Opus
- Profile: software
- Depends on: S-009
- Inputs: C-014, C-015, C-017, C-041, C-064, INSTRUMENT_DATA.md
- Actions:
  1. Plucked voice: `WaveguideString` + passive pluck at PluckPosition; frets quantise the stop position to `fretSemitones`; fretless sanxian continuous; tremolo = repeated plucks at 12 Hz (lunzhi DD); bend = continuous pitch change of the stopped length; muted = loss boost ×4 (DD); harmonic = touch-node damping at 1/2, 1/3, 1/4 of the string.
  2. Sanxian body: generic-membrane modal set (DD) in place of wood.
  3. Phrase files `tools/phrases/plucked_*.json`; render G-003b bundle; report.md.
- Outputs: core/src/PluckedVoice.cpp; tools/phrases/plucked_*.json; renders/G-003b/report.md
- Evidence produced: T-030, T-031, T-035 for plucked keys
- Done when: `ctest -R "^T-0[0-4]|^T-070"` passes; G-003b response received.
- Checkpoint: S-010 done
- On failure: DR-10 for DD timbre parameters; DR-02.
- Gate: G-003b
- Relevant decisions/claims: D-004, D-017, D-019, C-041, C-064

### S-011 Zithers and harp (guzheng, guqin, konghou)
- Tier: Opus
- Profile: software
- Depends on: S-010
- Inputs: C-011, C-012, C-040, C-065, INSTRUMENT_DATA.md
- Actions:
  1. Zither voice: plucked voice with no frets; left-hand press-bend = tension increase on the bridge-left segment, modelled as pitch change up to +2 semitones (D-017 Glissando/vibrato on CC1); glissando (guazou) = sequence of plucks across strings at 25 ms spacing in the played direction; guqin harmonics (fanyin) as plucked harmonic; guqin stopped slides = continuous stop position on a 1.10 m string.
  2. Konghou: string pairs; press-bend per string bridge; sympathetic coupling among strings through the shared body modal bank (C-065) — every sounding voice feeds the modal bank; free strings not simulated (DD).
  3. Render G-003c bundle; report.md.
- Outputs: core/src/ZitherVoice.cpp; tools/phrases/zither_*.json; renders/G-003c/report.md
- Evidence produced: T-030, T-031, T-034, T-035 for zither keys
- Done when: `ctest -R "^T-0[0-4]|^T-070"` passes; G-003c response received.
- Checkpoint: S-011 done
- On failure: DR-10, DR-02.
- Gate: G-003c
- Relevant decisions/claims: D-004, D-017, C-012, C-040, C-065

### S-012 Yangqin
- Tier: Opus
- Profile: software
- Depends on: S-011
- Inputs: C-016, C-028, INSTRUMENT_DATA.md
- Actions:
  1. Struck voice: one `WaveguideString` per course member (detuned symmetrically by courseDetuneCents), excited by a bamboo-hammer `HuntCrossleyContact` (K = 1e9, alpha = 2.5, beta = 1e-4, hammer mass 4 g — DD) at 1/8 of the length; roll (lunyin) = alternating strikes at 16 Hz.
  2. Body: INSTRUMENT_DATA.md yangqin modal set.
  3. Render G-003d bundle; report.md.
- Outputs: core/src/StruckVoice.cpp; tools/phrases/yangqin_*.json; renders/G-003d/report.md
- Evidence produced: T-030, T-031, T-034, T-035 for yangqin; full `cs_tests` suite green
- Done when: `ctest --test-dir build -L "unit|integration|acoustic|operational|security" -E "^T-09"` all pass (plugin-state tests T-09x belong to S-013); G-003d response received.
- Checkpoint: S-012 done
- On failure: DR-10, DR-02.
- Gate: G-003d
- Relevant decisions/claims: D-004, C-016, C-028

### S-013 Plugin state codec (+ fuzz)
- Tier: Sonnet
- Profile: software
- Depends on: S-006
- Inputs: plugin/core/include/cs/StateCodec.h, D-016, C-057
- Actions:
  1. `plugin/core/src/StateCodec.cpp` replacing the stub: XML `<cs-state schema="cs-state" version="1" instrument="…"><param key="…" value="…"/>…<scl>…</scl><kbm>…</kbm></cs-state>` via `juce::AudioProcessor::copyXmlToBinary` equivalent (magic + length + UTF-8 XML, implemented with juce::MemoryOutputStream to avoid needing an AudioProcessor); decode per header comment, catching all exceptions.
  2. Build and run `fuzz_state` for 300 s.
- Outputs: plugin/core/src/StateCodec.cpp; CMakeLists.txt
- Evidence produced: T-090, T-091, T-092, T-101
- Done when: `ctest -R "^T-09"` pass; fuzz_state 300 s clean.
- Checkpoint: S-013 done
- On failure: default; fuzz crash → regression input as in S-006.
- Gate: none
- Relevant decisions/claims: D-016, C-057

### S-014 JUCE plugin shell and UI
- Tier: Opus
- Profile: software
- Depends on: S-012, S-013
- Inputs: D-005, D-013, D-015, D-020, D-021, A-008, research/spikes/S-02-vst3-pluginval (CMake reference)
- Actions:
  1. `plugin/` target via `juce_add_plugin(ChineseStrings COMPANY_NAME qualitycoding PLUGIN_MANUFACTURER_CODE Qcod PLUGIN_CODE Cs01 FORMATS VST3 AU Standalone IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE PRODUCT_NAME "Chinese Strings")` (AU only on Apple).
  2. Processor: AudioProcessorValueTreeState with one parameter per ParamId (ranges from `paramRange`) + `instrument` choice (19) + `mpe` bool; calls `cs::Engine`; state via StateCodec; `isBusesLayoutSupported` stereo out only.
  3. Editor: programmatic vector illustration per family (D-013) drawn from data (string count, lengths); highlight the strings/positions of `voiceInfo` at 30 Hz via a lock-free snapshot; parameter panel; Scala load button (file chooser → loadScala).
  4. Build all formats locally (Linux: VST3 + Standalone) and run `tests/ci/pluginval.sh` (Linux).
- Outputs: plugin/CMakeLists.txt, plugin/src/*.cpp/.h; root CMakeLists.txt
- Evidence produced: T-110 (Linux)
- Done when: `tests/ci/pluginval.sh <pluginval> build/plugin/ChineseStrings_artefacts/Release/VST3/Chinese\ Strings.vst3` prints `T-110 PASS`.
- Checkpoint: S-014 done
- On failure: pluginval failure → fix; never lower strictness (DR-05).
- Gate: none
- Relevant decisions/claims: D-005, D-006, D-013, D-015, D-016, C-038, C-050

### S-015 Performance
- Tier: Opus
- Profile: software
- Depends on: S-014
- Inputs: D-008, C-054
- Actions:
  1. Run `ctest --test-dir build -L perf --output-on-failure` (Release).
  2. If it fails: apply DR-03 in order, re-running after each change.
- Outputs: changed sources only as required
- Evidence produced: T-080
- Done when: T-080 passes locally (Release).
- Checkpoint: S-015 done
- On failure: DR-03.
- Gate: none
- Relevant decisions/claims: A-012, D-008, C-035, C-054

### S-016 Continuous integration
- Tier: Sonnet
- Profile: software
- Depends on: S-015
- Inputs: D-006, D-012, D-012a, D-012b, D-014, plan/deps.lock.json, tests/TESTS.md
- Actions:
  1. `.github/workflows/ci.yml`: jobs `linux` (ubuntu-24.04), `windows` (windows-2025), `macos` (macos-26, universal), `macos-intel` (macos-26-intel, pluginval on the x86_64 slice), `fuzz` (ubuntu-24.04, clang, T-100/T-101 300 s each), `deps` (ubuntu-24.04: `python3 tools/osv_check.py`, `python3 tools/check_notice.py`). Every job first runs `sha256sum -c tests/FROZEN_MANIFEST.sha256` (on macOS `shasum -a 256 -c`). Build/test commands from plan/ENVIRONMENT.md; perf label run with `ctest -L perf --repeat until-pass:3` (pre-declared in D-008); pluginval via `tests/ci/pluginval.sh` (download URL from deps.lock.json); macOS also `tests/ci/auval.sh`. Upload VST3/AU/Standalone artifacts.
  2. On first successful downloads, record the sha256 of `pluginval_macOS.zip` and `pluginval_Windows.zip` in plan/deps.lock.json (`sha256_macos_zip`, `sha256_windows_zip`) and make the workflow verify them.
  3. Push; iterate until all jobs are green.
- Outputs: .github/workflows/ci.yml; plan/deps.lock.json
- Evidence produced: T-110 (all OS), T-111, T-112, T-113, T-100, T-101 in CI
- Done when: one workflow run on the head of `impl/v0` has every job green; then open G-004.
- Checkpoint: S-016 done (record run URL)
- On failure: DR-04 (vulnerability), DR-06 (JUCE 9), DR-07 (Windows generator), DR-08 (auval); default retry otherwise.
- Gate: G-004
- Relevant decisions/claims: D-006, D-012, D-012a, D-012b, D-014, C-051, C-058, C-059, C-061

### S-017 Release candidate (no release)
- Tier: Sonnet
- Profile: software
- Depends on: S-016 and G-004 = proceed or proceed-with-limitation
- Inputs: G-004 response; CI artifacts
- Actions:
  1. Update README: instrument list (19), techniques/keyswitch chart (D-017), MPE note (D-015 + any G-004 limitation), dropped instruments, licence.
  2. Write `RELEASE_NOTES_v0.1.0.md` and `CHECKSUMS.txt` (sha256 of every CI artifact).
  3. `gh pr create --base main --head impl/v0 --title "chinese-strings v0.1.0" --body-file RELEASE_NOTES_v0.1.0.md` (or via the GitHub web UI if gh is unavailable). Do not merge.
  4. Write GATE-G-002.md; halt.
- Outputs: README.md, RELEASE_NOTES_v0.1.0.md, CHECKSUMS.txt, GATE-G-002.md, open PR
- Evidence produced: none
- Done when: PR open with green CI; GATE-G-002.md pushed.
- Checkpoint: S-017 done
- On failure: default.
- Gate: G-002
- Relevant decisions/claims: D-002, D-015, A-014
