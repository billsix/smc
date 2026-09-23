# Reformat smc/src with clang-format (google style, 80 columns), as one commit before any other work

**Status:** proposed — go-ahead given in principle (William Emerison Six <billsix@gmail.com>,
2026-09-23: "yes, I do want to reformat, yes only smc/src, and yes git blame ignore revs";
banner protection yes; column limit 80). Execute on the word; it must land **before** the
pre-modernization fixes and the C++ modernization.
**Priority:** 3 · **Difficulty:** 3 (mechanical, but the preview + region-protection + blame-ignore
steps need care)
**Do before:** `tasks/smc-pre-modernization-fixes.md` and `tasks/smc-modernize-cpp.md` — a clean,
consistent base means every later fix and rewrite diffs cleanly on top.

## BLUF

Give the game's own source (`smc/src` only, never `third_party/`) a one-time clang-format pass with
gltron's exact `.clang-format` (google style, `SortIncludes: Never`, 80-column), as a single
standalone commit, then register that commit in `.git-blame-ignore-revs` so `git blame` still
reaches the reconstructed 2006–2014 history. The `/* *** *** */` banners survive because they are
~76 chars (under 80); any banner or aligned region the preview shows getting mangled is protected
with a `// clang-format off` marker. Done = `smc/src` is uniformly google-formatted, the game builds
and runs identically (the diff is whitespace-only), and blame skips the reformat commit.

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

## The `.clang-format` (new file at `smc/smc/.clang-format`)

Gltron's file, verbatim (maintainer, 2026-09-23: "like gltron's, sortincludes never and google
style") — no tuning, so the 80-column google default stands:

```yaml
BasedOnStyle: google
SortIncludes: Never
```

## Plan (one work commit; the archive/blame follow-ups are separate)

- [ ] Add `smc/smc/.clang-format` as above.
- [ ] **Preview first** (do not commit yet): on a copy or a throwaway branch, run
      `entrypoint/format.sh`, then `git diff` — scan for hunks where intentional column alignment
      got collapsed. Surface each to the maintainer for keep-or-reflow; `SDLKey_to_CEGUIKey` is the
      known candidate.
- [ ] Wrap each region the maintainer wants kept in `// clang-format off` … `// clang-format on`
      (permanent committed comments). Keep the list short — protect alignment that aids reading, not
      merely-neat blocks.
- [ ] Apply for real: `make format` (or `entrypoint/format.sh`) over `smc/src`.
- [ ] **Prove it is whitespace-only:** `git diff -w -- smc/src` must be empty except inside the
      `clang-format off` regions (which are unchanged anyway). This is the gate that it changed no
      tokens.
- [ ] Build + run gate: `make image` green, `tools/smoke_headless.sh` passes (clang-format cannot
      change behaviour, but confirm). Stage `smc/src` + the new `.clang-format`.
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
- Style: gltron's `.clang-format` verbatim — `BasedOnStyle: google` + `SortIncludes: Never`, no
  other keys, so the 80-column google default applies. Banners/aligned regions are protected with
  `// clang-format off` markers only where the preview shows a need (most survive untouched).
- Own single commit, **before** fixes and modernization.
- `.git-blame-ignore-revs` at repo root, with the reformat SHA, to preserve history-provenance blame.
- Split of duties: agent writes `.clang-format`, runs the preview, sets up the `clang-format off`
  regions, applies, verifies whitespace-only, and stages. The maintainer makes the commit (so the
  agent then reads back the SHA for the blame-ignore file).

## Open questions

None — the four decisions are made. Awaiting only "go" to run the preview.
