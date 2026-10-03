# Deviations

| Date | Step | What | Why |
|---|---|---|---|
| 2026-10-03 | S-005 | `DispersionFilter` uses second-order allpass sections (two sections for `numSections = 4`, coefficients reported as lattice reflection coefficients, all |k| < 1) instead of N identical first-order sections. | Spike on the planned design: 4 first-order sections leave ~9.8 cents error on yehu partial 5 (frozen T-021 requires ≤ 3 cents); first-order chains up to N=16 still miss (3.6 cents); 2 second-order sections fitted by least squares reach 0.26 cents. Reversible, DSP-internal, no test or interface change. |
| 2026-10-03 | S-005 | Private members/ctor/dtor of the classes in `core/include/cs/Dsp.h` replaced by pimpl state. | Implementation detail; no public signature changed (D-020). |
| 2026-10-03 | S-005 | Pluck excitation = comb-filtered impulse pair injected into the loop (no spring-finger junction constant k). | Open-loop input injection keeps the loop passive by construction; the spring junction adds nothing testable. |
| 2026-10-03 | S-006 | KBM mapping fields are range-checked before the library builds the tuning (`core/src/Tuning.cpp`). | T-100 fuzzing found signed-integer overflow inside tuning-library (KBM key index → scale rotation count). Regression input kept in `tests/fuzz/regressions/`; rerun of T-100: 2,768,345 runs / 301 s, clean. |
| 2026-10-03 | S-007 | Plucked voice adds a direct pick-attack transient (0.35 × low-passed excitation) to the loop output. | T-039 (sample-accurate onset within 64 samples) cannot be met by loop output alone for low strings (first bridge arrival is one period later). DD value. |
| 2026-10-03 | S-007 | `designsFor` caches per-note string designs per (instrument, fs) behind a mutex. | Keeps `setInstrument` / `EngineHost::requestInstrument` cheap (T-031 builds ~190 engines, T-044 requests 200 instruments). Control-thread only. |
