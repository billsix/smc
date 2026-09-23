#!/usr/bin/env bash
# format.sh -- clang-format over the game's C++ (opt-in `make format`; NEVER run on shell exit:
# this is upstream's hand style, and reformatting would drown the minimal-change diffs).
# Portable: relative paths from the repo root, runs in the container or on the host.
set -u
[ -d /smc ] && cd /smc
status=0
find smc/src -type f \( -name '*.cpp' -o -name '*.h' \) -print0 | xargs -0 clang-format -i || status=1
exit $status
