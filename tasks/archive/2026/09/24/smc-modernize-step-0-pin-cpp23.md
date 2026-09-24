# Modernize step 0 — pin the C++ standard to C++23

**Status:** DONE 2026-09-24 (branch `modernize-cpp`).
**Part of:** `tasks/smc-modernize-cpp.md` (umbrella). **Next:** step 1 (override sweep).

## Done

`smc/configure.ac` set the standard explicitly (it was accidental — GCC's default gnu++17):
`CXXFLAGS="${CXXFLAGS} -std=c++23 -Wall -Wextra"`. Full rebuild under C++23 compiles **clean, 0
errors** — the code needed no changes to build as C++23. `-Wextra` added so later steps have a
warning metric to drive down.

## Warning baseline (the metric)

4662 warnings under `-Wall -Wextra` at C++23:

| warning | count | later step that addresses it |
|---|---|---|
| `-Wdeprecated-copy` | 2036 | step 4/5 (rule-of-0/5 as ownership moves to `unique_ptr`/values) |
| `-Wunused-parameter` | 1604 | step 7 (cleanup) / removable as APIs tighten |
| `-Wdeprecated-declarations` | 304 | step 3 (Boost/CEGUI-era APIs), step 6 |
| `-Wsign-compare` | 14 | step 7 |
| `-Wunused-variable` | 10 | ongoing |
| `-Wdeprecated-enum-enum-conversion` | 8 | step 6 (`enum class`) |

## Verification

`build.sh game` green (0 errors) under `-std=c++23`. Container build + smoke run remain the gate for
every step; a full `make image` + `tools/smoke_headless.sh` runs at the end of the initiative.

## Notes

- Chose strict `-std=c++23` (not `gnu++23`); it compiled clean, so no GNU-extension fallback needed.
- 4662 is a baseline, not a gate — steps reduce it; the build staying at 0 errors is the gate.
