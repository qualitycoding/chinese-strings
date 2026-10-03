#!/usr/bin/env bash
# FROZEN — DO NOT MODIFY. T-111: AU validation on macOS (D-012b). Usage: tests/ci/auval.sh <path-to.component>
set -euo pipefail
COMP="$1"; [ -d "$COMP" ] || { echo "T-111 FAIL: component not found: $COMP"; exit 1; }
mkdir -p ~/Library/Audio/Plug-Ins/Components && rm -rf ~/Library/Audio/Plug-Ins/Components/"$(basename "$COMP")"
cp -R "$COMP" ~/Library/Audio/Plug-Ins/Components/
killall -9 AudioComponentRegistrar || true; sleep 2
if ! auval -strict -v aumu Cs01 Qcod; then
  codesign --force -s - ~/Library/Audio/Plug-Ins/Components/"$(basename "$COMP")"; killall -9 AudioComponentRegistrar || true; sleep 2
  auval -strict -v aumu Cs01 Qcod
fi
echo "T-111 PASS"
