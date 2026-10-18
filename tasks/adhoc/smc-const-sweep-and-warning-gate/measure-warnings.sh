#!/usr/bin/env bash
#
# measure-warnings.sh -- size the Phase-2 warning-gate worklist for smc's first-party game.
# Clean-rebuilds smc/src with the raised fleet C++ warning set (NO -Werror) under the chosen
# compiler and prints the diagnostics bucketed by warning class, so the triage can be planned
# per class. Advisory/measurement only -- mutates nothing in the tree.
#
#   measure-warnings.sh [g++|clang++]   (default g++)
#
# Run in the smc container with the repo mounted: it is invoked from the repo root
# (`make shell-exec SCRIPT=tasks/adhoc/smc-const-sweep-and-warning-gate/measure-warnings.sh`),
# but also finds the root itself so a plain `bash <script>` works.
set -uo pipefail

CXX="${1:-g++}"
root="$(git rev-parse --show-toplevel 2>/dev/null || echo /smc)"
cd "$root/smc" || { echo "no smc/ game dir under $root" >&2; exit 1; }

# Fleet raised set (gltron's, adapted to C++): deliberately NOT -Wconversion / -Wold-style-cast.
flags="-std=c++23 -O2 -g -Wall -Wextra -Wpedantic -Wshadow -Wvla -Wformat=2 -Wsign-conversion"
case "$CXX" in
  clang*) flags="$flags -Wshorten-64-to-32 -Wimplicit-int-conversion" ;;
esac

echo "== compiler: $CXX"
echo "== flags: $flags"
make clean >/dev/null 2>&1 || true
log="$(mktemp)"
make -k -j"$(nproc)" CXX="$CXX" CXXFLAGS="$flags" >"$log" 2>&1

echo "== total warning lines: $(grep -c 'warning:' "$log")"
echo "== total error lines:   $(grep -c 'error:' "$log")"
echo "== by warning class:"
grep -oE '\[-W[a-z0-9=+-]+\]' "$log" | sort | uniq -c | sort -rn
echo "== any hard errors (first 20):"
grep 'error:' "$log" | head -20
rm -f "$log"
