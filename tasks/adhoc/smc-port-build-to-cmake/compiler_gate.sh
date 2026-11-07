#!/bin/bash
# smc's -Werror warning gate for one compiler (CXX, default clang++; gcc via CXX=g++).
# A throwaway out-of-tree CMake build of the mounted smc/ sources against the
# image's CEGUI; nothing is written into the mounted tree.
#
#   [HOST] podman run --rm --cgroups=disabled --entrypoint /bin/bash -v "$PWD":/smc/:Z smc \
#            -c 'cd /smc && CXX=g++ bash tasks/adhoc/smc-port-build-to-cmake/compiler_gate.sh'
#
# Exit status is the build's: 0 = -Werror clean under that compiler.
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
src="$here/../../../smc"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
cxx="${CXX:-clang++}"
cmake -S "$src" -B "$work/build" -G Ninja -DCMAKE_CXX_COMPILER="$cxx" >/dev/null
cmake --build "$work/build" >"$work/build.log" 2>&1 || { grep -E 'error|warning' "$work/build.log" | head -40; exit 1; }
echo "$cxx -Werror build: OK ($(grep -c "warning:" "$work/build.log" || true) warnings)"
