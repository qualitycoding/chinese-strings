# Cold-read gate (§3.6) log

Executed by Opus 5.5 in the same context with role separation (A-016 tier substitution): a dry-run reader that only reads branch files, then a mechanical checker (`tools/plan_lint.py`, Haiku role).

## Pass 1 (dry run) — 9 items found, all fixed by amending the plan
1. S-005 loss-filter design (one-pole) cannot meet frozen T-010 → spike S-08/S-08b → FIR design (C-068); S-005 rewritten.
2. S-003: gain of S-07 bridge modes undefined → INSTRUMENT_DATA rule 4 defines gain_m = (1/M_m)/max(1/M).
3. Notes outside an instrument's range: behaviour undefined but required by T-034 → S-007 action 3.
4. CC1 default could zero the vibrato required by T-036 → S-007 defines CC1 default 127.
5. S-008 Done-when listed T-036/T-037, implemented only in S-009 → removed.
6. S-012 Done-when label set included plugin tests T-09x built in S-013 → excluded with `-E "^T-09"`.
7. S-007 Done-when could not pass for families implemented later → placeholder voice (S-007 5a); later steps run regression ranges.
8. DR-10 covered only data files, but S-008/S-012 name DD code constants → DR-10 widened.
9. A-011/A-017 referred to the listening gates without their a..d suffix and to a Reaper project at listening gates → clarified to G-003a..d WAV bundles, Reaper at G-004.

`tools/plan_lint.py` after pass 1: `plan_lint: 0 problems`.

## Pass 2 (fresh dry run of the amended plan)
Re-read HANDOFF.md → PLAN.md S-001..S-017 with the question "could I execute this without asking?". Items found: 0.
Mechanical: `plan_lint: 0 problems`.
