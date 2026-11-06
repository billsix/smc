# Add a vendored-dependency update target for CEGUI (like Craft's)

**Status:** proposed — needs go-ahead
**Priority:** 6
**Difficulty:** 4

## BLUF
smc vendors **CEGUI 0.7.9** as a committed source tree at
`third_party/cegui-0.7.9/` with **no** re-vendor machinery: no `make update-cegui`
target, no `tools/update-cegui.sh`, no `tools/update_dep_common.sh`, no record of
how/whence the tree was fetched. Add the standard `update-<dep>` system (the one
`github.com/billsix/Craft` uses) so the pinned CEGUI source can be re-fetched from
upstream reproducibly and bumped by one command. "Done" = `make update-cegui
CEGUI_REF=<ref>` re-vendors `third_party/cegui-0.7.9/` from upstream and is proven
idempotent for the current pin (re-vendoring leaves an empty `git diff`).

## Context
Convention: "`update-<dep>` targets for third-party deps vendored BY SOURCE"
(personal overlay / `~/.claude/reference/vendoring-third-party-source-deps.md`).
Craft is the worked example — `make update-<dep> <DEP>_REF=…` + `tools/update-<dep>.sh`
sourcing the shared `tools/update_dep_common.sh` helper, recording the pin in the
commit subject. CEGUI 0.7.9 is not packaged anywhere current, so smc builds it
from this vendored tree (`entrypoint/build.sh`); the tree is the pin, and right
now nothing documents or reproduces it.

Two wrinkles specific to smc:
- **Upstream source.** CEGUI 0.7.9 is a 2011 release; locate the authoritative
  upstream (the CEGUI project's 0.7.9 release tarball / the `v0-7` bitbucket→github
  history). Confirm the ref before writing it down (never invent a URL).
- **Local patches.** If the PCRE1 blocker (see
  `tasks/upgrade-to-fedora-45.md`) is resolved by patching the vendored CEGUI
  (e.g. swapping the regex matcher to std::regex), the tree is no longer pristine
  upstream. The update script must then re-apply those local patches after
  re-fetching (keep them as a `patches/cegui/` series the script applies), or the
  re-vendor would silently drop them. Decide the patch-carrying mechanism as part
  of this task, cross-linked with the F45 task's chosen PCRE fix.

## Plan
1. Identify + confirm the upstream CEGUI 0.7.9 source and a stable ref.
2. Vendor `tools/update_dep_common.sh` from the canonical (runClaudeInContainer
   `tasks/reference/update_dep_common.sh`, byte-identical) + write
   `tools/update-cegui.sh` (repo-relative paths; fetch ref → overwrite
   `third_party/cegui-0.7.9/` → re-apply any local patch series → print the
   version from a vendored header macro if one exists, else pin by git ref).
3. Add `make update-cegui CEGUI_REF=<ref>` with the usual empty-arg usage guard.
4. Prove idempotent: re-vendor the current pin → empty `git diff`.

## Open questions
1. Upstream source of record for CEGUI 0.7.9 (tarball vs a git `v0-7` ref)? I'll
   propose one from research and confirm before hardcoding.
2. If F45's PCRE fix patches the vendored tree, carry the patches as a
   `patches/cegui/` series the update script re-applies (recommended), or fork the
   vendored tree and re-vendor only on a deliberate upstream bump?
