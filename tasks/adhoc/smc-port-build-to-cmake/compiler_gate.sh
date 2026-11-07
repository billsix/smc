#!/bin/bash
# smc's -Werror warning gate for one compiler (CXX, default clang++; gcc via CXX=g++).
# Builds a scratch copy of the mounted smc/ tree with clang++ against the image's
# CEGUI, never touching the mounted tree (which holds the host's in-tree build).
#
#   [HOST] make shell-exec SCRIPT=tasks/adhoc/smc-port-build-to-cmake/compiler_gate.sh
#
# Exit status is the build's: 0 = clang -Werror clean.
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
src="$here/../../../smc"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
cp -r "$src" "$work/smc"
cd "$work/smc"
# The host's in-tree build leaves objects that would look up to date and a
# generated configure that may predate configure.ac; drop both and regenerate.
find . \( -name '*.o' -o -name '.deps' -o -name 'config.status' -o -name 'config.log' -o -name 'Makefile' -o -name 'autom4te.cache' \) -prune -exec rm -rf {} +
rm -f configure
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
[ -x configure ] || ./autogen.sh
CXX="${CXX:-clang++}" ./configure --prefix="$work/install" >/dev/null || { tail -25 config.log; exit 1; }
make -j"$(nproc)" >"$work/build.log" 2>&1 || { grep -E 'error|warning' "$work/build.log" | head -40; exit 1; }
echo "${CXX:-clang++} -Werror build: OK ($(grep -c "warning:" "$work/build.log" || true) warnings)"
