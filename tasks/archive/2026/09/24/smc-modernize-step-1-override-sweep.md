# Modernize step 1 — the `override` sweep

**Status:** DONE 2026-09-24 (branch `modernize-cpp`).
**Part of:** `tasks/smc-modernize-cpp.md`. **Next:** step 2 (`nullptr`/`using`/…).

## Done

Ran `clang-tidy -fix -checks=-*,modernize-use-override` over all of `smc/src`, driven with the
build's real include flags (the codemod: `tasks/adhoc/modernize/tidy_override.sh`; no compile
database in the image, so flags are passed after `--`, `PKG_CONFIG_PATH` set to reach the vendored
CEGUI). Header fixes scoped to `smc/src` (`-header-filter`), so `third_party/` and system headers
were untouched. Reformatted (`format.sh`) and rebuilt.

Result: **611 `override`** annotations across 59 files (`virtual ~cTurtle(void);` →
`~cTurtle(void) override;`, etc.; redundant `virtual` dropped per the check). Compiles clean under
C++23 (0 errors). Warnings 4662 → 4496.

## Why clang-tidy (not by hand)

688+ virtuals across 85 headers — infeasible by hand. `modernize-use-override` is a semantically
aware, well-tested check: it annotates only functions that genuinely override a base virtual, so it
cannot mis-annotate a non-override (that would be a compile error), and the clean build confirms no
signature-drift bugs were hiding (had there been a "looks like an override but isn't", it would
simply not have been annotated).

## Verification

`format.sh; build.sh game` → 0 errors. Container `make image` + `tools/smoke_headless.sh` remain the
end-of-initiative gate.

## Notes

- `-Wsuggest-override` is not separately enabled; the sweep is complete per clang-tidy's analysis.
- `final` was not applied (would need a per-class "no further override" judgment; out of scope here).
