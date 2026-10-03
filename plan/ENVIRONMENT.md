# Environment (pinned) — Phase 1.4

All versions below were installed and exercised in the planning sandbox (Ubuntu 24.04.4 LTS, x86-64) on 2026-10-03 unless marked *CI-only*. Third-party source pins (commit SHAs) live in `plan/deps.lock.json`.

## Languages & toolchains
| Item | Version | Where verified |
|---|---|---|
| C++ standard | C++20 | S-01, S-02 |
| GCC (Linux) | 13.3.0 (Ubuntu 24.04 default) | sandbox |
| Clang (Linux, fuzzing only) | 18.1.3 (`apt install clang`) | S-04 |
| MSVC (Windows) | Visual Studio 2026, toolset v145 (windows-2025 runner image) | *CI-only* (C-051, C-058) |
| Apple Clang (macOS) | Xcode default on `macos-26` runner (Xcode 26.6 per runner-images) | *CI-only* (C-051) |
| CMake | 4.4.3 (PyPI) | sandbox |
| Ninja | 1.13.2 (PyPI) | sandbox |
| Python | 3.12.3 (tools/ scripts) | sandbox |

## Libraries (FetchContent, pinned by SHA — see deps.lock.json)
| Name | Tag | Commit |
|---|---|---|
| JUCE | 9.0.3 | be29c81492b6151c8ea8d14c840e1311963b3a83 |
| Catch2 | v3.16.0 | 317ac1ed4c0bb6e6b91eafc817e05c488feffcb3 |
| surge tuning-library | (main) | 48422e2f014fcda8dd5d1a4678bb2674faf3bb3e |

## Tools
| Tool | Version | Notes |
|---|---|---|
| pluginval | v1.0.4 release binaries | Linux zip sha256 `c01c49d8…b55352` (S-02) |
| xvfb | 2:21.1.12-1ubuntu1.8 | Linux headless GUI for pluginval |
| auval / auvaltool | macOS system tool | *CI-only* (C-061) |

## Linux system packages (verified versions)
```
clang=1:18.0-59~exp2  ladspa-sdk=1.17-1build1  libasound2-dev=1.2.11-1ubuntu0.3
libfontconfig1-dev=2.15.0-1.1ubuntu2  libfreetype-dev=2.13.2+dfsg-1ubuntu0.1
libx11-dev=2:1.8.7-1build1  libxcomposite-dev=1:0.4.5-1build3  libxcursor-dev=1:1.2.1-1build1
libxext-dev=2:1.3.4-1build2  libxi-dev=2:1.8.1-1build1  libxinerama-dev=2:1.1.4-3build1
libxrandr-dev=2:1.5.2-2build1  libxrender-dev=1:0.9.10-1.1build1  xvfb=2:21.1.12-1ubuntu1.8
```

## Setup commands (literal; Linux verified in sandbox)
```bash
# Linux (ubuntu-24.04)
sudo apt-get update
sudo apt-get install -y g++ clang ladspa-sdk libasound2-dev libfontconfig1-dev libfreetype-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxi-dev libxinerama-dev \
  libxrandr-dev libxrender-dev xvfb
python3 -m pip install --user "cmake==4.4.3" "ninja==1.13.2"   # add --break-system-packages on PEP 668 systems
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure

# macOS (macos-26, CI-only)
python3 -m pip install "cmake==4.4.3" "ninja==1.13.2"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build -j && ctest --test-dir build --output-on-failure

# Windows (windows-2025, CI-only)
py -m pip install "cmake==4.4.3" "ninja==1.13.2"
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release -j && ctest --test-dir build -C Release --output-on-failure
```

## Determinism settings
- Tests use fixed seeds (`juce::Random` seed 42 or explicit per test), fixed sample rates (44.1/48/96 kHz), fixed block sizes.
- pluginval: `--random-seed 42`.
- Floating point: no `-ffast-math`; flush-to-zero via `juce::ScopedNoDenormals` in render paths (tests run with the same).

## Credentials
| Name | Scope | Needed by | Sandbox target |
|---|---|---|---|
| GITHUB_TOKEN (Actions built-in) | contents:read, actions:write (artifacts) | CI | n/a |
| (none for planning beyond the push token) | | | |
| Apple Developer ID / notarisation | signing for public release only | gated by G-002 | not used before G-002 |
