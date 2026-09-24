#!/usr/bin/env bash
# Step 8 (loops): explicit begin/end iterator loops -> range-for via clang-tidy
# modernize-loop-convert -fix, over smc/src. Same flag setup as tidy_step2/tidy_step4.
# clang-tidy skips any loop it cannot prove safe (erase-in-loop, index still used, etc.),
# leaving those for a later hand pass. The generated loop-variable names that come out
# m_-prefixed (container m_foos -> local m_foo) are fixed BY HAND after this runs -- this
# script only does the mechanical conversion.
set -u
cd /smc/smc
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig

FLAGS=(-std=c++23 -DHAVE_CONFIG_H '-DDATA_DIR="smc"' -I. -Isrc)
FLAGS+=($(pkg-config --cflags sdl2 SDL2_image SDL2_mixer SDL2_ttf CEGUI-OPENGL))

if clang-tidy -checks='-*,modernize-loop-convert' src/enemies/turtle.cpp -- "${FLAGS[@]}" 2>&1 \
     | grep -q "file not found"; then
  echo "ABORT: headers do not resolve"; exit 3
fi
echo "validation OK"

CHECKS='-*,modernize-loop-convert'
files=$(find src -name '*.cpp' | sort)
echo "=== applying [$CHECKS] to $(echo "$files" | wc -l) files ..."
clang-tidy -fix -checks="$CHECKS" -header-filter='.*/smc/src/.*' \
    $files -- "${FLAGS[@]}" > /smc/tasks/adhoc/modernize/data/loops_out.txt 2>&1
echo "clang-tidy exit=$?"
echo "=== conversions:"
grep -c '\[modernize-loop-convert\]' /smc/tasks/adhoc/modernize/data/loops_out.txt || true
