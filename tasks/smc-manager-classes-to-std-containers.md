# Retire the `*_Manager` classes that are only containers; keep (and rename) the ones with behavior

**Status:** proposed — needs go-ahead (William Emerison Six <billsix@gmail.com>, 2026-11-07)
**Priority:** 5
**Difficulty:** 7

## BLUF

The maintainer asked why smc still has manager classes (`cSprite_Manager` and the rest) when the
modernization was supposed to move to smart pointers. **What was agreed and done is narrower than
removing them.** The 2026-09-24 ownership step and the 2026-10-15 coding-standard pass replaced the
managers' *raw owning storage* (`vector<T*>` + hand `delete`) with `vector<std::unique_ptr<T>>`
inside the managers, and routed ownership through `Add(std::unique_ptr<T>)`. No document ever
decided to delete the classes themselves. The record is
`tasks/archive/2026/09/24/smc-modernize-step-5-ownership.md`,
`tasks/reference/smc/objects.md` (item 5), and `tasks/reference/smc/core.md` (item 2), which all say
"`cObject_Manager<T>` → `vector<unique_ptr<T>>`". So the smart pointers are in place, and the
classes remain. This task decides, per manager, whether the class still earns its place. A manager
that is only a typed `vector<unique_ptr<T>>` with lookups becomes a plain member
(`std::vector<std::unique_ptr<T>>` or `std::unordered_map<std::string, std::unique_ptr<T>>`) plus
free algorithms. A manager with real behavior stays, perhaps under a name that says what it does.
Done = the per-manager table below filled in with decisions, the approved conversions done one
commit each (gate green after each), and `tasks/reference/refactoring-to-modern-cpp.md` updated.

## Context

- Read first: `tasks/reference/refactoring-to-modern-cpp.md` ("The ownership migration"),
  `tasks/archive/2026/10/15/smc-apply-cpp-coding-standard.md` (the `Add(unique_ptr)` API and the
  value-member pass), `tasks/reference/smc/ownership-and-lifetime.md`, and the base class
  `smc/src/core/obj_manager.h` (today: `OwningList = vector<unique_ptr<T>>` in a **public**
  `objects` member, `virtual Add/Delete_All/Get_Pointer`, `Release`, `Get_Array_Num`).
- The design problem the managers still carry is **inheritance used for storage**: 10 classes
  derive from `cObject_Manager<T>` to get a vector, then override `Add`/`Delete_All` with narrowed
  virtuals. The 2026-10-18 `-Woverloaded-virtual` sweep needed ~17 suppressions because of this
  (`tasks/archive/2026/10/18/smc-overloaded-virtual.md`). The C++ Core Guidelines prefer composition
  here, and the public `objects` member breaks encapsulation (~63 direct loops).
- Gate: nested `make image` + headless smoke. **The smoke does not exercise teardown, save/load,
  the overworld, or the editor**, so each conversion needs a maintainer play-test item.

## Inventory (2026-11-07)

Size = `.h`/`.cpp` lines; uses = `p*_Manager` global references.

| Class | Base | Size | What it adds beyond a container | First-guess verdict |
|---|---|---|---|---|
| `cObject_Manager<T>` | — | 137/— | vector + find/index | **Remove** once its subclasses no longer derive from it |
| `cSprite_Manager` | `cObject_Manager<cSprite>` | 169/326 | z-ordering (`Move_To_Front/Back`), auto-destroy slot reuse, sorted views, the player's never-owned slot | **Keep**, as a member held by composition (it is the level's object store) |
| `cWorld_Sprite_Manager` | `cSprite_Manager` | 44/56 | `Add` also feeds the layer | Fold into `cSprite_Manager` or a free function |
| `cImage_Manager` | `cObject_Manager<cGL_Surface>` | 122/198 | path→texture cache, saved textures for GL context reload | **Keep** (a real cache); `Get_Pointer` → `unordered_map` (already proposed in `reference/smc/video.md`) |
| `cSound_Manager` | `cObject_Manager<cSound>` | 93/79 | filename cache | Candidate for `unordered_map<string, unique_ptr<cSound>>` member of `cAudio` |
| `cFont_Manager` | — | 80/159 | TTF loading + software text textures | Keep (behavior), check the name |
| `cAnimation_Manager` | `cObject_Manager<cAnimation>` | in `video/animation.*` | per-frame `Update`/`Draw` of all | Likely **remove** → `vector<unique_ptr<cAnimation>>` + two loops |
| `cBackground_Manager` | `cObject_Manager<cBackground>` | in `level/level_background.*` | load/save/draw all | Likely remove |
| `cHud_Manager` | `cObject_Manager<cHudSprite>` | in `gui/hud.*` | HUD layout | Inspect |
| `cLevel_Manager` | `cObject_Manager<cLevel>` | 91/477 | active level, camera, level switching | Keep (it is game-state logic, not a container) |
| `cOverworld_Manager` | `cObject_Manager<cOverworld>` | 88/204 | active world, world loading | Keep, same reasoning |
| `cCampaign_Manager` | `cObject_Manager<cCampaign>` | 98/190 | XML load of campaigns | Likely remove → vector + a loader function |
| `cResource_Manager` | — | 49/72 | user-dir paths, no ownership | Not a container. Rename or leave |

(Verdicts are first guesses from size and grep, **not** read-verified. Step 1 verifies each one.)

## Plan

1. Read each manager and fill in the verdict column with a reason (what behavior, who calls it).
2. **Un-derive first, mechanically:** for each kept manager, replace `: public cObject_Manager<T>`
   with a private `std::vector<std::unique_ptr<T>>` member plus the few accessors callers need. This
   removes the overloaded-virtual suppressions for that class. Let the compiler list the callers.
3. **Then remove** the pure containers one at a time (animation, background, campaign, sound
   cache): the owner holds the container directly, and loops become range-`for` or algorithms.
4. Delete `cObject_Manager<T>` when nothing derives from it.
5. After each step: gate + a play-test line (level load/teardown, overworld, editor, save/load).
6. Update the reference doc's ownership section and `tasks/reference/smc/README.md` (its
   ownership map names these classes).

## Open questions

1. Is the goal (a) **remove the pure-container managers and un-derive the rest**, keeping the
   behavior-rich ones (`cSprite_Manager`, `cImage_Manager`, `cLevel_Manager`, `cOverworld_Manager`)
   as classes, or (b) **remove every `*_Manager` class**, spreading their behavior into their owners
   and free functions? *Recommend (a): (b) moves real logic into `cLevel`/`cVideo`/`cAudio` and
   gains no safety, because ownership is already `unique_ptr`.*
2. Rename the kept ones to say what they hold (`cSprite_Manager` → `cSpriteStore`, etc.)? *Recommend
   no: the names appear in about 300 call sites and in the reference set, so a rename is churn
   without safety value.*
