#!/bin/bash
# Collect every XML attribute value in the shipped game data (levels, worlds,
# campaigns, menus, ...) into data/values.txt: the corpus the number-helper
# oracle replays through the old and new string<->number helpers.
set -euo pipefail
cd "$(dirname "$0")"
root=$(git rev-parse --show-toplevel)
rg --no-filename -o 'value="[^"]*"' "$root/smc/data" \
    | sed -E 's/^value="(.*)"$/\1/' | sort -u > data/values.txt
wc -l data/values.txt
