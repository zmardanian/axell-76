#!/usr/bin/env bash
# Portable Linux build: compiled on Ubuntu 20.04 (glibc 2.31) with the C++ runtime linked in, so the plugin and the
# standalone app run on practically every x86_64 Linux from 2020 on. Box: distrobox `a76-portable` (ubuntu:20.04).
set -euo pipefail
cd "$(dirname "$0")/.."
DIR="$PWD"
distrobox enter a76-portable -- bash -lc "cd '$DIR' && ~/.local/lib/python3.8/site-packages/cmake/data/bin/cmake -B build-portable -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/bin/gcc-11 -DCMAKE_CXX_COMPILER=/usr/bin/g++-11 \
  -DCMAKE_SHARED_LINKER_FLAGS='-static-libstdc++ -static-libgcc' -DCMAKE_EXE_LINKER_FLAGS='-static-libstdc++ -static-libgcc' -DCMAKE_MODULE_LINKER_FLAGS='-static-libstdc++ -static-libgcc' \
  && ~/.local/lib/python3.8/site-packages/cmake/data/bin/cmake --build build-portable -j\${JOBS:-12} --target Axell76_VST3 Axell76_LV2 Axell76_Standalone"
echo "Built: $DIR/build-portable/Axell76_artefacts/Release"
