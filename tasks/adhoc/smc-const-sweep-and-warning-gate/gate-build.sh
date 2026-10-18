#!/usr/bin/env bash
#
# gate-build.sh -- regenerate configure from configure.ac, configure, and build smc/src under the
# chosen compiler so the gate flags + -isystem third-party includes defined in configure.ac actually
# take effect (build.sh does NOT regenerate configure from configure.ac). Prints the per-class
# warning breakdown and whether the build linked. The iteration harness for the warning-gate pass.
#
#   gate-build.sh [g++|clang++]   (default g++)
set -uo pipefail

CXX="${1:-g++}"
root="$(git rev-parse --show-toplevel 2>/dev/null || echo /smc)"
data="$root/tasks/adhoc/smc-const-sweep-and-warning-gate/data"
mkdir -p "$data"
cd "$root/smc" || { echo "no smc/ game dir under $root" >&2; exit 1; }

export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
echo "== regenerating configure (autogen) for $CXX"
./autogen.sh >/dev/null 2>&1 || autoreconf -fi >/dev/null 2>&1
./configure --prefix=/usr/local CXX="$CXX" >"$data/configure-${CXX%%+*}.log" 2>&1 || {
    echo "configure FAILED -- tail:"; tail -20 "$data/configure-${CXX%%+*}.log"; exit 1; }

log="$data/gate-${CXX%%+*}.log"
make clean >/dev/null 2>&1 || true
make -k -j"$(nproc)" >"$log" 2>&1
rc=${PIPESTATUS:-0}
echo "== $CXX: $(grep -c 'warning:' "$log") warnings, $(grep -c 'error:' "$log") errors, build exit via make -k"
echo "== by class:"
grep -oE '\[-W[a-z0-9=+-]+\]' "$log" | sort | uniq -c | sort -rn
echo "== first 20 errors (if any):"
grep -E 'error:' "$log" | head -20
echo "== binary built? $( [ -x src/smc ] && echo yes || echo NO )"
