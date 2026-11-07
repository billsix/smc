# Replace the `*_Manager` classes with modern ownership shapes (and fix the stored-pointer lifetimes)

**Status:** in progress — phase 1 and `cBackground_Manager` done; continuing phases 2–8 on branch
`reformat-and-managers` (2026-11-07). The maintainer lifted the per-phase play-test gate: go ahead, and
he plays the accumulated play-test list afterwards. Direction decided by William Emerison Six
<billsix@gmail.com>, 2026-11-07.
**Priority:** 4
**Difficulty:** 7

## BLUF

Replace each `*_Manager` with the modern C++ shape its job calls for. A manager that is only a
container becomes a private standard container. A real cache becomes a cache class named for what
it caches. `cObject_Manager<T>` (inheritance used for storage) goes away. The sprite→sprite
pointers that can dangle get **generational handles**. Done = no class derives from
`cObject_Manager`, which is deleted; every kept class holds its storage privately and is named for
what it guarantees; the ~6 dangling-capable stored `cSprite*` members are handles; the `-Werror`
gate is green under gcc and clang; smoke is green; there is a maintainer play-test after each
phase; and `tasks/reference/refactoring-to-modern-cpp.md` records the pass.

## Decisions (William Emerison Six <billsix@gmail.com>, 2026-11-07)

- **Replace, not just remove.** The maintainer's framing: modern C++ should make lifetime
  "managers" much simpler. This replaces the earlier (a)/(b) "remove some vs remove all" question.
- **The agent's design input, written into the shared C++ standard** (runClaudeInContainer
  `tasks/reference/cpp-coding-standard.md`, new section "Stored borrows and 'manager' classes",
  mirrored to runCrushInContainer), applied here as follows:
  - **Raw pointers passed as parameters are not the problem.** The C++ Core Guidelines (`F.7`,
    `R.30`) say a function that only uses an object should take `T*`/`T&`, not a smart pointer.
    smc's ~300 raw-pointer parameters are mostly correct as they are. The gain is converting the
    never-null ones to `T&` opportunistically, not a sweep.
  - **The real hazard is stored pointers whose target can die first.** Ownership is already
    `unique_ptr` (2026-09-24 pass), so the work left is the *borrow* side, sorted by category
    below.
  - **A container class must earn its existence with an invariant** (cache, draw order, deferred
    destruction, handle validation), be held by composition with private storage, and be named
    for that invariant.

## Context

- Read first: the standard section above; `tasks/reference/refactoring-to-modern-cpp.md`
  ("The ownership migration"); `tasks/reference/smc/ownership-and-lifetime.md`;
  `tasks/archive/2026/10/15/smc-apply-cpp-coding-standard.md`; `smc/src/core/obj_manager.h`.
- What the earlier passes did: the managers' *storage* went from `vector<T*>` + hand `delete` to
  `vector<unique_ptr<T>>`, and ownership enters through `Add(std::unique_ptr<T>)`
  (`tasks/archive/2026/09/24/smc-modernize-step-5-ownership.md`). No document decided to remove the
  classes, which is why they remain.
- Remaining design debts: 10 classes inherit from `cObject_Manager<T>` to get a **public**
  `objects` vector (about 63 direct loops reach in), and override `Add`/`Delete_All` with narrowed
  virtuals. That inheritance is the source of most of the ~17 `-Woverloaded-virtual` suppressions
  (`tasks/archive/2026/10/18/smc-overloaded-virtual.md`).
- Gate: nested `make image` + `make shell-exec SCRIPT=tools/smoke_headless.sh`. **The smoke run
  does not exercise teardown, save/load, the overworld, or the editor**, so each phase needs a
  maintainer play-test item.

## Stored raw-pointer members, by category (survey 2026-11-07, `grep` of `smc/src/**/*.h`)

Categories are the standard's four: (1) outlived by construction → raw `T*`/`T&` + comment;
(2) can die first → deferred destruction or owner-clears; (3) cross-object refs in a store →
generational handle; (4) truly shared → `shared_ptr`/`weak_ptr`.

| Member(s) | Count | Category | Action |
|---|---|---|---|
| `cSprite_Manager* m_sprite_manager`, `cOverworld* m_overworld`, `cLevel* m_level`, `cParticle_Emitter* m_parent`, `cLayer_Line_Point_Start* m_line`, `cLayer_Line_Point* m_linked_point` | ~16 | 1 (owner/parent outlives) | Keep raw; add the "owned by X" comment; `T&` where never null and never reseated |
| `cGL_Surface* m_image`/`m_start_image`/`m_maryo_head`/`m_item_image`/`m_goldpiece`/`m_base`/`m_arrow_*` | ~10 | 1, **verify**: the image cache lives for the whole application, but check that a resolution-change texture reload (`cImage_Manager::m_saved_textures`) reloads *in place* and keeps the `cGL_Surface*` valid | Keep raw if the check passes |
| `cSound* m_data` | 1 | 1 (sound cache lives for the application) | Keep raw |
| `cSprite* m_ground_object`, `cSprite* m_active_object` / `cMovingSprite* m_active_object`, `cSprite* m_last_clicked_object`, `cSprite* m_obj` (×2) | ~6 | **2/3: can dangle.** Today they are safe only because of `m_auto_destroy` (a destroyed sprite stays in the vector until its slot is reused by `reset()`) plus hand-nulling at about 8 sites | **Generational `SpriteHandle`** issued by the sprite store; `Get(handle)` returns `nullptr` once the slot is reused |
| 35 `extern cX* pX` global aliases (`pLevel_Manager` 62 uses, `pOverworld_Manager` 60, `pActive_Animation_Manager` 51, …) | 35 | 1 (owned by `cApplication`, application lifetime) | Leave as the documented migration stage. Passing context by reference is the end state, but it is out of scope here (thousands of call sites) |

## Per-manager outcome (agent's recommendation)

| Today | Becomes | Why |
|---|---|---|
| `cObject_Manager<T>` | **deleted** | Inheritance used for storage, with a public owning vector |
| `cSprite_Manager` | `cSprite_Store`: private `vector<unique_ptr<cSprite>>` + slot generations + draw order (`Move_To_Front/Back`) + deferred-destroy sweep; issues `SpriteHandle` | It has real invariants. The rename says what it guarantees |
| `cWorld_Sprite_Manager` | folded into `cSprite_Store` (or one free function feeding the layer) | 44/56 lines; one extra side effect in `Add` |
| `cImage_Manager` | `cTexture_Cache`: private `unordered_map<path, unique_ptr<cGL_Surface>>` + reload-on-context-loss | A real cache. Map lookup replaces the linear `Get_Pointer` scan (`reference/smc/video.md`) |
| `cSound_Manager` | `cSound_Cache` (same shape), owned by `cAudio` | A cache by filename |
| `cFont_Manager` | `cFont_Renderer` (or keep the name). No container change | It is TTF rendering behavior, not storage |
| `cAnimation_Manager` | private `vector<unique_ptr<cAnimation>>` in its owners + two loops (`Update`, `Draw`) | A pure container |
| `cBackground_Manager` | `vector<unique_ptr<cBackground>>` member of `cLevel` + load/save free functions | A pure container |
| `cCampaign_Manager` | `vector<unique_ptr<cCampaign>>` + a `Load_Campaigns()` function | A pure container + loader |
| `cHud_Manager` | inspect, likely `cHud` owning its sprites by composition | Layout behavior, not lifetime |
| `cLevel_Manager`, `cOverworld_Manager` | keep as classes, un-derived, storage private (optionally `cLevel_Switcher`/`cWorld_Map`) | Game-state logic (active level/world, camera, switching) |
| `cResource_Manager` | `cUser_Paths` (or keep). No container change | Path lookup; owns nothing |

Each rename is mechanical and compiler-verified. Do it by type name only, in its own commit, and
update `tasks/reference/smc/` in the same commit (the reference set names these classes).

## Progress (2026-11-07, authorized overnight batch)

- **Phase 1 findings:**
  - `cGL_Surface*` members are **category 1 (safe as raw pointers)**: `cImage_Manager::Restore_Textures`
    reloads each saved texture *in place* (`soft_tex->m_base->Load_Software_Texture(...)`), so a GL
    context reload never invalidates a stored surface pointer.
  - **`cAnimation_Manager` stays a class** (the table's "likely remove" guess was wrong): 44
    `pActive_Animation_Manager->Add` sites, the per-scene `Update`/`Draw` lifecycle, and the "active"
    pointer that switches between level, overworld, and menu. Turning it into a bare vector would
    scatter that behaviour. It gets the phase-2 treatment (composition, private storage) instead.
  - `cBackground_Manager` was the one **pure container** (an empty subclass).
- **Done:** `cBackground_Manager` → `cLevel::m_backgrounds` (`vector<unique_ptr<cBackground>>`) +
  `Gradient_Background()` (commit `1d980d9`; codemod
  `tasks/adhoc/smc-manager-classes-to-std-containers/backgrounds_to_vector.py`). gcc + clang `-Werror`
  clean, rebuild + smoke pass. **Play-test owed:** level backgrounds draw; the editor's level-settings
  screen adds/deletes background images and edits the gradient colours; the main menu's background
  fades.
- **Phase 2 done (un-derive):** `cObject_Manager<T>` and `core/obj_manager.h` are deleted. The nine
  subclasses (sprite, collision list, sound, campaign, HUD, level, overworld, animation, image) each
  hold a private `Owning_Vector<T>` from the new `core/owning_vector.h`, which is `vector<unique_ptr<T>>`
  plus free lookups (`Find_Owned`, `Erase_Owned`, `Release_Owned`, `Borrow_At`, `Index_Of`) and
  `Borrowed()`, a `std::views::transform` view of `T*`. Each class declares only what its callers
  use: `Objects()` (the view, on the seven classes iterated from outside), `Add`/`Delete_All` where
  the base supplied them, `size`/`empty`/`Get_Pointer`/`Get_Array_Num` where called, and
  `cObjectCollisionType::Take_All`/`Append`. Those replace the two places that moved collisions
  between lists by reaching into `objects`. The 63 outside `->objects` loops became
  `for (auto* x : mgr->Objects())`. Mechanical part: codemod
  `tasks/adhoc/smc-manager-classes-to-std-containers/underive.py` (idempotent); the rest was
  compiler-driven.
  - **Two traps:** clang rejects an inline `Add`/`Delete_All` in `level_manager.h` and
    `world_manager.h`, because `cLevel`/`cOverworld` are incomplete there, so those bodies live in the
    `.cpp`. `cSound_Manager::Get_Pointer(string)` was `virtual` with nothing deriving, which tripped
    clang's `-Wdelete-non-abstract-non-virtual-dtor` once the virtual base destructor was gone, so it
    is non-virtual now. `cSprite_Manager` keeps a virtual `Add`/`Delete_All`/destructor, because
    `cWorld_Sprite_Manager` derives from it until phase 5.
  - **`-Woverloaded-virtual`:** only 2 of the ~17 suppressions belonged to the managers, on
    `cObject_Manager::Delete_All` and `cSprite_Manager::Delete_All(bool)`; both are gone. The other 15
    are the sprite hierarchy's deliberate narrowing (`enemies/*.h`, `objects/movingsprite.h`,
    `input/mouse.h`, `video/animation.h`) and are outside this task.
  - Gates: gcc and clang `-Werror` clean (0 warnings); `build.sh game` + smoke pass.
  - **Play-test:** a level plays as before (enemies, items, collisions, stomping, falling into a
    pit); the HUD shows points/time/lives/gold/item box; the overworld map loads and walks; the
    main menu's campaign and world lists fill; the save/load of a game in a level and on the
    overworld; the editor's object placement and deletion.
- **Pause lifted (William Emerison Six <billsix@gmail.com>, 2026-11-07):** "just go ahead, I'll try to
  play test it later." Each phase still passes the gcc/clang gate and smoke, and adds its items to the
  play-test list below; the maintainer plays the whole list once.
- **Why it had paused:** the remaining phases touch gameplay that the smoke can't see (the sprite
  store's draw order, auto-destroy slot reuse, and the ground/active-object pointers; the level and
  overworld switching). This task requires a play-test between phases; stacking them unattended would
  trade verified gameplay for structure.
- **Formatting:** the tree is clang-format 23 clean since `cfbc1d5` (2026-11-07), so a whole-file
  `clang-format -i` (or `make format`) on an edited file is fine.
- **Gate for every phase:** `tasks/adhoc/smc-port-build-to-cmake/compiler_gate.sh` with `CXX=g++` and
  `CXX=clang++`, then `build.sh game` + `tools/smoke_headless.sh` in one container (see that script's
  header for the `podman run` line).

## Plan (each phase its own commits; gate + play-test after each)

1. **Verify** the `cGL_Surface*` reload question and read each manager to confirm the outcome
   table, amending it where reading contradicts the guess.
2. **Un-derive:** every subclass holds a private container instead of inheriting from
   `cObject_Manager<T>`. Callers that reached into `objects` get a borrow view or an algorithm.
   Delete `cObject_Manager`, and drop the `-Woverloaded-virtual` suppressions this removes.
3. **Pure containers → members:** animation, background, campaign.
4. **Caches:** `cTexture_Cache`, `cSound_Cache` (`unordered_map`).
5. **`cSprite_Store` + `SpriteHandle`:** generation per slot; convert the ~6 dangling-capable
   members; remove the hand-nulling. Play-test: stomping/ground contact, editor selection and
   clicking, item pickup.
6. **Renames** (one commit per type) + reference-set update.
7. Optional, opportunistic: never-null `T*` parameters → `T&`, only in files already being edited.
8. Record the pass in `tasks/reference/refactoring-to-modern-cpp.md` (a "third pass" section).
