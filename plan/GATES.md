# Human gates (Rule 10, §3.3)

At a gate the implementer halts, writes `GATE-<id>.md` at the repository root of the implementation branch (evidence summary + links), commits and pushes it, and waits. The human replies by committing `GATE-<id>.RESPONSE.md` containing exactly one allowed response line. The implementer resumes only after reading that file.

G-001: N/A (`math`/`computational` inactive — PROFILE.md).

| Gate | Trigger | Evidence bundle the implementer assembles | Questions for the human | Allowed responses | Branch taken |
|---|---|---|---|---|---|
| G-003a | End of S-008 (bowed family complete) | `renders/G-003a/*.wav` from `cs_render` (each bowed instrument: open strings, scale, legato phrase, vibrato phrase; 48 kHz/24-bit); `renders/G-003a/report.md` (T-0xx results, DD parameters in use) | 1. Does each bowed instrument sound recognisably like itself? 2. Any instrument to retune (which DD rows)? | `proceed`; `proceed-with-changes: <instrument>: <DD field>=<value>; ...`; `stop` | proceed → S-009. proceed-with-changes → edit only the named DD rows (DR-10), re-render, re-run suite, re-open G-003a once. stop → write STOPPED.md, halt. |
| G-003b | End of S-010 (plucked lutes) | `renders/G-003b/*.wav` (each lute: open strings, chromatic run, tremolo, bend, harmonic) + report.md | same as G-003a for plucked lutes | same | proceed → S-011; changes → as G-003a; stop → halt |
| G-003c | End of S-011 (zithers & harp) | `renders/G-003c/*.wav` (guzheng glissando, press-bend, yaozhi; guqin harmonics & slides; konghou bends) + report.md | same for zithers/harp | same | proceed → S-012; changes → as G-003a; stop → halt |
| G-003d | End of S-012 (yangqin) | `renders/G-003d/*.wav` (single strikes across range, rolls, damped) + report.md | same for yangqin | same | proceed → S-013; changes → as G-003a; stop → halt |
| G-004 | End of S-016 (CI green on all jobs) | CI run URL; downloadable VST3/AU/Standalone artifacts; `GATE-G-004.md` checklist: load in Reaper on Windows 11 (A-017), instrument selector, UI highlight (SC-17), MPE in Reaper VST3 (D-015), 16-voice CPU on the human's laptop (A-012 absolute check) | 1. Plugin loads and plays in Reaper? 2. UI highlights correct strings? 3. MPE works in Reaper VST3? (if not: accept as documented limitation?) 4. CPU acceptable? | `proceed`; `proceed-with-limitation: <text>` (recorded in README "Known limitations"); `fix: <text>` (implementer fixes within existing frozen tests, re-opens G-004); `stop` | proceed / proceed-with-limitation → S-017; fix → back to the relevant step; stop → halt |
| G-002 | End of S-017 (release candidate ready) | PR from the implementation branch to `main` (opened, not merged); release-notes draft; artifact checksums; licence summary (D-002) | Merge to `main`? Create tag `v0.1.0` and a GitHub Release with CI artifacts? Sign/notarise macOS (needs Apple credentials)? | `merge-only`; `merge-and-release`; `hold` | The implementer never merges, tags or releases itself. On any response it records the response in DEVIATIONS.md and halts — the human performs merge/tag/release. |

## Blind listening augmentation for G-003a–d (D-025)

Applies to every family gate G-003a/b/c/d as a supplement to the rows above;
original evidence bundles remain in force and are not overwritten.

- **audio = true** (plan/PROFILE.md); realism comparison against matched,
  redistribution-compatible real recordings of each named instrument. Never
  substitute a different instrument; never score against placeholder voices.
- **Frozen settings:** 12 onset pairs per active gate, `--window start`,
  1.0 s clips, common RMS −23 dBFS, 10 ms fades, 44.1 kHz PCM-24; pass =
  ≤ 8/12 correct AND acceptable per-instrument demos AND no unresolved
  qualitative finding; maximum three rounds per gate; base seed 20261105
  per gate, rounds 2/3 add 1000/2000; p-value reported exactly; general
  mechanisms only, never per-clip constants.
- **Pair manifest** must identify instrument/technique coverage per
  instrument so a family-wide score cannot hide a badly failing instrument.
- **Result record:** `listening/G-00Xx/round-00N/` per the common schema;
  the native checkpoint and `.impl/state.json` gain a listening pointer, not
  a replacement.
- **Questions:** which side is the real instrument; is each instrument
  recognisable; which DD row or general mechanism explains any retuning
  need. Existing weak-spot questions and known issues are reported even when
  the score passes.
- **Allowed responses (additions):** `iterate: <notes>` — non-DD findings
  requiring a general algorithm change via the normal plan/test amendment
  path (never undocumented DD edits); new seed. `proceed-with-rescope:
  <text>` — narrowed claim. `proceed-with-changes` remains the DD-tuning
  iteration response; DD rows editable only under DR-10, cited values never
  changed (DR-02).
- **Resume mapping unchanged:** proceed on the active gate resumes exactly
  the step named in the G-003a–d table.
- **Status:** G-003a round-001 blocked — references not secured
  (`listening/G-003a/round-001/result.json`).
