#!/usr/bin/env bash
# Step 4: the remaining WART-FREE, BEHAVIOR-PRESERVING clang-tidy modernize checks, applied over
# smc/src via -fix (same flag setup as steps 1-2).
#
# Deliberately NOT here:
#  - modernize-loop-convert: correct, but it names each range-for local after its container, so a
#    member container m_foos yields an m_foo-prefixed LOCAL (breaks the m_ = member convention, 32
#    such names). Picking good loop-var names is judgment -> moved to the reviewed loops/strings
#    step (umbrella step 8), not this autonomous one.
#  - style/judgment or ownership-changing (maintainer's call): use-auto, use-default-member-init,
#    redundant-void-arg, make-unique/shared, use-nodiscard, avoid-c-arrays.
set -u
cd /smc/smc
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig

FLAGS=(-std=c++23 -DHAVE_CONFIG_H '-DDATA_DIR="smc"' -I. -Isrc)
FLAGS+=($(pkg-config --cflags sdl2 SDL2_image SDL2_mixer SDL2_ttf CEGUI-OPENGL))

# Validate the headers resolve before touching anything (same guard as step 2).
if clang-tidy -checks='-*,modernize-use-emplace' src/enemies/turtle.cpp -- "${FLAGS[@]}" 2>&1 \
     | grep -q "file not found"; then
  echo "ABORT: headers do not resolve"; exit 3
fi
echo "validation OK"

CHECKS='-*,modernize-use-emplace,modernize-use-equals-default,modernize-use-equals-delete,modernize-deprecated-headers'
files=$(find src -name '*.cpp' | sort)
echo "=== applying [$CHECKS] to $(echo "$files" | wc -l) files ..."
clang-tidy -fix -checks="$CHECKS" -header-filter='.*/smc/src/.*' \
    $files -- "${FLAGS[@]}" > /smc/tasks/adhoc/modernize/data/step4_out.txt 2>&1
echo "clang-tidy exit=$?"
echo "=== applied fixes (by check):"
grep -oE '\[modernize-[a-z-]+\]' /smc/tasks/adhoc/modernize/data/step4_out.txt | sort | uniq -c || true

# use-equals-default is safe for destructors and default ctors, but on this old code it BOTCHES
# GL_point's copy ctor (emits "( ... ) : , = default;", won't compile) and rewrites three
# hand-written copy-assignment operators (GL_rect/GL_Vector/Color) to =default -- a copy/assign
# SEMANTICS change, which belongs in the maintainer-reviewed pass, not this mechanical one. Revert
# exactly those four files so this script's output = keep-the-safe-bulk. Keeps the script idempotent
# to HEAD (after this step lands, those files already match HEAD, so the checkout is a no-op).
git config --global --add safe.directory /smc 2>/dev/null || true
git -C /smc checkout HEAD -- \
    smc/src/core/math/point.h smc/src/core/math/rect.h \
    smc/src/core/math/vector.h smc/src/video/color.h
echo "=== reverted 4 hazardous copy/assign =default conversions (see comment above)"
