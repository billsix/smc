# Reformat smc/src for clang-format 23 (done 2026-11-07)

The Fedora 45 image's **clang-format 23.1.4** disagreed with **72 of 162** files in `smc/src`
(about 4,200 diff lines). The tree had last been formatted on 2026-09-23 (`bec5840`) under an older
clang-format. The disagreement showed up during the `cBackground_Manager` change, where a whole-file
`clang-format -i` of `gui/menu_data.cpp` alone produced a ~290-line diff. That change was formatted
with `git diff -U0 | clang-format-diff -p1 -i` instead.

The maintainer approved the reformat (William Emerison Six <billsix@gmail.com>, 2026-11-07).
`entrypoint/format.sh` (`make format`) ran over the tree in the image. The result was one layout-only
commit, `cfbc1d5` ("reformat smc/src with clang-format 23.1.4"), with 72 files, +2103/−2120 lines.

- A second `format.sh` run produced a byte-identical diff, so the formatter is idempotent.
- gcc and clang `-Werror` builds were clean with 0 warnings
  (`tasks/adhoc/smc-port-build-to-cmake/compiler_gate.sh`). Every `#pragma` suppression still
  compiled silent.
- `build.sh game` from the mount, followed by `tools/smoke_headless.sh`, passed (exit 137 at the
  deadline; `ps` audit clean).
- `cfbc1d5` was added to `.git-blame-ignore-revs` beside `bec5840`.

Whole-file `make format` / `clang-format -i` is safe again for any edit.
