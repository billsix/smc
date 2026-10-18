#!/usr/bin/env bash
#
# measure-const.sh -- size the const-correctness delta for smc/src. Builds with `bear` to capture a
# compile_commands.json, then runs clang-tidy misc-const-correctness (NO --fix) and counts the
# suggested const additions per file. Measurement only; mutates no source. Run in the smc container
# with the repo mounted (make shell-exec SCRIPT=...). Second arg "fix" applies the fixes instead.
set -uo pipefail

mode="${1:-measure}"
root="$(git rev-parse --show-toplevel 2>/dev/null || echo /smc)"
data="$root/tasks/adhoc/smc-const-double-const-sweep/data"
mkdir -p "$data"
cd "$root/smc" || { echo "no smc/ game dir under $root" >&2; exit 1; }
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"

# Fresh configure (default g++) and a bear-wrapped build to capture compile_commands.json.
[ -x configure ] || ./autogen.sh >/dev/null 2>&1 || autoreconf -fi >/dev/null 2>&1
[ -f Makefile ] || ./configure --prefix=/usr/local >/dev/null 2>&1
make clean >/dev/null 2>&1 || true
echo "== generating compile_commands.json via bear"
bear -- make -j"$(nproc)" >/dev/null 2>&1 || { echo "bear/make failed"; exit 1; }
[ -f compile_commands.json ] || { echo "no compile_commands.json produced"; exit 1; }

files=$(find src \( -iname '*.cpp' -o -iname '*.h' \))
if [ "$mode" = fix ]; then
  # Apply SERIALLY, one .cpp at a time, applying each file's fixes before analysing the next.
  # (run-clang-tidy computes every TU's fix against the ORIGINAL sources then stacks them, so a
  # header included by N TUs gets `const` inserted N times -> `const const const ...`. Serial
  # fix-then-next means a shared header is already `const` when the next TU sees it = fixed once.)
  echo "== applying misc-const-correctness --fix serially per .cpp"
  : >"$data/tidy-fix.log"
  for f in $(find src -iname '*.cpp' | sort); do
    echo "--- $f" >>"$data/tidy-fix.log"
    clang-tidy -p . -checks='-*,misc-const-correctness' --fix "$f" >>"$data/tidy-fix.log" 2>&1 || true
  done
  echo "== done; changed files:"; ( cd "$root" && git diff --stat -- smc/src | tail -4 )
  echo "== sanity: any 'const const' doubling? (should be 0):"; ( cd "$root" && grep -rc "const const" smc/src 2>/dev/null | grep -v ":0$" | head )
else
  echo "== counting misc-const-correctness suggestions (no fix)"
  clang-tidy -p . -checks='-*,misc-const-correctness' $files >"$data/tidy-measure.log" 2>&1 || true
  echo "== total suggestions: $(grep -c 'misc-const-correctness' "$data/tidy-measure.log")"
  echo "== by file (top 25):"
  grep 'misc-const-correctness' "$data/tidy-measure.log" | sed -E 's#:[0-9]+:[0-9]+:.*##; s#.*/smc/src/#src/#' | sort | uniq -c | sort -rn | head -25
fi
