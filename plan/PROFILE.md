# Active Profiles & Applicability (Rule 0)

## Active profiles
| Profile | Active | Justification |
|---|---|---|
| `software` | yes | The deliverable is a C++/JUCE plugin intended to be built, run in DAWs, and maintained. |
| `math` | no | No mathematical statements or proofs are deliverables (A-001). |
| `computational` | no | DSP accuracy is enforced as software tests against published acoustic measurements; no numerical research result is produced (A-001). |
| `publication` | no | No manuscript or archive deposit is in scope (A-001). |

## Mode flags
| Flag | Value | Justification |
|---|---|---|
| `software.deploys` | false | No running service. CI produces build artifacts only; any public GitHub Release is out of plan scope and gated by G-002 (A-014). |
| `math.exploration` | n/a | `math` inactive. |

## Not-applicable sections
- Rule 7 (Evidence Classes) — `math, publication` inactive. `evidence_class` in claims.json is always `n/a`.
- Rule 8 (Result-Agnostic Planning) — `math, computational` inactive.
- Rule 10 G-001 — `math, computational` inactive.
- 0.3.3 Mathematical intake, 0.3.4 Computational intake, 0.3.5 Publication intake.
- 1.3 R2b Novelty & prior-art search; `research/NOVELTY.md`.
- 1.4 pinning of CAS / proof assistant / BLAS / TeX.
- 2A Mathematical specification (all of 2A.0–2A.5); `math/` tree.
- 2B.2 Provenance contract; 2B.3 Numerical V&V categories; 2B.4 Unknown-outcome tests.
- 2B.3 Deployment tests (`software.deploys = false`).
- 2C Figure specification; `figures/SPEC.md`.
- 2D Manuscript specification; `manuscript/` tree.
- 3.4 `plan/OPERATIONS.md` (`software.deploys = false`).
- Phase 4 lenses tagged only `[math]`, `[computational]`, `[publication]`.

## Applicable artifact set
HANDOFF.md, plan/{PROFILE,PLAN,ASSUMPTIONS,DECISIONS,GATES,ENVIRONMENT,TRACEABILITY}.md,
research/{QUESTIONS.md,claims.json,SOURCES.md,rounds/,spikes/}, tests/, tests/FROZEN_MANIFEST.sha256,
premortem/{round-N.md,RISK_REGISTER.md}, .checkpoints/state.json.
