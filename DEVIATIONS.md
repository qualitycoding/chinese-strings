# Deviations

| Date | Step | What | Why |
|---|---|---|---|
| 2026-10-03 | S-005 | `DispersionFilter` uses second-order allpass sections (two sections for `numSections = 4`, coefficients reported as lattice reflection coefficients, all |k| < 1) instead of N identical first-order sections. | Spike on the planned design: 4 first-order sections leave ~9.8 cents error on yehu partial 5 (frozen T-021 requires ≤ 3 cents); first-order chains up to N=16 still miss (3.6 cents); 2 second-order sections fitted by least squares reach 0.26 cents. Reversible, DSP-internal, no test or interface change. |
| 2026-10-03 | S-005 | Private members/ctor/dtor of the classes in `core/include/cs/Dsp.h` replaced by pimpl state. | Implementation detail; no public signature changed (D-020). |
| 2026-10-03 | S-005 | Pluck excitation = comb-filtered impulse pair injected into the loop (no spring-finger junction constant k). | Open-loop input injection keeps the loop passive by construction; the spring junction adds nothing testable. |
