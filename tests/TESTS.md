# Frozen test catalogue

| Category (protocol 2B.3) | Tests |
|---|---|
| Unit | T-001..T-005, T-010..T-015, T-042, T-050..T-052 |
| Integration | T-030..T-040, T-090 |
| Acoustic (verification vs cited parameters) | T-020..T-023 |
| Operational (startup, config errors, resource exhaustion, resume, shutdown) | T-032, T-034 (voice exhaustion), T-041, T-043, T-091 (oversize input) |
| Security (input validation, fuzzing, dependency scan) | T-052, T-091, T-092, T-100 (fuzz_scala, 300 s), T-101 (fuzz_state, 300 s), T-112 |
| Performance | T-080 (Release only, label `perf`) |
| Real-time safety | T-070 (separate executable `cs_rt_tests`) |
| Conformance (CI scripts) | T-110 (tests/ci/pluginval.sh), T-111 (tests/ci/auval.sh), T-113 (tools/check_notice.py) |
| Infrastructure self-test (not requirement tests; must always pass) | SUPPORT-1..SUPPORT-4 |

Deployment tests: N/A (`software.deploys = false`, PROFILE.md).

Fuzz commands (Linux, clang 18):
```
CC=clang CXX=clang++ cmake -S . -B build-fuzz -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCS_FUZZ=ON
cmake --build build-fuzz --target fuzz_scala fuzz_state
build-fuzz/tests/fuzz_scala -max_total_time=300 -seed=1 tests/fuzz/corpus_scala   # T-100: exit 0, no crash file
build-fuzz/tests/fuzz_state -max_total_time=300 -seed=1 tests/fuzz/corpus_state   # T-101
```
