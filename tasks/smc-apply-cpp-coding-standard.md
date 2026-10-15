# Bring the modernized tree in line with the shared C++ coding standard (ownership first)

**Status:** proposed — needs go-ahead (William Emerison Six <billsix@gmail.com>, 2026-10-15)
**Priority:** 3 · **Difficulty:** 6 (mostly compiler-driven mechanical edits; two phases are
gameplay/teardown-sensitive and need the maintainer's play-test)
**Depends on:** nothing in flight; the modernization umbrella is archived
(`tasks/archive/2026/09/24/smc-modernize-cpp.md`) and `master` carries it. Gate per commit:
nested `make image` + `make shell-exec SCRIPT=tools/smoke_headless.sh`.
**Follow-on (separate, after this executes):** harvest what the execution teaches back into the
shared `cpp-coding-standard.md` (canonical in runClaudeInContainer, mirrored twice in
runCrushInContainer) — the maintainer asked for that explicitly.

## BLUF

The 2026-09-24 modernization put ownership into `unique_ptr` at the *storage* (the managers, the
render queue, the savegame graph, the `cApplication`) but left it **raw at every API boundary**:
`Add(T*)` adopts a raw pointer, `Copy()` returns a `unique_ptr` that eleven callers immediately
`.release()`, and ~35 owning members plus ~30 transient `col_list` owners still hand-`delete`. That
is why the tree reads as "lots of `unique_ptr`, lots of `.get()`": the smart pointer is at the
edges of a raw-pointer protocol instead of *being* the protocol. This task makes ownership transfer
flow through the type system end to end (`Add(std::unique_ptr<T>)`, `make_unique`, `unique_ptr`
returns from factories, `unique_ptr` members with the rule of zero), then applies the smaller
standard items (`const auto&` observation loops, `auto` for iterators, no `.get()` loop preludes,
RAII teardown of the application object, `std::optional` for the one sentinel return). Done = zero
`.release()` outside a documented hand-off, zero owning `delete` outside CEGUI's own-contract
sites, the 11 `Copy().release()` gone, build + smoke green per commit, and the maintainer's
play-test (editor copy/paste, save/load, quit) clean.

## Context

Read first: `tasks/reference/refactoring-to-modern-cpp.md` (what the modernization did and
deliberately skipped), `tasks/reference/smc/ownership-and-lifetime.md` (the pre-modernization map;
still accurate for *who owns whom*), `~/.claude/reference/cpp-coding-standard.md` (the standard
this task applies — its "Ownership and lifetime" and "`auto`" sections are the yardstick), and
`~/.claude/reference/cpp-ownership-migration.md` (the method; the double-free trap).

### What the audit found (2026-10-15, `rg` over `smc/src`, 67.6K lines)

Counts first (the earlier "772 `.get()`" figure was a bad regex that matched CEGUI's `.getX()`;
the real number is below):

| idiom | count | verdict against the standard |
|---|---|---|
| `std::unique_ptr` mentions | 157 | storage is owned — good |
| `std::make_unique` | 32 | only `main.cpp` (25) and `savegame.cpp` (7) use it |
| raw `new T` (non-CEGUI) | ~150 | `Add(new X)` / `= new X` feeding raw-adopting APIs — **R.11/R.23 violated at every site** |
| `new CEGUI::…` / `createWindow` | ~440 | CEGUI owns these (its API contract) — leave |
| `delete` statements | 156 | ~110 are *owners* that should not exist (members, transient lists); ~15 are CEGUI's protocol; the rest are error-path frees of a just-`new`ed object |
| `.release()` | 11 | all are `Copy().release()` → `Add(raw)` or `Add(x.release())` — the transfer is forced back to raw because `Add` takes `T*` |
| `.get()` (real) | ~80 | 24 are the `pX` alias setup in `main.cpp` (fine); ~20 are `static_cast<cX*>(sprite.get())` tag-downcasts (fine, that's task 7's problem); ~15 are the `cSprite* sprite = sprite_ptr.get();` **loop prelude** (unnecessary — use the handle); the rest are legitimate borrows (`return x.get()`, comparisons, passing a `T*`) |
| `for (auto& x : …)` / `for (const auto& x : …)` | 134 / 18 | observation loops default to mutable — the standard says `const auto&` unless the handle is mutated |
| `for (auto x : …)` | 49 | all over vectors of raw pointers → cheap copies, fine as is |
| non-loop `auto` | 8 | types are spelled everywhere, including iterators (`typename OwningList::iterator itr = std::find_if(…)`) — the standard wants `auto` there |
| `enum class` / `constexpr` / `[[nodiscard]]` / `noexcept` / `std::optional` | 0 each | enums are task 7; `#define` (131) is a deferred step; the rest are small wins listed below |
| `NULL` | 87 | all inside `/* = NULL */` default-argument comments — cosmetic |

### The specific smells, by standard rule

1. **`cObject_Manager<T>::Add(T*)` adopts a raw pointer** (`core/obj_manager.h`; overridden by
   `cSprite_Manager`, `cWorld_Sprite_Manager`, `cImage_Manager`, `cSound_Manager`,
   `cAnimation_Manager`, `cObjectCollisionType`; the same shape in `cRenderQueue::Add`). The
   standard (F.26, TotW #187): pass `unique_ptr<T>` **by value** to transfer ownership. Because
   `Add` takes raw, every creator writes `Add(new X(…))` and every `Copy()` caller writes
   `Add(sprite->Copy().release())` — the eleven `.release()`s exist only to undo the `unique_ptr`
   the previous step added. Fix the signature and both go away: `Add(std::make_unique<X>(…))`,
   `Add(sprite->Copy())`.
2. **`Copy()` bodies still `new`:** `cFurball* furball = new cFurball(…); …; return
   std::unique_ptr<cSprite>(furball);` ×34. Standard: `auto furball = std::make_unique<cFurball>(…);
   …; return furball;` (the derived → base conversion is implicit on return). Also `[[nodiscard]]`
   on `Copy()`.
3. **Owning members still raw with hand-written destructors** (rule of zero, C.20/C.21): `cLevel`
   (`m_sprite_manager`, `m_animation_manager`, `m_background_manager`), `cOverworld` (six members),
   `cMenuHandler`/`cMenuCore`/`cMenu_Item` (eight), the three `m_camera`s
   (`cLevel_Manager`, `cLevel_Settings`, `cOverworld_Manager`), `cEditor_Level::m_settings_screen`,
   `cOverworld_Player`'s three debug sprites, `cLevel_Entry`/`cLevel_Exit::m_editor_entry_name`
   (re-created in `Set_Name`/`Set_Entry`), `cMouseCursor::m_hovering_object` +
   `cCopyObject::m_obj` + the `m_copy_objects`/`m_selected_objects` raw-owning vectors,
   `cCampaign_Manager::m_campaign` + its XML handler, `cSavegame::m_savegame` + its XML handler,
   `cItemBox::m_item`, `cPlayerPoints::m_points_objects`, `cHud_Manager`-side `m_sprites`,
   the animation `m_objects` lists (`cAnimation_Goldpiece`, `cAnimation_Fireball`,
   `cParticle_Emitter`), `cFramerate::m_perf_timer`, `cFont_Manager::m_software_textures`,
   `cImage_Manager::m_saved_textures`, `cAudio::m_active_sounds`, `cBonusBox::m_active_items`,
   `cEditor`'s `m_tagged_item_objects`/`m_tagged_item_images` + `cEditor_Item_Object::sprite_obj`,
   `cMenu_Base::m_draw_list`, `cMenu_Savegames::m_savegame_temp`, and **`cCollidingSprite::
   m_collisions`** (a raw-owning `vector<cObjectCollision*>` — the one owner the step-5 migration
   skipped, which is why `Add_Collisions` needs `col.release()` today).
4. **Transient owners deleted on every exit path:** `cObjectCollisionType* col_list =
   Collision_Check…(…); … delete col_list;` ×~30 (`level_player.cpp` alone has 14, with paired
   `delete col_list; return;` early exits), plus `cSave* savegame = pSavegame->Load(…); … delete
   savegame;`, the XML `loader`s, `cImage_Settings_Data* settings`, `temp_parser`, the dialog
   `box`es, `surface_copy`/`position_info`/`surface_temp` (with the `m_auto_del_img = false;
   delete` dance), and three raw arrays (`new GLubyte[]`, `new png_byte[]`, `new std::string[]`).
   Standard (R.5, F.20): return `unique_ptr` (or a value) from the factory and let scope end it;
   `std::vector` for the arrays.
5. **Conditional ownership flags:** `cSprite::m_image` + `bool m_delete_image`, `cGL_Surface::
   m_auto_del_img`, `cObject_Manager::Delete(obj, delete_data)` (whose `else if (delete_data)
   delete obj;` frees a pointer the manager never owned). The standard's R.3 says a raw pointer
   never owns; a flag that makes it *sometimes* own is the hardest shape to reason about. See
   open question 1.
6. **Application teardown is still hand-ordered** (`Exit_Game`: 24 `reset()` + alias-null pairs,
   with the CEGUI system/renderer destroyed in the middle). The step-6 task kept the original
   order deliberately; the standard's RAII rule says destruction order is the *declaration*
   order's job. Fix without changing behaviour: declare the `cApplication` members in the
   **reverse of today's `Exit_Game` order** so `g_app.reset()` reproduces it, and give the CEGUI
   system/renderer/resource-provider/logger an owner (`cVideo`, which creates them — today
   `~cVideo` is empty and `main.cpp` destroys what `Init_CEGUI` made). The `pX` borrowed aliases
   stay (thousands of sites; a raw borrow is standard-conformant) and are nulled in one place.
7. **Observation loops are mutable by default** (`for (auto& sprite : objects)` ×134 where the body
   only reads through the handle) and **iterator types are spelled out**. Standard: `const auto&`
   unless the handle itself is reset/moved/assigned; `auto` for iterators and other
   right-hand-obvious types; and the `cSprite* sprite = sprite_ptr.get();` prelude inside loops
   goes — `sprite->` works on the `unique_ptr` handle and `.get()` is written only where a raw
   `cSprite*` is stored or compared.
8. **One sentinel return:** `cObject_Manager::Get_Array_Num` returns `-1` for "not found" (an
   `int` for a `size_t` index). Standard: `std::optional<std::size_t>`.
9. Small: `[[nodiscard]]` on the factories (`Copy`, `Collision_Check*`, `Load`/`New`);
   `/* = NULL */` → `/* = nullptr */` (87 comments); the error strategy is undocumented — the game
   uses exceptions only around init/CEGUI and `nullptr`/`bool` returns elsewhere, and `CLAUDE.md`
   should say so, per the standard.

### Was `unique_ptr` the right choice in September? (the maintainer's question, 2026-10-15)

Mostly yes, with one correction. (`std::auto_ptr` is not used anywhere — it was removed in
C++17 and the tree is C++23; the question is `unique_ptr` vs the alternatives.)

- **Right:** the manager storage (`vector<unique_ptr<cSprite>>` etc.), the render queue, the
  savegame graph, `Copy()`'s return. Those hold *polymorphic* objects with exactly one owner that
  are created by factories and moved between owners — the textbook `unique_ptr` case. `shared_ptr`
  would have been wrong: nothing shares lifetime, the hundreds of borrowers (`pX`, `cSprite_List`,
  collision lists) would have become `weak_ptr` churn, and every copy would pay an atomic. Values
  are impossible for a heterogeneous polymorphic container.
- **Over-applied:** the non-polymorphic, always-present subobjects (`cLevel::m_sprite_manager`,
  the cameras, the menu handler's level/player, the HUD sprites) are `new`ed in a constructor and
  `delete`d in a destructor — they should be **value members** (smell 3, phase D). The
  `cApplication` members are the borderline case: non-polymorphic, but constructed in three
  dependency stages with arguments that exist only mid-`Init_Game`, so delayed construction is
  real. `unique_ptr` is defensible there (TotW #187's "large object needed conditionally"
  exception); `std::optional<T>` + `emplace` would be the pure-value spelling and is a reasonable
  phase-E option if the aliases become `&*m_x`.
- **Incomplete, not wrong:** stopping the transfer at `Add(T*)` and leaving `m_collisions` raw is
  what produced the `.release()` and `.get()` noise — the smart pointer was placed at the storage
  but not made the protocol. That is what phases A–C fix.

### What this task deliberately does NOT touch (and why)

- The 24 `extern cX* pX` aliases: borrowed raw pointers are what the standard prescribes for
  non-owning access; replacing them with accessors is a different, much larger refactor.
- CEGUI allocations (`new CEGUI::ListboxTextItem` handed to `addItem`, the `rp`/`logger`
  pattern): CEGUI's documented ownership contract — an externally-defined protocol.
- `cSprite_List`/`cObjectCollision_List` as raw views, and the `static_cast<cX*>(sprite.get())`
  tag downcasts: the views are correct; the downcasts are the type-tag dispatch of
  `tasks/smc-modernize-step-7-enums-dispatch.md`.
- `enum class`, `int_to_string`/`std::format`, enemy dedup, CEGUI lambdas: their own parked tasks.
- `#define` → `constexpr` (131): a separate mechanical pass; most string macros are used in
  literal concatenation and must stay (see open question 3).

## Goal

A tree where reading a signature tells you who owns what: `Add(std::unique_ptr<T>)` means "I take
it", `T*`/`T&` means "I borrow it", a `unique_ptr` return means "you own it now", and no destructor
in `smc/src` hand-`delete`s a member. Every change compiles against the existing call sites or
fails loudly (compiler-driven), and the headless smoke plus one AddressSanitizer run prove no
double-free crept in.

## Plan (ordered by safety; each phase = one or a few commits, gate green after each)

- [ ] **A. Transfer ownership through `Add`.** `cObject_Manager<T>::Add(std::unique_ptr<T>)` (+ the
      six overrides) and `cRenderQueue::Add(std::unique_ptr<cRender_Request>)`. Compiler-driven:
      every `Add(new X(…))` → `Add(std::make_unique<X>(…))`; every `Add(x.release())` →
      `Add(std::move(x))`; the 60 render-request sites → `auto request =
      std::make_unique<cRect_Request>(); pVideo->Draw_Rect(…, request.get()); pRenderer->Add(
      std::move(request));`. Where a creator keeps using the object after adding it (the
      `Set_Pos_Z`-then-use shape), take the borrowed `T*` back from `Add` (make `Add` return `T*`)
      so no caller needs a pre-`release()` raw copy. Delete the eleven `.release()`s. Codemod the
      bulk (`tasks/adhoc/apply-cpp-standard/`), hand-fix the residue the compiler flags.
- [ ] **B. `Copy()` bodies → `make_unique`, `[[nodiscard]]` on `Copy`.** 34 sites, regex-able
      (`cX* v = new cX(args);` → `auto v = std::make_unique<cX>(args);`, `return
      std::unique_ptr<cSprite>(v);` → `return v;`).
- [ ] **C. Transient owners → `unique_ptr` returns.** `Collision_Check`, `Collision_Check_Relative/
      Absolute`, `Col_Move_in_Steps` return `std::unique_ptr<cObjectCollisionType>`; delete every
      `delete col_list` (the paired early-exit deletes collapse). `cSavegame::Load` returns
      `unique_ptr<cSave>`; the XML handlers and `cImage_Settings_Parser` become stack objects; the
      dialog boxes become stack objects; the three raw arrays become `std::vector`. Then
      `cCollidingSprite::m_collisions` → `vector<unique_ptr<cObjectCollision>>` with
      `Create_Collision_Object` returning `unique_ptr` and `Add_Collision(unique_ptr)` — this
      retires the `release()` in `Add_Collisions` and the five `delete collision` sites.
- [ ] **D. Owning members → values first, `unique_ptr` second, rule of zero.** The list in smell
      3, one owner (or one class) per commit. **Per the standard's smart-pointer table (`R.5`,
      EMC++ 18): a subobject constructed unconditionally with its owner, non-polymorphic and never
      replaced, becomes a plain value member** — `cLevel`'s three managers, `cOverworld`'s sprite/
      animation managers, description and layer, the three `m_camera`s, `cMenuHandler`'s camera/
      level/player, `cFramerate`'s timers (a `std::vector<cPerformance_Timer>`), the HUD's fixed
      sprites. Call sites that pass `level->m_sprite_manager` as a `cSprite_Manager*` become
      `&level->m_sprite_manager` (compiler-driven). `unique_ptr` stays only where it earns it:
      polymorphic (`cMenu_Base* m_menu_data`, the enemy/anim object lists), optional or re-created
      (`m_editor_entry_name`, `m_hovering_object`, `cCampaign_Manager::m_campaign`,
      `cSavegame::m_savegame`), or moved between owners. Each destructor that only `delete`s
      becomes `= default` or disappears. Decision point for the conditional-ownership flags (open
      question 1).
- [ ] **E. RAII teardown.** Reorder `cApplication`'s members to reproduce today's `Exit_Game`
      order in reverse-declaration destruction; move CEGUI system/renderer/rp/logger ownership into
      `cVideo`; `Exit_Game` shrinks to save-prefs, unload, null-the-aliases, `g_app.reset()`.
      **Play-test gate:** quit from menu, from a level, from the editor (teardown is invisible to
      the SIGKILL smoke).
- [ ] **F. Loops and `auto`.** `for (auto& x : c)` → `for (const auto& x : c)` tree-wide, then
      revert exactly the loops the compiler rejects (they mutate the handle); drop the
      `cSprite* sprite = sprite_ptr.get();` preludes; `auto` for the spelled-out iterator locals;
      `std::optional<std::size_t>` for `Get_Array_Num`; `/* = NULL */` → `/* = nullptr */`;
      split `cObject_Manager::Delete(obj, delete_data)` into `Delete(T*)` and `std::unique_ptr<T>
      Release(T*)` and drop the free-what-we-never-owned branch.
- [ ] **G. One AddressSanitizer smoke.** Build once in a throwaway container with
      `CXXFLAGS="-fsanitize=address -fno-omit-frame-pointer"` (the sandbox ships `libasan`) and run
      `tools/smoke_headless.sh` plus a scripted level load; a clean run is the double-free proof
      the step-5 migration had to get from gdb after the fact. Not a permanent flag (open
      question 2).
- [ ] **H. Docs.** `CLAUDE.md`: one line pointing at the shared standard plus the error strategy
      ("exceptions around init/CEGUI only; `nullptr`/`bool` returns elsewhere"); re-sync
      `tasks/reference/smc/ownership-and-lifetime.md` (its SUPERSEDED banner still says "not yet
      merged") and the `.get()`/`Copy()` bullets in `refactoring-to-modern-cpp.md`; stage.
- [ ] **I. (after the maintainer's go-ahead on the result)** Harvest into the shared
      `cpp-coding-standard.md` + its two mirrors: what this execution teaches that the standard
      does not yet say (candidates: "the smart pointer must be the *protocol*, not the edge — an
      `Add(T*)` that adopts forces `release()` on every caller"; the `.get()` loop-prelude
      anti-pattern; the conditional-ownership-flag smell; `delete` on every return path as the
      tell for a `unique_ptr` return; the reverse-declaration trick for a preserved teardown order;
      the compile-driven `const auto&` pass). Its own small task in runClaudeInContainer.

## Notes / decisions

- Measure before and after, in this doc: `new` (non-CEGUI), `delete`, `.release()`, `.get()`,
  `make_unique`, `for (auto&` vs `for (const auto&`. The targets are 0 / ~15 (CEGUI only) / 0 /
  ~45 / everything owning / inverted.
- Workflow is the one that worked in September: a branch, distinct single-purpose commits that
  each build + smoke green, the maintainer play-tests and bisects at the end. Whether the agent
  may commit on the branch this time is open question 4.
- Phases A–C are compiler-driven and smoke-visible (a wrong transfer crashes at startup or on
  first collision); D and E are the ones whose failure mode is a leak or a teardown crash the
  smoke cannot see — that is why the ASan run (G) and the play-test are in the plan.

## Open questions

1. **Conditional ownership (`cSprite::m_image` + `m_delete_image`, `cGL_Surface::m_auto_del_img`):**
   (a) leave the flag-owned pairs as they are and document them as a known exception, or
   (b) split each into an owning `std::unique_ptr<cGL_Surface> m_owned_image` plus the existing
   raw `m_image` view (the view points at either the owned surface or the image manager's). I
   recommend (b) — it is the shape the standard prescribes and removes the `m_auto_del_img =
   false; delete` dance — but it touches `cSprite`, so it is the riskiest edit in phase D.
2. **AddressSanitizer:** one-off throwaway run only (my recommendation), or add a `USE_ASAN`
   build flag to the Makefile/Dockerfile so it stays runnable?
3. **`#define` → `constexpr` (131 macros):** include as an extra phase here, or leave it as the
   separate task the step-2 record deferred it to? I recommend leaving it separate.
4. **Commit authorization:** may I commit on a branch (`apply-cpp-standard`) per phase, as in the
   September run, or stage only and you commit?
