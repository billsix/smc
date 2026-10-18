#!/usr/bin/env bash
#
# measure.sh -- size the FIRST-PARTY -Wsign-conversion delta (CEGUI/SDL suppressed via the -isystem
# in configure.ac's CPPFLAGS) and bucket the hits by from->to type pair, so a policy can be chosen
# per boundary. Measurement only. Run in the smc container with the repo mounted.
set -uo pipefail

CXX="${1:-g++}"
root="$(git rev-parse --show-toplevel 2>/dev/null || echo /smc)"
data="$root/tasks/adhoc/smc-sign-conversion-sweep/data"
mkdir -p "$data"
cd "$root/smc" || { echo "no smc/ game dir under $root" >&2; exit 1; }
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"

# Configure so CPPFLAGS carries the -isystem for SDL/CEGUI; then build adding ONLY -Wsign-conversion
# on top (override CXXFLAGS but leave the configured CPPFLAGS -isystem in place). Keep-going.
[ -x configure ] || ./autogen.sh >/dev/null 2>&1 || autoreconf -fi >/dev/null 2>&1
./configure --prefix=/usr/local CXX="$CXX" >/dev/null 2>&1
make clean >/dev/null 2>&1 || true
log="$data/signconv-${CXX%%+*}.log"
make -k -j"$(nproc)" CXXFLAGS="-std=c++23 -O2 -Wsign-conversion" >"$log" 2>&1

echo "== $CXX first-party -Wsign-conversion warnings: $(grep -c 'sign-conversion' "$log")"
echo "== distinct source lines (dedup headers): $(grep 'sign-conversion' "$log" | grep -oE '[^ ]+\.(cpp|h):[0-9]+' | sed -E 's#.*/smc/src/##' | sort -u | wc -l)"
echo "== by directory:"
grep 'sign-conversion' "$log" | grep -oE '/smc/src/[a-z]+/' | sort | uniq -c | sort -rn
echo "== from->to type pairs (top 20):"
grep -oE "conversion (to|from) .[a-zA-Z0-9_: ]+. (from|to) .[a-zA-Z0-9_: ]+." "$log" | sort | uniq -c | sort -rn | head -20
