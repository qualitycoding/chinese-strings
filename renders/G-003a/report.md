# G-003a evidence — bowed family (8 instruments)

Branch `impl/v0`, step S-008. Files: `renders/G-003a/<instrument>_<phrase>.wav` (stereo, 48 kHz, 24-bit; 8 instruments x 4 phrases = 32 files, 86 MB,
checksums in `SHA256SUMS`). The WAVs are not on `impl/v0`: they live on the branch `evidence/G-003a` of the same repository.
Regenerate them with `tools/cs_render` (command lines below).

## What to listen to

| phrase | file suffix | what it exercises |
|---|---|---|
| open strings | `_open_strings` | each open string bowed for 1.4 s (strings an instrument lacks are skipped) |
| scale | `_scale` | major scale from the lowest open string, two octaves up and back, detached notes |
| legato | `_legato` | slurred phrase on the upper string; mono legato with a 20 ms glide |
| vibrato | `_vibrato` | one long note: straight, then 20 cents, then 45 cents at 5.5 Hz, then straight |

All WAVs are **normalised to -3 dBFS peak** so you can compare timbre without chasing volume. The raw levels are in the table at the end:
the engine's own output differs between instruments by up to about 8 dB (see Known issues).

## Test results (frozen suite, unmodified; manifest check: 25/25 files OK)

`ctest` on the Release build (no sanitizers), excluding the fuzz targets:

| group | result |
|---|---|
| T-001..T-005, T-010..T-015, T-020..T-023 | pass |
| T-030 (sounds, finite, decays), T-031 (every open string within +-3 cents), T-032 (22.05-192 kHz, blocks 1-1024) | pass |
| T-033 (60 s random automation), T-034 (polyphony, legato), T-039 (sample-accurate onset) | pass |
| T-041..T-045 (operational, EngineHost concurrency, family signature: bowed sustain / others decay) | pass |
| T-050..T-052 (tuning), T-070 (zero heap allocations), T-080 / T-081 (16-voice load <= 0.45) | pass |
| T-113 (licence notice) | pass |
| T-100 (Scala/KBM fuzz, 300 s) | 2,768,345 runs, no crash (after the fix logged in DEVIATIONS.md) |

42 of 42 non-fuzz tests pass. Not yet run: TSAN job for T-044 (CI, S-016), pluginval (S-014), T-101 fuzz (S-013).
Tests of later steps (T-035..T-038, T-040 technique/MPE/vibrato in the engine) belong to S-009 and are not part of this gate.

## Pitch of every note (`tools/pitch_sweep`, 48 kHz, default parameters, velocity 100)

Each note of each instrument's range is played alone and the settled oscillation (0.5-0.9 s) is measured. The frozen test T-031 only covers the open
strings (all pass, +-3 cents). This table covers every note:

```
erhu      notes 32  mean  -0.00 c  worst   +0.77 c  outside +-3 c:  0
yehu      notes 32  mean  -1.01 c  worst  -32.53 c  outside +-3 c:  1 95(-33)
gaohu     notes 32  mean  +0.07 c  worst   +3.53 c  outside +-3 c:  2 88(-3) 96(+4)
zhonghu   notes 32  mean  +0.04 c  worst   +1.42 c  outside +-3 c:  0
banhu     notes 32  mean  -1.90 c  worst  -74.35 c  outside +-3 c:  6 84(+5) 97(+9) 99(-36) 102(-74) 103(+10) 104(+24)
jinghu    notes 32  mean  +1.26 c  worst  +23.80 c  outside +-3 c:  5 88(+4) 99(+7) 102(-4) 103(+7) 104(+24)
sihu      notes 32  mean  +0.24 c  worst  -36.00 c  outside +-3 c:  4 52(-3) 54(+31) 55(+16) 56(-36)
matouqin  notes 30  mean  -0.04 c  worst   -0.40 c  outside +-3 c:  0
```

Numbers in brackets are the error in cents. 236 of 254 notes are within +-3 cents. Of the 18 that are not, 10 are small (3-10 cents, mostly above 1 kHz);
the eight large ones are banhu 99, 102 and 104, jinghu 104, yehu 95 and sihu 54, 55, 56 (16-74 cents). They fall in two places: very high notes on the
huqin with a loop of only 10-20 samples (banhu, jinghu, yehu), and sihu 54-56 (stopped notes on its lower string pair, where the dispersion fit puts
sharp resonances inside the band). The pitch there is right on average but the oscillation does not lock cleanly. See Known issues.

## DD parameters in use (design defaults, changeable at this gate)

| DD | value | note |
|---|---|---|
| bow velocity | (0.04 + 0.20 x BowSpeed) x (0.4 + 0.6 x velocity) m/s | plan formula was 0.05 + 0.45 x BowSpeed; see Deviations |
| bow force | (Z x v_bow / beta_eff) x (0.15 + 1.35 x BowPressure) N | plan formula was 0.05 + 1.95 x BowPressure N; Z = string wave impedance, beta_eff = bow-to-bridge fraction of the sounding length |
| bow position | 0.12 of the open string's length from the bridge | as planned; the bridge-segment length is then nudged by up to +-3 samples to keep P/Db off an integer, and by up to +-5 more where a note does not lock |
| friction | mu_s 0.8, mu_d 0.3, v0 0.2 m/s | |
| bow attack | 30 ms smoothstep | |
| legato | 20 ms pitch glide; 25 ms output duck at the note change | |
| release | -60 dB in about 0.83 s (time constant 0.12 s) | |
| body | bridge/body modes mixed in at 0.35 | |
| radiation (yehu, banhu only) | 60 % measured radiation response + 40 % dry body signal, normalised over the playing range, level trim 0.8 | |
| output level of a bowed voice | 0.6 | |
| lock partials | 7, then per-note closed-loop calibration (see below) | |

Impedances Z of the bowed strings run from 0.15 kg/s (erhu inner) to 0.68 kg/s (matouqin), a factor of 4.5, so one absolute bow force cannot sit
in the same playing window for all eight; scaling by Z and by the bow position keeps the force in proportion. For the erhu at default settings the plan's literal
formulas give a force 2.0 x the scaled reference, mine gives 0.8 x. I did not run the plan's literal formulas through the pitch sweep.

## How the pitch is made to land (for transparency)

A bowed, stiff string does not oscillate at the phase-delay fundamental of its loop; it locks near the mean of its stretched partials. The first
version therefore played erratically (errors of +-10 to +-180 cents). Now, once per instrument and sample rate (cached), every note is simulated with the real
voice and its loop length is trimmed until the settled oscillation matches the note. The note must also stay periodic and in tune at bow pressures 0.25 and
0.8. If a note will not lock, other bow-to-bridge distances are tried, then a loop without dispersion. This is a design-time cost of about 2 s per bowed
instrument per sample rate (see Known issues).

## Known issues and what is not done

1. **Bowed techniques are not implemented yet.** Pizzicato, harmonic, tremolo and glissando for the bowed row of the technique table, and the
   keyswitch-driven routing around them, belong with T-035..T-040 in S-009. The plan lists them under S-008 action 2; I moved them to S-009 where their tests are.
   The "noise suppressor" (extra damping of the body mode nearest the string's open frequency) is also not done.
2. **Level differences.** Raw levels differ by up to about 8 dB between instruments (table below): banhu is the quietest (-22 to -26 dBFS rms), erhu/zhonghu/matouqin
   the loudest. For yehu and banhu this follows from the measured yehu radiation response, which varies by about 47 dB across 100 Hz - 8 kHz; I blended it with the
   dry signal (DD above) to keep notes from vanishing in its valleys.
3. **Note-to-note level** also varies within a phrase (up to about 7 dB in the erhu legato phrase): notes that settle into a less clean motion are weaker.
4. **The 18 notes outside +-3 cents** listed above.
5. **Design cost:** the first time a bowed instrument is selected at a given sample rate takes about 2 s on this sandbox (single core). Test run time grew
   accordingly (T-043 / T-044 about 45 s each, before TSAN). This will be revisited with the performance step (S-015, DR-03); a precomputed table is the likely fix.
6. Not checked by ear by me, naturally. I verified numerically that the legato notes land within +-0.8 cents and that commanded vibrato depths of 20 and 45 cents
   come out at about 19 and 36 cents amplitude (120 ms analysis windows). Whether it sounds like an erhu is your call.

## Renders: raw levels

| instrument | phrase | length s | raw peak dBFS | raw rms dBFS | gain applied (to reach -3 dBFS peak) |
|---|---|---|---|---|---|
| erhu | open_strings | 8.70 | -3.4 | -17.4 | 0.4 dB |
| erhu | scale | 16.00 | -3.4 | -16.1 | 0.4 dB |
| erhu | legato | 7.50 | -3.2 | -12.5 | 0.2 dB |
| erhu | vibrato | 6.50 | -1.9 | -11.0 | -1.1 dB |
| yehu | open_strings | 8.70 | -6.4 | -19.7 | 3.4 dB |
| yehu | scale | 16.00 | -4.1 | -19.2 | 1.1 dB |
| yehu | legato | 7.50 | -4.2 | -17.3 | 1.2 dB |
| yehu | vibrato | 6.50 | -8.8 | -19.5 | 5.8 dB |
| gaohu | open_strings | 8.70 | -2.8 | -16.4 | -0.2 dB |
| gaohu | scale | 16.00 | -2.9 | -16.4 | -0.1 dB |
| gaohu | legato | 7.50 | -2.3 | -13.8 | -0.7 dB |
| gaohu | vibrato | 6.50 | -4.1 | -15.2 | 1.1 dB |
| zhonghu | open_strings | 8.70 | -3.8 | -16.1 | 0.8 dB |
| zhonghu | scale | 16.00 | -3.5 | -14.9 | 0.5 dB |
| zhonghu | legato | 7.50 | -3.1 | -12.0 | 0.1 dB |
| zhonghu | vibrato | 6.50 | -3.2 | -11.1 | 0.2 dB |
| banhu | open_strings | 8.70 | -9.0 | -22.1 | 6.0 dB |
| banhu | scale | 16.00 | -9.3 | -24.6 | 6.3 dB |
| banhu | legato | 7.50 | -12.6 | -24.4 | 9.6 dB |
| banhu | vibrato | 6.50 | -14.6 | -25.8 | 11.6 dB |
| jinghu | open_strings | 8.70 | -8.6 | -20.8 | 5.6 dB |
| jinghu | scale | 16.00 | -6.9 | -18.0 | 3.9 dB |
| jinghu | legato | 7.50 | -6.6 | -16.9 | 3.6 dB |
| jinghu | vibrato | 6.50 | -6.5 | -15.7 | 3.5 dB |
| sihu | open_strings | 8.70 | -2.0 | -13.0 | -1.0 dB |
| sihu | scale | 16.00 | -2.1 | -15.6 | -0.9 dB |
| sihu | legato | 7.50 | -1.4 | -12.1 | -1.6 dB |
| sihu | vibrato | 6.50 | -6.5 | -13.9 | 3.5 dB |
| matouqin | open_strings | 8.70 | -1.0 | -13.5 | -2.0 dB |
| matouqin | scale | 16.00 | -2.4 | -15.2 | -0.6 dB |
| matouqin | legato | 7.50 | -0.2 | -11.1 | -2.8 dB |
| matouqin | vibrato | 6.50 | -1.5 | -11.4 | -1.5 dB |

Reproduce one: `cs_render --instrument erhu --phrase tools/phrases/bowed_scale.json --fs 48000 --normalize -3 --out erhu_scale.wav`
