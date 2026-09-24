# Bug: command-line argument parsing is broken (`option = i + i`, `arguments[1]`)

**Status:** DONE 2026-09-23 (branch `pre-modernization-fixes`). **Done:** the `--debug` value loop
now iterates `option = i + 1 .. size` (was `i + i` with the outer counter, re-reading
`arguments[2*i]`), consuming each token via `i = option`; and the `--world` branch tests
`arguments[i]` (was `arguments[1]`). Compiles green. The later `--level`/`--world` value handling
(reading `arguments[1]`/`[2]` directly) was already correct and untouched.
**Priority:** 6 · **Difficulty:** 2

## BLUF

`main()`'s argument loop has two typos: it computes `option = i + i` (double, not `i + 1`) and never
advances it, and it tests `arguments[1]` instead of `arguments[i]` for `--world`. So options that
take a value (`--level <name>`, `--world <name>`) read the wrong index or loop. Fix both.

## Context

- Verified in `tasks/reference/smc/core.md` (§4 and §9).
- `smc/src/core/main.cpp` (~:142): `for( unsigned int option = i + i; i < arguments.size(); i++ )`
  — `option` is `2*i` (meant `i + 1`) and never advances, so a value argument is read from the wrong
  slot and the same token can be re-processed until `i` runs out.
- `smc/src/core/main.cpp` (~:168): `arguments[1] == "--world"` tests index 1 rather than `i`.

## Fix

Use `i + 1` for the value index (with a bounds check `i + 1 < arguments.size()`), and test
`arguments[i]` in the `--world` branch. Consider consolidating the `--level`/`--world` handling
while here (both take the next token). Keep it a small, self-contained fix.

## Verification

Container build green; run `smc --level <name>` and `smc --world <name>` (headless is fine to reach
the parse) and confirm the right value is picked up (add a temporary debug print if needed, remove
before committing). Bare `smc` unaffected.

## Open questions

None.
