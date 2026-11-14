#!/usr/bin/env bash
# Copies the committed axell-core (~/axell-core HEAD) into external/axell-core, so the repo builds alone (CI, Windows).
# CMake prefers external/axell-core when present; rerun after every core change you want in this repo.
set -euo pipefail
cd "$(dirname "$0")/.."
CORE="${AXELL_CORE_SRC:-../axell-core}"
[ -z "$(git -C "$CORE" status --porcelain)" ] || { echo "axell-core has uncommitted changes; commit them first."; exit 1; }
rm -rf external/axell-core; mkdir -p external/axell-core
git -C "$CORE" archive HEAD | tar -x -C external/axell-core
echo "vendored axell-core $(git -C "$CORE" rev-parse --short HEAD)" | tee external/axell-core/VENDORED
