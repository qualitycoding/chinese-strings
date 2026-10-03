# GATE G-003a — bowed family (end of S-008)

Status: **waiting for your response.** I have stopped here and will not start S-009 until `GATE-G-003a.RESPONSE.md` is committed to `impl/v0`.

## Evidence

- `renders/G-003a/report.md` — test results, DD values in use, per-note pitch table, known issues, raw levels (on `impl/v0`).
- 32 WAVs (8 bowed instruments x open strings / scale / legato / vibrato; 48 kHz, 24-bit, normalised to -3 dBFS) on the branch **`evidence/G-003a`**
  of this repository, folder `renders/G-003a/`. They are kept off `impl/v0` because they are 86 MB. `SHA256SUMS` is in both places.
- Tool and phrases: `tools/cs_render/`, `tools/phrases/bowed_*.json`; pitch check: `tools/pitch_sweep/`.
- Commit: see `git log` on `impl/v0` (message begins "S-008").

## Questions (plan GATES.md)

1. Does each bowed instrument sound recognisably like itself? (erhu, yehu, gaohu, zhonghu, banhu, jinghu, sihu, matouqin)
2. Any instrument to retune? If so, which DD row (table in `report.md`)?

Things I would listen for, because I know they are the weakest points:
- banhu and yehu: the measured yehu radiation response is applied (60 % wet). Is the colour right, or too strong / too weak? DD: radiation wet share, level trim.
- banhu is 8 dB quieter than the rest in the raw output.
- sihu legato phrase around 4.2 s (a string change; it had a click, now ducked) and the sihu note 54-56 region (weaker, slightly unstable).
- bow noise: no bow-hair noise component is modelled. If it sounds too clean, tell me; that would be a design addition, not a DD change.

## Allowed responses

Create `GATE-G-003a.RESPONSE.md` on `impl/v0` containing exactly one of:

- `proceed`
- `proceed-with-changes: <instrument>: <DD field>=<value>; ...`  (example: `proceed-with-changes: yehu: radiationWet=1.0; banhu: radiationTrim=1.4`)
- `stop`

DD fields you can name (they are constants in `core/src/Engine.cpp`, `core/src/BowedVoice.cpp`, `core/src/Designs.cpp`; I will map the names for you in the DEVIATIONS entry):
`bowPosition`, `bowSpeedScale`, `bowForceLo`, `bowForceSpan`, `radiationWet`, `radiationTrim`, `bodyMix`, `outputLevel`, `attackMs`, `legatoGlideMs`.
Per-instrument values need a small change to make those constants per-instrument; I will do that if you ask for it.

## Also for you (not a gate question)

- Please revoke both GitHub tokens when we are finished. The second one is still on the sandbox.
- Bowed instrument design takes ~2 s per instrument per sample rate the first time it is selected (see report, Known issues 5).
