# chinese-strings

A physical-modelling plugin (VST3, AU on macOS, and Standalone) for Chinese stringed instruments, built
with digital waveguides. No samples.

> **Status: work in progress. There is no usable plugin yet.** This `main` branch holds only the
> licence and this file. The engine is being built on the **`impl/v0`** branch, which stopped at step
> S-008 to wait for a human listening check (gate G-003a). The planning documents are on the
> `gen-20261003T134254Z-chinese-strings-vst` branch.

## What exists on `impl/v0`

- A C++ engine library (`core/`) with a bowed voice (a two-segment waveguide plus a bow junction),
  a plucked voice, a modal body bank, fingering and tuning support (including Scala/KBM tunings).
  Only the bowed family has been through a listening check; the plucked, zither and struck
  instruments are in the data but not yet signed off.
- Instrument data for 19 instruments in `data/instruments/`:
  - bowed: erhu, yehu, gaohu, zhonghu, banhu, jinghu, sihu, matouqin
  - plucked lutes: pipa, liuqin, yueqin, xiaoruan, zhongruan, daruan, sanxian
  - zithers and harp: guzheng, guqin, konghou
  - struck: yangqin
- Command-line tools for rendering phrases to WAV and checking the pitch of every note.
- A frozen test suite. At the G-003a gate the 42 non-fuzz tests passed and a 300-second Scala/KBM
  fuzz run completed with no crash.
- The plugin itself is **not built yet**: the plugin state code is still a stub that throws
  `NotImplemented`.

The G-003a evidence renders (32 WAV files for the eight bowed instruments) are on the
`evidence/G-003a` branch. The plan says the next step (S-009) does not start until the listening
check is answered.

## Left out

No usable published acoustic data was found for gehu, diyingehu, zhuihu, qinqin, leiqin, rawap, dutar,
satar, dombra, se, zhu and yazheng, so they are not included.

## Licence

Source code: Apache-2.0 (see `LICENSE`). Plugin binaries link JUCE under the AGPLv3 and are therefore
distributed under AGPLv3 terms; see `NOTICE` on `impl/v0` for the third-party list.
