# Modernize step 4 — safe clang-tidy modernize sweep (loops, emplace, = default/delete, C headers)

**Status:** DONE 2026-09-24 — branch `modernize-cpp`, William Emerison Six <billsix@gmail.com>
**Priority:** 4 · **Difficulty:** 3
**Part of:** `smc-modernize-cpp.md` (umbrella)
**Depends on:** steps 0–3 (done); `make image` + `tools/smoke_headless.sh` is the regression gate.

## BLUF

Run the remaining **wart-free, behavior-preserving** clang-tidy `modernize-*` checks that the
compiler enforces — `modernize-use-emplace`, `modernize-use-equals-default`,
`modernize-use-equals-delete`, `modernize-deprecated-headers` — across `smc/src` with `-fix`,
exactly the mechanism steps 1–2 used. Done = the tree compiles clean under C++23, links, and
passes the headless smoke run. This is the last purely-mechanical modernization step before the
architectural rewrites (ownership, application object, dispatch) that need the maintainer's
gameplay review.

**Excluded `modernize-loop-convert` after trying it:** it is correct but names each range-for
local after its container, so a member container `m_foos` yields an `m_foo`-prefixed *local* (32
such names) — breaking the `m_` = member convention. Choosing good loop-variable names is a
judgment call, so range-for conversion moved to the reviewed loops/strings step (umbrella step 8).

## Context

- Same tooling as steps 1–2: clang-tidy 22, no compile DB, so per-file flags after `--`
  (`-std=c++23 -DHAVE_CONFIG_H -DDATA_DIR="smc" -I. -Isrc` + pkg-config for SDL2/CEGUI-OPENGL),
  `PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig`,
  `header-filter='.*/smc/src/.*'` so third_party is never touched. Run inside the container
  (`make shell-exec SCRIPT=tasks/adhoc/modernize/tidy_step4.sh`).
- **Checks deliberately EXCLUDED** (judgment/style, not autonomously-safe, or ownership-changing):
  `modernize-use-auto` and `modernize-use-default-member-init` (style the maintainer should opt
  into, large churn), `modernize-redundant-void-arg` (the code deliberately writes `(void)`;
  cosmetic, touches nearly every function), `modernize-make-unique`/`make-shared` and anything
  touching ownership (that is the reviewed step 5), `modernize-use-nodiscard`
  (can introduce new warnings), `modernize-avoid-c-arrays`/`pass-by-value` (too invasive).
- Why this is safe to do autonomously while steps 5+ are not: each of these transforms is
  behavior-preserving and the compiler re-checks the result — a broken conversion does not
  compile. The ownership/dispatch/file-format steps can compile and smoke-pass while being
  gameplay- or save-format-wrong, so they need the maintainer's play-test.

## Plan

- [x] Write `tasks/adhoc/modernize/tidy_step4.sh` (validate headers resolve, then `-fix`).
- [x] Apply; the verbose clang-tidy log goes to `tasks/adhoc/modernize/data/step4_out.txt`
  (gitignored — 300K+ of duplicated header diagnostics; counts recorded here).
- [x] Compiles + links clean under C++23 (`make image` builds the game; make aborts on any error).
- [x] `make format` (the committed `.clang-format`), confirmed idempotent (identical re-run).
- [x] `make image` + `tools/smoke_headless.sh` green (game alive at 30 s = exit 137, ps clean).
- [ ] Commit; archive this step-task in its own commit.

## Notes / decisions

- **Result:** 34 files, +354 / −385 (net −31). Two checks fired: `modernize-use-emplace` **247**
  (`push_back(std::string("x"))` → `emplace_back("x")`, all clean); `modernize-use-equals-default`
  **37 kept** (destructors and default ctors → `= default`). `modernize-use-equals-delete` and
  `modernize-deprecated-headers` found nothing.
- **Warning count unchanged** from the step-2 baseline (4496): these checks do not touch the
  dominant categories (`-Wdeprecated-copy`, `-Wunused-parameter`). This step is a code-cleanup
  step, not a warning-reduction one.
- **`modernize-use-equals-default` opt-outs (4, reverted by the codemod):** on this old code it
  emitted broken syntax for `GL_point`'s copy ctor (`... ) : , = default;`, would not compile) and
  rewrote three hand-written copy-assignment operators (`GL_rect`/`GL_Vector`/`Color`) to
  `= default` — a copy/assign *semantics* change, not a mechanical cleanup. The script reverts
  exactly those four files (`core/math/point.h`, `core/math/rect.h`, `core/math/vector.h`,
  `video/color.h`); adopting `= default` there deliberately is a maintainer-reviewed-pass call.
- **`modernize-loop-convert` excluded up front** (see BLUF): correct but names each range-for local
  after its container, yielding 32 `m_`-prefixed *locals*. Range-for conversion is now umbrella
  step 8 (reviewed loops/strings pass), where a human picks the names.

## Open questions

None.
