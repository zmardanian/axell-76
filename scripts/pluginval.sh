#!/usr/bin/env bash
# Downloads pluginval (once) and validates the built plugin at strictness 10.
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p tools
if [ ! -x tools/pluginval ] && [ -x ../axell-gate/tools/pluginval ]; then cp ../axell-gate/tools/pluginval tools/; fi
if [ ! -x tools/pluginval ]; then
  curl -fL -o tools/pluginval.zip https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Linux.zip
  (cd tools && unzip -o pluginval.zip && chmod +x pluginval)
fi
BUNDLE="build/Axell76_artefacts/Release/VST3/Axell 76.vst3"
# Steinberg's VST3 SDK validator too, when it was built into tools/ (same wrapper as Axell Penta EQ)
VAL=()
[ -x ../axell-gate/tools/vst3-validator ] && [ ! -x tools/vst3-validator ] && cp ../axell-gate/tools/vst3-validator tools/ && cp ../axell-gate/scripts/vst3-validator-wrap.sh scripts/
[ -x tools/vst3-validator ] && VAL=(--vst3validator "$PWD/scripts/vst3-validator-wrap.sh")
./tools/pluginval --strictness-level 10 --validate-in-process "${VAL[@]}" --output-dir tools/pluginval-logs --validate "$BUNDLE" "$@"
