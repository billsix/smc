# Add a vendored-dependency update target for CEGUI (like Craft's)

**Status:** proposed — needs go-ahead (deferred: do after the F45 branch merges)
**Priority:** 6
**Difficulty:** 4

## BLUF
smc vendors **CEGUI 0.7.9** as a committed source tree at
`third_party/cegui-0.7.9/` with **no** re-vendor machinery: no `make update-cegui`
target, no `tools/update-cegui.sh`, no `tools/update_dep_common.sh`, no record of
how/whence the tree was fetched. Add the standard `update-<dep>` system (the one
`github.com/billsix/Craft` uses) so the pinned CEGUI source can be re-fetched from
upstream reproducibly and bumped by one command. "Done" = `make update-cegui
CEGUI_REF=<ref>` re-vendors `third_party/cegui-0.7.9/` from upstream, **re-applies
the local patch series (below)**, and is proven idempotent for the current pin
(re-vendoring leaves an empty `git diff`).

## Context
Convention: "`update-<dep>` targets for third-party deps vendored BY SOURCE"
(personal overlay / `~/.claude/reference/vendoring-third-party-source-deps.md`).
Craft is the worked example — `make update-<dep> <DEP>_REF=…` + `tools/update-<dep>.sh`
sourcing the shared `tools/update_dep_common.sh` helper, recording the pin in the
commit subject. CEGUI 0.7.9 is not packaged anywhere current, so smc builds it
from this vendored tree (`entrypoint/build.sh`); the tree is the pin, and right
now nothing documents or reproduces it.

## The vendored tree is NOT pristine upstream — it carries local patches
The re-vendor machinery must re-apply these after fetching, or a re-vendor
silently drops them. Keep them as a `patches/cegui/` series the update script
applies (the recommended mechanism). Current local modifications to 0.7.9:

- **std::regex matcher backport (F45 PCRE1 removal).** Fedora 45 removed PCRE1
  (`pcre`/`pcre-devel`); CEGUI 0.7.9's `CEGUIPCRERegexMatcher` used the PCRE1 C
  API. We reimplemented that matcher with **C++ `std::regex`** (the same engine
  upstream CEGUI 0.8 switched to) so the editbox validation the SMC level editor
  relies on keeps working with no external regex library. Files touched:
  - `cegui/src/CEGUIPCRERegexMatcher.cpp` + `cegui/include/CEGUIPCRERegexMatcher.h`
    — body now uses `std::regex` / `std::regex_match` (full-string match, same
    semantics as the old PCRE length check); class name/filename kept so
    `CEGUIEditbox.cpp` and `Makefile.am` are untouched.
  - `acinclude.m4` `CEGUI_CHECK_PCRE` — no longer probes `libpcre`; defines
    `CEGUI_HAS_PCRE_REGEX`, forces `AM_CONDITIONAL([CEGUI_BUILD_PCRE_REGEX])`
    true, and leaves `pcre_CFLAGS`/`pcre_LIBS` empty.
  - (`entrypoint/01-install-base.sh` dropped `pcre-devel` — that's a smc build
    file, not part of the vendored tree, so it is NOT in the patch series.)
  - Any GCC-16 / clang-23 compile fixes the 2011-era CEGUI needs under the
    current toolchain (added as part of the F45 build; fold each into the series).

  Full rationale + the options we weighed (std::regex vs PCRE2 vs vendoring PCRE1
  vs upgrading CEGUI): the archived `tasks/upgrade-to-fedora-45.md` done-record.

## Options WHEN we update to a newer CEGUI (future, not this task)
Deferred by the maintainer until after the F45 branch merges. When we do move off
0.7.9, the regex local-patch question changes shape:

- **CEGUI 0.8.x** ships `StdRegexMatcher` (std::regex) **natively**, PCRE optional
  — so our std::regex backport becomes **redundant and should be dropped** (use
  upstream's matcher, don't re-apply our patch). BUT 0.8 is a **major API break**
  from 0.7 (ImageManager, Font/Window APIs, renderer bootstrap); SMC calls CEGUI
  pervasively, so adopting it is a real port, not a re-vendor. Upstream's successor
  TSC (The Secret Chronicles) did migrate to 0.8 — precedent to mine.
- **CEGUI "v0" / later (CMake-only)** — no autotools; the `build.sh` autotools path
  and the acinclude.m4 patch would both be replaced by a CMake configure. Bigger
  build-system change.
- **Decision rule for the update:** if the new version provides its own std::regex
  (or otherwise PCRE-free) matcher, retire our patch; only keep a regex patch if a
  target version still hard-requires PCRE1. Re-evaluate the GCC/clang compile
  patches against the new tree (a version bump can fix or move them).

So this task has two distinct futures: (a) **reproducible re-vendor of the SAME
0.7.9 + our patches** (what the `update-cegui` target is for first), and (b) a
**separate, larger task** to port SMC to a newer CEGUI, which would supersede the
regex patch entirely. Keep them separate; this task is (a).

## Plan
1. Identify + confirm the upstream CEGUI 0.7.9 source and a stable ref (tarball vs
   a git `v0-7` ref). Confirm before hardcoding — never invent a URL.
2. Extract the current local modifications into a `patches/cegui/` series.
3. Vendor `tools/update_dep_common.sh` from the canonical (runClaudeInContainer
   `tasks/reference/update_dep_common.sh`, byte-identical) + write
   `tools/update-cegui.sh` (repo-relative paths; fetch ref → overwrite
   `third_party/cegui-0.7.9/` → re-apply `patches/cegui/` → print the version from
   a vendored header macro if one exists, else pin by git ref).
4. Add `make update-cegui CEGUI_REF=<ref>` with the usual empty-arg usage guard.
5. Prove idempotent: re-vendor the current pin + re-apply patches → empty `git diff`.

## Open questions
1. Upstream source of record for CEGUI 0.7.9 (tarball vs a git `v0-7` ref)? I'll
   propose one from research and confirm before hardcoding.
