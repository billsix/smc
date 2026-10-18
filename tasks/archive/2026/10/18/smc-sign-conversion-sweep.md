# smc: resolve `-Wsign-conversion` and add it to the gate

**Status:** DONE 2026-10-17 (work committed by the maintainer; this doc is the retrospective).
**Priority:** 6 · **Difficulty:** 7 (anticipated) → in practice far smaller once third-party headers
were excluded. **Created:** 2026-10-17 (William Emerison Six <billsix@gmail.com>)
**Follows:** `smc-const-sweep-and-warning-gate.md`, which raised the gate to the fleet set **minus
`-Wsign-conversion`** and deferred this class. This task added `-Wsign-conversion` to the gate and drove
it to zero.

## BLUF

The gate pass had measured **1835 `-Wsign-conversion`** and deferred it as too large to grind by hand,
planning a clang-AST bucketing approach. It turned out much smaller and simpler: the 1835 was
pre-`-isystem` (mostly CEGUI headers). Scope: `smc/src/` only. Done = `-Wsign-conversion` in the gate,
green under gcc **and** clang with `-Werror`, `make image` + `tools/smoke_headless.sh` pass. Achieved.

## What was done

- **Re-measured first-party only** (`tasks/adhoc/smc-sign-conversion-sweep/measure.sh`): with SDL/CEGUI
  as `-isystem`, the true delta was **273** warnings under gcc, in simple buckets (`unsigned↔int` 183,
  `int←Uint32` 24, a long tail). The elaborate clang-query/AST bucketing the plan envisioned was
  unnecessary at this size — the type pairs were obvious.
- **Fix = make each implicit conversion an explicit `static_cast<DEST>` at the site.** This is
  behaviour-identical (the cast does exactly what the implicit conversion already did), so the sweep is
  behaviour-preserving by construction. Fanned out one Edit-only subagent per directory
  (`tasks/adhoc/smc-sign-conversion-sweep/BRIEF.md`); 273 sites cast. Dominant patterns:
  `Get_Pointer(collision->m_number)` → `static_cast<size_t>`, SDL / `int_to_string` call-arg casts,
  `vec[idx]` → `static_cast<size_t>`. Boundary casts only — no variable retyped (fleet rule).
- **clang surfaced 8 more that gcc permits.** After the gcc-measured 273, the clang `-Werror` build
  flagged `int`→`Uint8` (even in-range constants) in `Color(r,g,b,a)` literal/`rand()%N` args in
  `ball.cpp`, `enemy.cpp`, `level_player.cpp` — cast those too. The gate must be green under both, so
  the clang worklist, not just gcc's, is the bar.
- **Added `-Wsign-conversion` to `smc/configure.ac`.** The clang-only finer-conversion flags
  (`-Wshorten-64-to-32`, `-Wimplicit-int-conversion`) were NOT added — a separate narrower pass if ever
  wanted; this task's `-Wsign-conversion` goal is met.

## Verification

gcc `-Werror` = 0, clang `-Werror` = 0, `make image` builds, `make format` applied,
`tools/smoke_headless.sh` passes (ran to the 30 s deadline, ps audit clean). No gameplay change. The
"gcc vs clang differ" and "static_cast-at-site is behaviour-preserving" lessons were harvested into
`cpp-build-modernization.md` / `cpp-coding-standard.md` (runClaude/runCrush, 2026-10-18).
