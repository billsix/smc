# Reformat smc/src for clang-format 23 (one tree-wide commit + blame-ignore)

**Status:** proposed — needs go-ahead (found 2026-11-07 during the manager refactor)
**Priority:** 5
**Difficulty:** 2

## BLUF

`smc/src` is meant to be clang-format clean (`smc/.clang-format`, google style; `make format`), but the
Fedora 45 image's **clang-format 23.1.4** now disagrees with **72 of 162** source files (~4,200 lines
of diff, measured 2026-11-07; the largest: `gui/menu_data.cpp` 272, `video/animation.cpp` 249,
`video/video.cpp` 150). The tree was last formatted on 2026-09-23 (`4e461d2`, the reformat commit in
`.git-blame-ignore-revs`), under an older clang-format. Until it is reformatted, `make format` or a
whole-file `clang-format -i` on any of those files buries a real change in formatting churn. Done = one
layout-only commit that runs `make format` over the tree, its SHA added to `.git-blame-ignore-revs`, and
the gcc/clang gates + smoke still green.

## Context

- How it showed up: formatting the four files of the `cBackground_Manager` change with `clang-format -i`
  produced a ~290-line diff in `menu_data.cpp` alone. That commit instead formatted only its changed
  lines (`git diff -U0 | clang-format-diff -p1 -i`). Until this task lands, use the same for any edit.
- Measure again with: for each `git ls-files 'src/*.cpp' 'src/*.h'` (run in `smc/`), diff the file
  against `clang-format --style=file <file>` and count.
- The reformat commit goes in `.git-blame-ignore-revs` like `bec5840` (see `CLAUDE.md`).

## Plan

1. `make format` (or `clang-format -i` over `smc/src/**/*.{cpp,h}` in the image), nothing else in the
   commit; message says layout-only and names the clang-format version.
2. Gates: gcc + clang `-Werror` (`tasks/adhoc/smc-port-build-to-cmake/compiler_gate.sh`) and the smoke.
   `-Werror` should not move; check that nothing inside a `#pragma` suppression block shifted out of it.
3. Add the commit's SHA to `.git-blame-ignore-revs` in a second commit.
