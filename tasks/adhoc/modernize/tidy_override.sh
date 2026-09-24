#!/usr/bin/env bash
# Step 1 override sweep: clang-tidy modernize-use-override -fix over all of smc/src, using the
# build's real include flags. Validates parsing on one file first (aborts if headers don't resolve).
set -u
cd /smc/smc
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig

FLAGS=(-std=c++23 -DHAVE_CONFIG_H '-DDATA_DIR="smc"' -I. -Isrc)
FLAGS+=($(pkg-config --cflags sdl2 SDL2_image SDL2_mixer SDL2_ttf CEGUI-OPENGL))
echo "FLAGS: ${FLAGS[*]}"

# 1) validate: parse one file, fail loudly if SDL/CEGUI headers don't resolve
echo "=== validation (src/enemies/turtle.cpp): file-not-found check"
if clang-tidy -checks='-*,modernize-use-override' src/enemies/turtle.cpp -- "${FLAGS[@]}" 2>&1 \
     | grep -q "file not found"; then
  echo "ABORT: headers do not resolve, flags are wrong"; exit 3
fi
echo "validation OK (headers resolve)"

# 2) apply the fix across all .cpp (headers scoped to smc/src, so third_party/system stay untouched)
files=$(find src -name '*.cpp' | sort)
echo "=== applying modernize-use-override -fix to $(echo "$files" | wc -l) files ..."
clang-tidy -fix -checks='-*,modernize-use-override' -header-filter='.*/smc/src/.*' \
    $files -- "${FLAGS[@]}" > /smc/smc/../tidy_override_out.txt 2>&1
echo "clang-tidy exit=$?"
echo "=== override annotations applied (approx):"
grep -c "use override" /smc/smc/../tidy_override_out.txt || true
