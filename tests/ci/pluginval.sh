#!/usr/bin/env bash
# FROZEN — DO NOT MODIFY. T-110: pluginval v1.0.4 strictness 10 on the built VST3 (D-006). Usage: tests/ci/pluginval.sh <path-to-pluginval> <path-to.vst3>
set -euo pipefail
PV="$1"; PLUG="$2"
[ -e "$PLUG" ] || { echo "T-110 FAIL: plugin not found: $PLUG"; exit 1; }
"$PV" --version | grep -q "v1.0.4" || { echo "T-110 FAIL: pluginval is not v1.0.4"; exit 1; }
if [ "$(uname)" = "Linux" ]; then RUN="xvfb-run -a"; else RUN=""; fi
$RUN "$PV" --strictness-level 10 --random-seed 42 --timeout-ms 600000 --validate "$PLUG"
echo "T-110 PASS"
