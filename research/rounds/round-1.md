# Research Round 1 — 2026-10-03

Executed by: Opus 5.5 in a single context for all roles (tier substitution, A-016).

## R1 Decompose
Question tree written to `research/QUESTIONS.md` (Engineering branch only; `software` profile).
31 leaves across E-P (platform), E-D (DSP/acoustics), E-Q (quality/security/perf).

## R2 Breadth pass
13 web searches plus GitHub API / `git ls-remote` queries. 31 claims registered (C-001..C-031),
42 distinct sources. Coverage by instrument so far:

| Instrument | Tier 1/2 acoustic or modelling source found | Claims |
|---|---|---|
| erhu | yes (Samejima 2023; Waltham 2018; Long et al. scanning-laser) | C-008, C-009, C-029 |
| yehu (new) | yes (Zheng/Darabundit/Scavone DAFx-2026) | C-010 |
| leiqin | membrane humidity study only | C-022 |
| pipa | yes (Chen & Huang 2011; Waltham 2013) | C-014, C-015 |
| yueqin | yes (Waltham 2013; J. Wood Sci 2017) | C-015 |
| ruan | yes (Pfeifle 2011, abstract only) | C-017 |
| guzheng | yes (BioResources 2026; BJFU modal studies) | C-012, C-013 |
| guqin | yes (Waltham et al. 2016 JASA) | C-011 |
| yangqin | yes (Tsai 1991) | C-016 |
| sanxian | organology only | C-018 |
| matouqin | organology only | C-019 |
| gaohu, zhonghu, banhu, jinghu, sihu, gehu, zhuihu, liuqin, qinqin, konghou, Tier C | not yet searched | — |

## R3 Synthesis & gap analysis
1. **Version mismatch (load-bearing):** A-005 assumed JUCE 8.0.x. JUCE 9.0.0 shipped 2026-07-21, the same day as 8.0.15, and 9.0.3 shipped 2026-09-28 (C-001). The licence is unchanged (C-002), and the headless module builds (C-004). **Provisional D-001:** pin JUCE 9.0.3; fallback 8.0.15 via decision rule. This must survive the round-2 adversarial pass (breaking changes, pluginval/host compatibility) before it is final.
2. **Contradiction:** pipa fret count (C-014 vs C-031). Resolve with a Tier 1 organology source for the modern 6 xiang + 24 pin layout.
3. **Single-source load-bearing claims:** C-004 (now verified), C-008, C-009, C-010, C-012, C-014, C-015, C-016, C-017, C-025, C-028, C-029 → R4 depth pass in round 2.
4. **Inferred load-bearing claims:** C-007 (licence compatibility), C-027 (bow-friction literature) → need Tier 1 sources.
5. **Scope signal:** the yehu has the most complete published real-time physical model (C-010) but is not in A-002. Proposed D-003 entry: add yehu to Tier B (within the human's stated goal of "all instruments with reference material"; no new subsystem required).
6. **Analogy sources needed:** sanxian → shamisen acoustics; matouqin → bowed-string with wooden top / skin variant; konghou → concert harp (Waltham harp studies, C-0xx pending).
7. **Reference-audio licensing:** only CC BY sources (CCOM-HuQin, C-020) may yield committed fixtures; CC BY-NC-ND sets (C-021) are excluded from committed artefacts (proposed D-007).
8. **New leaves:** Q-D11 shamisen analogy; Q-D12 konghou/harp; Q-D13 Tier C instruments; Q-D14 huqin variants' tunings and dimensions; Q-P11 JUCE 9 BREAKING_CHANGES affecting plugin/MPE APIs.

## R4 Depth pass
Not started (round 2).

## R5 Adversarial pass (round 1, light)
- C-006: pluginval's develop branch replaced its CLI (CLI11, subcommands; PR #175). The v1.0.4 release predates this. The plan must pin the **v1.0.4 binary** and use flags documented at that tag; to be verified by spike S-02.
- C-008: the "lowest membrane mode ≈ 2 kHz" figure is Tier 3 only → must not drive a frozen test until R4 finds the primary source.
- C-012: arithmetic self-check passes (5 notes/octave × 4 octaves + 1 = 21 strings, D2→D6).
- S-01 caveat: the sandbox already had libx11-dev and libfreetype-dev, so S-01 does not prove a clean-image build. The CI plan must install dependencies explicitly.

## R6 Empirical verification
- **S-01** (`research/spikes/S-01-juce9-headless/`): JUCE 9.0.3 headless console build with juce_dsp + juce_audio_processors_headless; KS string tuned to −0.019 cents at 293.66 Hz. → C-004 verified, C-030 verified.

## Saturation status
Not saturated: new load-bearing claims were added this round. Minimum 3 rounds required.
