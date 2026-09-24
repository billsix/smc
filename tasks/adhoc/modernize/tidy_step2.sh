#!/usr/bin/env bash
# Step 2: NULL->nullptr, typedef->using, int-literal->bool-literal, over all of smc/src via
# clang-tidy -fix (same flag setup as the override sweep). #define->constexpr is NOT here
# (clang-tidy can't do it safely) — left for a later manual pass.
set -u
cd /smc/smc
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig

FLAGS=(-std=c++23 -DHAVE_CONFIG_H '-DDATA_DIR="smc"' -I. -Isrc)
FLAGS+=($(pkg-config --cflags sdl2 SDL2_image SDL2_mixer SDL2_ttf CEGUI-OPENGL))

if clang-tidy -checks='-*,modernize-use-nullptr' src/enemies/turtle.cpp -- "${FLAGS[@]}" 2>&1 \
     | grep -q "file not found"; then
  echo "ABORT: headers do not resolve"; exit 3
fi
echo "validation OK"

CHECKS='-*,modernize-use-nullptr,modernize-use-using,modernize-use-bool-literals'
files=$(find src -name '*.cpp' | sort)
echo "=== applying [$CHECKS] to $(echo "$files" | wc -l) files ..."
clang-tidy -fix -checks="$CHECKS" -header-filter='.*/smc/src/.*' \
    $files -- "${FLAGS[@]}" > /smc/tidy_step2_out.txt 2>&1
echo "clang-tidy exit=$?"
