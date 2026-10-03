# Red verification (Phase 2B.5) — 2026-10-03T16:12:59Z

Build: GCC 13.3, CMake 4.4.3, Ninja 1.13.2, Release, JUCE be29c81 (9.0.3), Catch2 317ac1e (v3.16.0). Command: `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build && ctest --test-dir build`

## Result
```
10% tests passed, 38 tests failed out of 42
```

## Failure reasons (all cs::NotImplemented thrown by stubs, or the absent NOTICE file)
```
   1  cs::NotImplemented("BowJunction::reflectionCoefficient")
   1  cs::NotImplemented("DispersionFilter::design")
  13  cs::NotImplemented("Engine::Engine")
   1  cs::NotImplemented("FractionalDelayLine::prepare")
   1  cs::NotImplemented("HuntCrossleyContact")
   1  cs::NotImplemented("LossFilter::design")
   1  cs::NotImplemented("ModalBank::configure")
   1  cs::NotImplemented("TuningTable::equal")
   2  cs::NotImplemented("WaveguideString::prepare")
   1  cs::NotImplemented("decodeState")
   1  cs::NotImplemented("fingeringFor")
   2  cs::NotImplemented("inharmonicityB")
   2  cs::NotImplemented("loadScala")
   3  cs::NotImplemented("paramRange")
   6  cs::NotImplemented("spec")
T-113: NOTICE missing JUCE | NOTICE missing JUCE
```

## Expected passes at red stage (test infrastructure, not requirement tests)
- SUPPORT-1..4 (tests/support_selftest): validate the analysis code the requirement tests depend on. SUPPORT-2 initially FAILED (neighbour-partial leakage in componentT60); fixed by a 3x cascaded moving-average envelope before freeze.
- T-112 (tools/osv_check.py) is an environment scan independent of implementation; in the sandbox it fails cleanly with 'OSV unreachable' (exit 2) because api.osv.dev is outside the sandbox allowlist; runs in CI.

## Scripts and fuzz targets
- tests/ci/pluginval.sh with no plugin -> 'T-110 FAIL: plugin not found' (exit 1).
- fuzz_scala / fuzz_state build with clang 18 (-fsanitize=fuzzer,address,undefined) and abort on cs::NotImplemented at the first input.
- tests/ci/auval.sh: macOS only (not runnable in sandbox); fails with 'component not found' when the path is absent (script logic identical).
