# Reformat smc/src with clang-format (google style, 80 columns), as one commit before any other work

**Status:** DONE — the reformat landed as `bec5840` "make format" (an ancestor of master and every
work branch); `.git-blame-ignore-revs` lists it; `smc/smc/.clang-format` is in place and
`make format` maintains it; `CLAUDE.md` records "smc/src is clang-format google style". Verified
token-identical at the time and the game has built + smoked green through all later work
(William Emerison Six <billsix@gmail.com>, 2026-09-23 landed; archived 2026-09-24).
**Priority:** 3 · **Difficulty:** 3 (mechanical, but the preview + region-protection + blame-ignore
steps need care)
**Do before:** `tasks/smc-pre-modernization-fixes.md` and `tasks/smc-modernize-cpp.md` — a clean,
consistent base means every later fix and rewrite diffs cleanly on top.

## BLUF

Give the game's own source (`smc/src` only, never `third_party/`) a one-time clang-format pass with
a google-based `.clang-format` (gltron's base + 80-column + three keys that make it layout-only), as
a single standalone commit, then register that commit in `.git-blame-ignore-revs` so `git blame`
still reaches the reconstructed 2006–2014 history. The `/* *** *** */` banners (546 of 565 exceed 80
cols) are kept intact by `ReflowComments: false`. Done = `smc/src` is uniformly google-formatted,
the reformat is **provably token-identical** (changes layout only — verified on a scratch preview),
the game builds and runs the same, and blame skips the reformat commit.

## Context

- This reverses the earlier "keep upstream's hand style, format.sh is opt-in" decision (recorded in
  `CLAUDE.md` and `tasks/reference/smc/../building-2010-era-smc-today.md`) — deliberately, and only
  because it is done **first**, as its own commit, so it does not drown the later minimal-change
  diffs. Update those two docs when this lands (the hand-style note becomes "google-formatted since
  <commit>; `make format` maintains it").
- **Current style vs google** (measured 2026-09-23): SMC uses **tabs** (599 tab-indented lines in
  `main.cpp` alone), `cClass :: Method` with spaces around `::` (~1,919 method definitions), lines
  up to **227 chars**, and `{` on its own line. google is 2-space, `cClass::Method`, 80-column,
  attached braces. So the diff is effectively all ~73k lines — expected and accepted.
- **What actually needs protecting is small.** Most "tables" are single-space style google keeps
  fine (`SpriteType` enum `TYPE_MASSIVE = 46,`; the `color.h` named colors). The genuinely
  column-aligned code is narrow — the known case is `input/keyboard.cpp:SDLKey_to_CEGUIKey`, whose
  `case SDLK_X:    return CEGUI::Key::Y;` returns line up in a column that google collapses to one
  space. The banners are `/* *** *** */` block comments ~76 chars wide, under the 80-column limit,
  so google's default comment handling leaves them alone; the `clang-format off` markers are the
  fallback for any specific banner or aligned region the preview shows getting reflowed.
- `entrypoint/format.sh` already runs `clang-format -i` over `smc/src`; it will pick up the new
  `.clang-format` automatically. clang-format discovers `.clang-format` by searching upward from
  each file, so placing it at `smc/smc/.clang-format` (the game dir) scopes it to `smc/src` and
  leaves `third_party/` (a sibling of `smc/`) untouched.
- `clang-format` ships in the image via `clang-tools-extra`. Its **version** affects output, so
  record it in the commit message (`clang-format --version`) — re-running a different version later
  would produce spurious churn.

## The `.clang-format` (new file at `smc/smc/.clang-format`, already written)

gltron's google base + 80-column, with three keys added so the pass is **layout-only** — the
preview proved that gltron's 2-line file plus these three changes **no tokens at all** across
`smc/src` (whitespace-stripped files are byte-identical before/after). Each added key stops a google
default that would rewrite/reorder tokens, which is against the maintainer's "keep it as written"
intent (the same reason they set `SortIncludes: Never`):

```yaml
BasedOnStyle: google
SortIncludes: Never            # keep include order
ReflowComments: false          # keep the /* *** *** */ banners (546 of 565 exceed 80 cols) intact
SortUsingDeclarations: false   # don't alphabetize `using std::x;`
BreakStringLiterals: false     # don't split long string literals across lines
```

Findings that forced each added key (preview, clang-format 22.1.8, 2026-09-23): without
`ReflowComments: false`, 546 banners reflow into ugly two-liners; without `SortUsingDeclarations:
false`, google alphabetizes the `using std::…` block in `global_basic.h`; without
`BreakStringLiterals: false`, ~19 files get long strings split into adjacent literals (safe, but a
token rewrite). With all three, the reformat is provably layout-only.

## Plan (one work commit; the archive/blame follow-ups are separate)

- [x] `smc/smc/.clang-format` written (the 5-line file above), staged.
- [x] **Preview done** (2026-09-23, on a scratch copy, clang-format 22.1.8): with the 5-line config
      the whitespace-stripped tree is byte-identical before/after — **no `// clang-format off`
      markers are needed**. Banners are handled by `ReflowComments: false`; the only "alignment" that
      changes is column padding (e.g. the `SDLKey_to_CEGUIKey` returns) collapsing to single spaces,
      which is acceptable whitespace normalization, not content loss.
- [ ] Maintainer runs `make format` over `smc/src` (formats in place on the host).
- [ ] **Prove it is layout-only:** the strong check is token equivalence, not `git diff -w` (which
      flags brace-moves and rewraps because they add newlines). Confirm the whitespace-stripped
      files are unchanged: for each file, `diff <(tr -d '[:space:]' <new) <(tr -d '[:space:]' <old)`
      is empty. (The preview already proved this for the committed config; re-confirm on the real run.)
- [ ] Build + run gate: `make image` green, `tools/smoke_headless.sh` passes. Stage `smc/src`.
- [ ] **Maintainer commits it as ONE standalone commit** titled e.g. "reformat smc/src with
      clang-format (google, 80 col)"; the commit message records the `clang-format --version`.
- [ ] **After the commit exists, capture its SHA into `.git-blame-ignore-revs`** at the repo root
      (one SHA per line, a comment above it), in a tiny follow-up commit; and document
      `git config blame.ignoreRevsFile .git-blame-ignore-revs` in `README.md`/`CLAUDE.md` so other
      clones opt in (GitHub honors the file automatically). This is the chicken-and-egg step: the
      SHA is unknown until the reformat is committed.
- [ ] Update `CLAUDE.md` + `building-2010-era-smc-today.md` (hand-style note → "google-formatted").
- [ ] Optional: refresh the exact code snippets quoted in the `smc-bug-*` tasks so they match the
      reformatted source (they match by symbol/content, so this is tidiness, not required).

## Notes / decisions (all settled 2026-09-23)

- Scope: `smc/src` only. Never `third_party/cegui-0.7.9` (vendored dependency).
- Style: google + `SortIncludes: Never` (gltron's base) + three keys that make the pass layout-only
  (`ReflowComments: false`, `SortUsingDeclarations: false`, `BreakStringLiterals: false`). 80-column
  stands. See the `.clang-format` section for why each was added; the preview proved token-identity,
  so no `// clang-format off` markers are needed anywhere.
- Own single commit, **before** fixes and modernization.
- `.git-blame-ignore-revs` at repo root, with the reformat SHA, to preserve history-provenance blame.
- Split of duties: agent writes `.clang-format`, runs the preview, sets up the `clang-format off`
  regions, applies, verifies whitespace-only, and stages. The maintainer makes the commit (so the
  agent then reads back the SHA for the blame-ignore file).

## Open questions

None — the four decisions are made. Awaiting only "go" to run the preview.
