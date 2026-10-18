#!/usr/bin/env bash
#
# detail.sh -- one clean gcc build of smc/src with the DECIDED gate flags (fleet set minus
# -Wsign-conversion, plus -Wno-unused-parameter; NO -Werror), dumping the full warning log to
# tasks/adhoc/smc-const-sweep-and-warning-gate/data/ (a committed snapshot worklog) and printing
# the per-class breakdown + the distinct classes behind -Wdeprecated-copy / -Woverloaded-virtual
# so the fixes can be planned centrally. Measurement only.
set -uo pipefail

CXX="${1:-g++}"
root="$(git rev-parse --show-toplevel 2>/dev/null || echo /smc)"
data="$root/tasks/adhoc/smc-const-sweep-and-warning-gate/data"
mkdir -p "$data"
cd "$root/smc" || { echo "no smc/ game dir under $root" >&2; exit 1; }

flags="-std=c++23 -O2 -g -Wall -Wextra -Wpedantic -Wshadow -Wvla -Wformat=2 -Wno-unused-parameter"
case "$CXX" in clang*) flags="$flags -Wshorten-64-to-32 -Wimplicit-int-conversion" ;; esac

log="$data/warnings-${CXX%%+*}.log"
echo "== $CXX  flags: $flags"
make clean >/dev/null 2>&1 || true
make -k -j"$(nproc)" CXX="$CXX" CXXFLAGS="$flags" >"$log" 2>&1
echo "== log: $log  ($(wc -l <"$log") lines, $(grep -c 'warning:' "$log") warnings, $(grep -c 'error:' "$log") errors)"
echo "== by class:"
grep -oE '\[-W[a-z0-9=+-]+\]' "$log" | sort | uniq -c | sort -rn
echo
echo "== distinct classes behind -Wdeprecated-copy (from the 'has user-provided'/'user-declared' notes):"
grep -E "deprecated because .* has a user" "$log" | grep -oE "'[A-Za-z_:<>]+'" | sort -u | head -60
echo
echo "== -Woverloaded-virtual sites (file:line):"
grep -B0 "overloaded-virtual" "$log" | grep -oE "^[^ ]+\.(h|cpp|hpp):[0-9]+" | sort -u | head -60
