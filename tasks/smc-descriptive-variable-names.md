# Give loop/local variables descriptive, type-anchored names

**Status:** proposed — needs go-ahead (William Emerison Six <billsix@gmail.com>, 2026-09-24)
**Priority:** 6 · **Difficulty:** 4

## BLUF

Rename terse, generic loop and local variables to names that say what they hold. The maintainer's
example: `for (const auto& obj_ptr : objects)` says nothing — if `objects` holds `cSprite`s, the
element should read like a sprite (e.g. `sprite`), and the collection could be named for its
element type. Done = the mechanical, meaningless handles introduced across `smc/src` (especially
by the C++ modernization) read as their element type, with no behavior change (pure renames,
build + smoke green). This is a polish pass, separate from the modernization steps that created
most of the offenders.

## Context

- Trigger: during the modernization (`smc-modernize-cpp.md`) two tools mint generic names:
  - `clang-tidy modernize-loop-convert` names each range-for variable after its container
    (`m_sprites` → `sprite`, but also awkward singulars like `m_properties` → `propertie`).
  - The `cObject_Manager` → `unique_ptr` ownership flip (step 5) introduces
    `for (const auto& obj_ptr : objects) { cSprite* obj = obj_ptr.get(); ... }` — `obj_ptr` and the
    generic `obj`/`object` carry no meaning.
- The underlying design (see `tasks/reference/smc/object-model.md`) is that a manager's `objects`
  is generic (`cObject_Manager<T>`), but each concrete manager has a real element type: e.g.
  `cSprite_Manager` → `cSprite`, `cSound_Manager` → `cSound`, `cCampaign_Manager` → `cCampaign`,
  `cObjectCollisionType` → `cObjectCollision`. The good name follows the concrete type at each site.
- Naming rule for this repo (cross-project `CLAUDE.md`): externally-defined names are exempt, but
  ordinary locals should read clearly; `m_` prefix means *member*, so a loop local must never carry
  it (the modernization already avoids that, but the resulting names are still generic).

## Goal

A pass over `smc/src` that renames generic loop/local handles to type-anchored names — `sprite`,
`sound`, `campaign`, `collision`, `waypoint`, `property`, etc. — chosen per site from the concrete
element type, fixing clang-tidy's awkward singulars (`propertie` → `property`) along the way. Pure
renames, scoped to each variable's block; no behavior change.

## Plan

- [ ] Inventory the generic names: `rg -n "for \(.*\b(obj_ptr|obj|object|propertie|line|item)\b"`
      across `smc/src`, plus the `.get()` aliases the ownership flip added.
- [ ] Rename per site to the concrete element type (scoped, collision-guarded — a codemod like
      `tasks/adhoc/modernize/fix_loop_names.py` can do the scoped rename; the *choice* of name is
      per-site judgment, so this is not fully mechanical).
- [ ] `make format`; `build.sh` green; `make image` + `tools/smoke_headless.sh` green.
- [ ] Do it in reviewable commits (per subsystem), pure-rename so a diff is easy to eyeball.

## Notes / decisions

- Best done AFTER the modernization ownership + dispatch steps land, so it renames the final set of
  handles once rather than chasing a moving target. Cross-links: [[smc-modernize-cpp]] (umbrella),
  the step-5 ownership task, the step-8 loops task.

## Open questions

1. Preferred convention for the element vs. the collection: element = singular type name
   (`sprite`), collection stays `objects` (the shared member name) — or also rename collections
   where they are concrete? (My recommendation: rename the element to its type; leave the shared
   `objects` member name alone since it is inherited API. Confirm.)
