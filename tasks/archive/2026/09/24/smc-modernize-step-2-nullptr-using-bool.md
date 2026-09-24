# Modernize step 2 — nullptr, using, bool literals

**Status:** DONE 2026-09-24 (branch `modernize-cpp`); one sub-item deferred (below).
**Part of:** `tasks/smc-modernize-cpp.md`. **Next:** step 3 (drop Boost).

## Done

clang-tidy `-fix` over all of `smc/src` with `modernize-use-nullptr,modernize-use-using,
modernize-use-bool-literals` (codemod: `tasks/adhoc/modernize/tidy_step2.sh`, same flag/header-filter
setup as step 1). 141 files changed, reformatted, compiles clean under C++23 (0 errors).

- `NULL` → `nullptr`: 487 → 87 remaining (400 converted). The 87 left are contexts clang-tidy is
  conservative about (macro bodies, ambiguous integer/pointer uses); harmless, can be swept later.
- `typedef` → `using`: all converted (0 `typedef` left in `smc/src`).
- int-literal-as-bool → `true`/`false` where a bool is expected.

## Deferred

`#define` constants → `constexpr` / `inline constexpr std::string_view` — clang-tidy has no safe
auto-fix for this (only `cppcoreguidelines-macro-usage`, which flags, not fixes). It is a manual
pass (the `#define` constants in `core/global_game.h` etc.); lower value, do it in a later step or
alongside step 6 (enums). Noted so the umbrella's step 2 is honestly partial.

## Verification

`format.sh; build.sh game` → 0 errors. All three checks are behaviour-preserving (nullptr, alias
rename, bool literal). Full `make image` + smoke is the end-of-initiative gate.
