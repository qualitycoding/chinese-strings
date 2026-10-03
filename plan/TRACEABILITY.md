# Traceability (Phase 2E)

## Success criteria (from A-004..A-013, D-003)
| SC | Criterion | Source | Evidence (tests) | Steps |
|---|---|---|---|---|
| SC-01 | All 19 retained instruments are selectable, produce finite, audible output, and use their family's excitation (bowed sustain, others decay) | A-002, D-003, R-006 | T-001, T-030, T-045 | S-003, S-007..S-012 |
| SC-02 | Pitch accuracy ±3 cents (open strings, all fs 22.05–192 kHz, stiffness compensated); data physically consistent and equal to cited anchors | A-011, C-030..C-037 | T-002, T-003, T-012, T-020, T-031, T-032 | S-003..S-012 |
| SC-03 | Frequency-dependent string decay follows the cited loss law (decay oracle) | A-011, C-063 | T-010, T-022 | S-005 |
| SC-04 | Body/bridge modal sets reproduce configured frequencies (0.5 %) and T60 (10 %) | D-004, D-019 | T-015 | S-004 |
| SC-05 | Stiff-string inharmonicity reproduced (partials within 3 cents) | C-037 | T-004, T-021 | S-003, S-005 |
| SC-06 | Numerical stability & passivity; no NaN/Inf; bounded output | D-004a, C-062 | T-010, T-011, T-013, T-014, T-023, T-030, T-033 | S-004..S-008 |
| SC-07 | Realistic polyphony, free-polyphony override (16), bowed legato | A-010, D-022 | T-034 | S-007 |
| SC-08 | Techniques via keyswitch/CC; vibrato; pitch bend; sample-accurate MIDI | A-007, D-017 | T-035..T-037, T-039 | S-008, S-009 |
| SC-09 | MPE per-note expression (in-process; VST3 host-dependent per D-015) | A-007, D-015 | T-038 | S-009; G-004 |
| SC-10 | Tuning: equal temperament, Scala import with validation and limits | A-009, D-010 | T-040, T-050..T-052, T-100 | S-006, S-009 |
| SC-11 | Plugin state round-trip, validation, clamping | D-016, A-013 | T-005, T-090..T-092, T-101 | S-007, S-013 |
| SC-12 | Real-time safety: zero allocations on the audio path | A-012, D-009 | T-070 | S-007..S-012 |
| SC-13 | Performance: 16 voices of heaviest instrument (held and decaying tails), normalised load ≤ 0.45 | A-012, D-008, R-007 | T-080, T-081 | S-015 |
| SC-14 | Plugin conformance: pluginval strictness 10 (all OS), auval (macOS) | A-005, D-006, D-012 | T-110, T-111 | S-014, S-016 |
| SC-15 | Dependency vulnerability scan clean; licence/NOTICE correct | A-013, D-002, D-014 | T-112, T-113 | S-002, S-016 |
| SC-16 | Operational robustness: config errors, fs/block range, resume after re-prepare, shutdown | 0.3.2 / 2B.3 | T-032, T-041, T-043 | S-007 |
| SC-17 | UI highlights string & stop position for played notes | A-008, D-013, D-021 | T-042 (+ human check at G-004) | S-007, S-014 |
| SC-19 | Thread safety: instrument switching, parameter writes and UI snapshots concurrent with rendering are race-free (TSAN-clean) | R-005, R-008, D-024 | T-044 | S-007, S-014, S-016 |
| SC-18 | Human listening sign-off per family | A-011 | G-003a..d, G-004 (no automated test) | S-008, S-010, S-011, S-012, S-016 |

## Test → requirement map
Every test docstring/TEST_CASE tag lists its SC-, C-, D- or A- IDs. Infrastructure checks SUPPORT-1..4 map to no SC (they validate tests/support).
| Test | SC |
|---|---|
| T-001 | SC-01 | 
| T-002, T-003 | SC-02 |
| T-004 | SC-05 |
| T-005 | SC-11 |
| T-010 | SC-03, SC-06 |
| T-011, T-013, T-014 | SC-06 |
| T-012 | SC-02 |
| T-015 | SC-04 |
| T-020 | SC-02 |
| T-021 | SC-05 |
| T-022 | SC-03 |
| T-023 | SC-06 |
| T-030 | SC-01, SC-06 |
| T-031, T-032 | SC-02 (T-032 also SC-16) |
| T-033 | SC-06 |
| T-034 | SC-07 |
| T-035..T-037, T-039 | SC-08 |
| T-038 | SC-09 |
| T-040, T-050..T-052, T-100 | SC-10 |
| T-041, T-043 | SC-16 |
| T-042 | SC-17 |
| T-070 | SC-12 |
| T-080, T-081 | SC-13 |
| T-044 | SC-19 |
| T-045 | SC-01 |
| T-090..T-092, T-101 | SC-11 |
| T-110, T-111 | SC-14 |
| T-112, T-113 | SC-15 |
