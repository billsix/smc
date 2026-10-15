# Bring the modernized tree in line with the shared C++ coding standard (ownership first)

**Status:** CODE-COMPLETE, STAGED — go-ahead 2026-10-15 (William Emerison Six <billsix@gmail.com>);
the four open questions were answered the same day (recorded under Notes / decisions). All phases
(A–I) done: A–F committed by the maintainer ("mid work"); E, G, H and the harvest (I) staged. The
final gate passed (full `make image` + smoke + codemod fixpoint). **Remaining before archive: the
maintainer's play-test of the teardown, then the maintainer's work commit; archive is its own commit
after that.** **Stage only — no agent commits**; the per-phase file ledger below lets the maintainer
split commits with `git add -p`.
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

## Ledger (per phase: what landed, files, numbers)

### Phase A + B — ownership transfer through the API (DONE 2026-10-15; build + smoke green)

- **Codemod** `tasks/adhoc/apply-cpp-standard/transfer_ownership.py` (idempotent on the finished
  tree; a `SKIP` set names the five sites deliberately left raw for phases C/D): 4 inline
  `Add(new …)`, 148 local-then-sink conversions, the 11 `create_request` draw blocks, the 31 XML
  factory `return new` → `make_unique`. **Hand script** `fix_wave1.py` (asserting, idempotent) for
  what the compiler then flagged: the 12 `Save_To_Savegame` signatures, `Create_Texture` and the
  two mouse-collision getters returning `unique_ptr`, the three "create the emitter if none was
  passed" blocks, multi-line `Draw_*(…, request)` arguments, `cCopyObject::m_obj`, the moving
  platform's reused request local, and the reverts listed below.
- **Hand edits:** `cObject_Manager<T>::Add(std::unique_ptr<T>)` returning the borrowed `T*`,
  `Delete(T*)` without the free-what-we-never-owned branch, a new `Release(T*)`; the six `Add`
  overrides and `cRenderQueue::Add`; `cCollidingSprite::m_collisions` → `vector<unique_ptr>` with
  `Create_Collision_Object` → `unique_ptr` and `Add_Collision(unique_ptr)` (the `release()` in
  `Add_Collisions` is gone); the two XML factories and the editors' `Get_Object` return
  `unique_ptr`; `cLayer::Add(unique_ptr)` + `Register(raw)` with the line's end point owned by its
  start point until registration (`m_end_point`); `cEditor_Item_Object::sprite_obj`,
  `cEditor::m_tagged_item_objects`, `cCampaign_XML_Handler::m_campaign` → `unique_ptr`;
  `Load_Campaign`/`Load_GL_Surface` → `unique_ptr`; the level/overworld tile-copy chains and the
  `cLevel_Manager`/`cAudio`/`cHud_Manager`/`cOverworld_Manager` creation sites; `[[nodiscard]]` on
  `Copy()` and `Save_To_Savegame()`.
- **Numbers (before → after):** `.release()` 11 → 1 (the documented `Render_Text` hand-off);
  `Copy().release()` 11 → 0; non-CEGUI `new` ~150 → 76 (the rest are phase C transients and phase D
  members); `delete` 156 → 105; `make_unique` 32 → 255; `std::unique_ptr<cSprite>(raw)` 34 → 1 (the
  documented `bonusbox` hand-off from `m_active_items`). `.get()` rose 80 → 180: ~100 are `request.get()`
  borrows into `Draw_*` and the remaining loop preludes, which phase F trims.
- **Left raw on purpose (next phases):** `cAudio::m_active_sounds`, `cMenuHandler::m_items`,
  `cGL_Surface::Copy`/`Get_Software_Texture` and `m_saved_textures`, `cBonusBox::m_active_items`,
  the menu `m_draw_list` HUD sprites, the animation `m_objects` lists, the mouse selection lists,
  `cSprite::m_image` + `m_delete_image` (phase D); the `col_list` collision transients (phase C).
- **Lessons for the harvest (phase I):** (1) an `Add(T*)` that adopts forces `release()` on every
  caller — change the sink, not the callers; (2) a `unique_ptr` to a class the header only
  forward-declares cannot be *destroyed* inline (the `Save_To_Savegame` default body had to move to
  the `.cpp`, and four forwarding overrides needed the include) — EMC++ Item 22 generalised beyond
  pimpl; (3) when a caller keeps using the object after the hand-off, borrow first
  (`T* raw = owned.get(); … Add(std::move(owned));`) or take the `T*` the sink returns; (4) a scanner
  that treats "use after `std::move`" as a hard stop and a *same-name redeclaration* as a scope end
  was what made a 148-site codemod safe — use-after-move compiles; (5) the "create one if none was
  passed" optional-out-param shape (`request`, `anim`) converts to an `owned_x` + raw view pair;
  (6) the nested `make image` bakes the tree at `COPY` time, so never edit source while it runs,
  and tag the last good intermediate layer (`podman tag <id> localhost/smc-base`) to iterate from
  the mount; `podman run` needs the `localhost/` prefix without a TTY.
- **Files:** `smc/src/core/{obj_manager.h,sprite_manager.{h,cpp},collision.{h,cpp},editor.{h,cpp},
  campaign_manager.{h,cpp}}`, `video/{renderer.{h,cpp},img_manager.{h,cpp},animation.{h,cpp},
  video.{h,cpp},gl_surface.cpp,font.cpp}`, `audio/{sound_manager.{h,cpp},audio.cpp,random_sound.cpp}`,
  `objects/*`, `enemies/*`, `level/{level.{h,cpp},level_manager.cpp,level_player.cpp,level_editor.{h,cpp},
  level_settings.cpp}`, `overworld/{overworld.{h,cpp},world_layer.{h,cpp},world_sprite_manager.{h,cpp},
  world_manager.cpp,world_editor.{h,cpp},world_player.cpp,world_waypoint.cpp}`, `gui/{hud.cpp,menu.cpp,
  menu_data.cpp,generic.cpp}`, `input/mouse.{h,cpp}`, `user/savegame.cpp`, `core/global_basic.h`
  (`<memory>`); plus `tasks/adhoc/apply-cpp-standard/{transfer_ownership.py,fix_wave1.py}`.

### Phase C — transient owners (DONE 2026-10-15; build + smoke green)

- **Codemod** `transient_owners.py` (idempotent): the four collision-list factories
  (`Collision_Check`, `_Relative`, `_Absolute`, `Col_Move_in_Steps`) return
  `std::unique_ptr<cObjectCollisionType>`; 22 call-site declarations converted, every `delete
  col_list` and `if (col_list) { delete col_list; }` removed, `Add_Collisions(col_list.get(), …)`.
- **Hand scripts** `fix_wave2.py`…`fix_wave5.py` (asserting, idempotent): `cSavegame::Load` and
  `Acquire_Savegame` return `unique_ptr<cSave>`, the XML handler owns its `m_savegame` and is a
  stack object, `Save_Game` builds with `make_unique`; `cImage_Settings_Parser::Get` returns
  `unique_ptr`, its `m_settings_temp` is owned, the nested base-settings parser is a stack object;
  `cVideo::cSoftware_Image` owns its settings (move-only; special members out of line);
  `cEditor::m_tagged_item_images` owns its entries; the two dialog boxes are stack objects; the
  screenshot buffer, the PNG row buffers and the line parser's `parts` are `std::vector`.
- **Numbers (after A → after C):** `delete` 105 → 55; non-CEGUI `new` 76 → 58; `make_unique`
  255 → 280.
- **Lessons for the harvest:** (7) **an inline constructor instantiates its members' destructors**
  (for unwinding), so a struct holding `unique_ptr<Incomplete>` must define *all* its special
  members out of line, not just the destructor — the `cSoftware_Image` fix took two rounds;
  (8) an "insert before anchor" script edit is not idempotent by the "old text absent" test;
  guard it on the inserted text instead (the duplicate-definition round).
- **Files:** `objects/movingsprite.{h,cpp}`, `level/level_player.cpp`, `enemies/{enemy,turtle,
  bosses/turtle_boss}.cpp`, `objects/{box,spinbox}.cpp`, `gui/{hud,menu_data,generic}.cpp`,
  `user/savegame.{h,cpp}`, `video/{img_settings.{h,cpp},video.{h,cpp}}`, `core/{editor.{h,cpp},
  file_parser.cpp}`; `tasks/adhoc/apply-cpp-standard/{transient_owners.py,fix_wave2.py,fix_wave3.py,
  fix_wave4.py,fix_wave5.py}`.

### Phase D (part 1 + 2) — owning members: values first, then `unique_ptr` vectors (DONE 2026-10-15; build + smoke green)

- **Values** (`fix_wave6.py` + `fix_wave7.py`): `cLevel`'s three managers; `cOverworld`'s sprite/
  animation managers, description, layer and two HUD names; `cMenuHandler`'s level, camera and
  player (declared level-first so the camera is built on it); `cMenuCore`'s handler and animation
  manager; `cMenu_Item`'s two images; `cItemBox::m_item`; the three `m_camera`s;
  `cEditor_Level::m_settings_screen`; `cOverworld_Player`'s three debug sprites;
  `cFramerate::m_perf_timer` as a vector of values. `cMenuCore::m_menu_data` → `unique_ptr`
  (polymorphic, replaced per menu). Access sites rewritten by rule (`X->m_camera->` →
  `X->m_camera.`, a pointer pass → `&X->m_camera`), then compiler-driven; the headers that only
  forward-declared the member types now include them (`cOverworld` ↔ `cWorld_Sprite_Manager`
  needed a forward declaration the other way).
- **`unique_ptr` vectors** (`fix_wave8.py` + `fix_wave9.py`): the HUD sprite lists, the points
  texts, the menus' draw lists and savegame slots, the three animation lists, the texture
  snapshots (`Get_Software_Texture` returns `unique_ptr`), the audio pool, the bonus-box items,
  the mouse copy/selection lists, the menu handler's items (`Auto_Menu` returns `unique_ptr`,
  `Add_Menu_Item` takes it and returns the borrowed pointer), the editor settings items and the
  editor list text. Delete loops → `clear()`; erase-with-delete → erase; by-value loops →
  `const auto&`.
- **Numbers (after C → after D2):** `delete` 55 → 16 (CEGUI's own protocol + the image/name
  surfaces of part 3); non-CEGUI `new` 58 → 5.
- **Lessons for the harvest:** (9) a value member needs the complete type in the header, so
  "value-first" moves includes up the graph — check for a cycle and break it with a forward
  declaration on the *pointer* side; (10) the two destructive scanner mistakes were both
  "same name, different owner" (`m_description` is a string in two classes and a value in one;
  `parent->m_sprite_manager` is a sprite's borrowed pointer) — scope every rewrite rule by owner
  expression, never by member name alone; (11) when the code keeps using the object after
  `push_back`, either borrow before the move or have the sink return the borrowed pointer.
- **Files:** `level/{level.{h,cpp},level_manager.{h,cpp},level_settings.{h,cpp},level_editor.{h,cpp}}`,
  `overworld/{overworld.{h,cpp},world_manager.{h,cpp},world_player.{h,cpp},world_sprite_manager.{h,cpp}}`,
  `gui/{menu.{h,cpp},menu_data.{h,cpp},hud.{h,cpp}}`, `core/{framerate.{h,cpp},game_core.cpp,editor.{h,cpp}}`,
  `video/{animation.{h,cpp},font.cpp,img_manager.{h,cpp},gl_surface.{h,cpp}}`, `audio/audio.{h,cpp}`,
  `objects/{bonusbox.{h,cpp},sprite.{h,cpp}}`, `input/mouse.{h,cpp}` and every file that reached a
  converted member; `tasks/adhoc/apply-cpp-standard/fix_wave{6,7,8,9}.py`.

### Phase D (part 3) — the conditional-ownership pairs (DONE 2026-10-15; build + smoke green)

- `fix_wave10.py` + `fix_wave11.py`: `cSprite` owns an image through `m_owned_image`
  (`unique_ptr`) with `m_image` as the raw view; `Set_Image(cGL_Surface*, bool)` borrows and the new
  `Set_Image(std::unique_ptr<cGL_Surface>, bool)` takes ownership — the `del_img` flag is gone from
  `cSprite`, `cMovingSprite` (`using cSprite::Set_Image;` keeps the overload visible) and
  `cAnimated_Sprite::Set_Image_Num`; `cFont_Manager::Render_Text` returns `unique_ptr` (the
  manager keeps only a borrowed reference that the surface's destructor removes), so every
  `Set_Image(pFont->Render_Text(…), x, true)` became `Set_Image(pFont->Render_Text(…), x)` and
  overload resolution picks the owning one; the two surfaces shared between a menu item (borrows)
  and a HUD sprite (owns) are explicit; `cLevel_Entry/Exit::m_editor_entry_name` are `unique_ptr`;
  `cMouseCursor::m_hovering_object` is a value.
- **Decision 1(b) applied.** `cGL_Surface::m_auto_del_img` stays: it guards the GL *texture
  handle* (a C resource shared between surface copies), not C++ object ownership; the standard's
  answer there is a custom-deleter `unique_ptr` for `GLuint`, left for a later pass.
- **Numbers (after D2 → after D3):** `delete` 16 → 5 (the CEGUI fake system, the CEGUI logger /
  resource provider at exit, and the editor's CEGUI texture — CEGUI's own protocol); non-CEGUI
  `new` 5 → 1 (`cGL_Surface::Copy`, which has no caller — dead code for the dead-code task);
  `.release()` 0.
- **Lesson for the harvest:** (12) an overriding class *hides* the base's other overloads, so
  adding an owning `Set_Image(unique_ptr)` overload to the base needs `using Base::Set_Image;` in
  every class that overrides the borrowing one (EC++ Item 33).

### Phase F — observation loops take `const auto&`; the preludes go (DONE 2026-10-15; build + smoke green)

- `const_loops.py apply` turned every `for (auto& x : …)` into `for (const auto& x : …)` (115
  loops), then `const_loops.py revert <build-log>` used the compiler as the oracle: each loop the
  build rejected for mutating through the const handle (`discards qualifiers`, `read-only`,
  `binding reference`, `use of deleted function` from moving a const handle) was turned back to
  `auto&`. Six loops genuinely mutate and stayed `auto&` (`framerate.cpp:154`,
  `sprite_manager.cpp:47`, `animated_sprite.cpp:128`, `video.cpp:1922`, `sprite.cpp:118`,
  `savegame.cpp:286`).
- `fix_wave12.py` (prelude half) removed the 8 `for (const auto& x_ptr : v) { T* x = x_ptr.get();`
  preludes in `sprite_manager.cpp`, `savegame.cpp` and `level_player.cpp`, so the loop variable is
  the owning handle and raw-pointer uses spell `.get()` at the point of use. The one-line borrowed
  copy `Get_Objects_sorted` (`new_objects.push_back(sprite_ptr.get())`) is left as is — it is a
  direct `.get()`, not a prelude.
- Also in this wave: 60 `/* = NULL */` → `/* = nullptr */`, 20 iterator declarations → `auto`, 22
  `[[nodiscard]]` on the `unique_ptr`-returning factories.
- **Numbers (after F):** `for (const auto&` 18 → ~170; `.get()` ~80 (every one now a deliberate
  borrow at the use site, not a prelude alias).
- **Deviation recorded as debt:** `cObject_Manager::Get_Array_Num`'s `-1` sentinel stays. 48 call
  sites index the object arrays through the `m_number` / `m_line_number` protocol and expect the
  sentinel; converting it to `std::optional` is its own task, not this one.

### Phase E — one owner for the process singletons; teardown by declaration order (DONE 2026-10-15; build + smoke green)

- `fix_wave12.py` (application half): `cApplication`'s 24 `std::unique_ptr<cX>` slots became
  `cOwned<cX>` members. `cOwned<T>` holds the `unique_ptr` plus a reference to the `extern cX* pX`
  alias every call site reads; `Create(args…)` makes the object and points the alias at it, and the
  destructor resets the pointer and nulls the alias, so no alias ever dangles.
- **The teardown order is preserved exactly, by construction.** Members die in reverse declaration
  order, so the members are declared in the *reverse* of the order the old hand-written `Exit_Game`
  destroyed them (audio first … resource manager last). The CEGUI system, which `Exit_Game`
  destroyed between the framerate and the video subsystem, now dies in `~cVideo` (`video.cpp`,
  previously `= default`).
- `Init_Game`'s 24 `g_app->m_x = std::make_unique<cX>(args); pX = g_app->m_x.get();` pairs collapsed
  to `g_app->m_x.Create(args);`. `Exit_Game` shrank from the 24 hand-ordered `reset()` + `= nullptr`
  pairs to: save preferences, `pLevel_Manager->Unload()`, `pMenuCore->m_handler.m_level.Unload()`,
  `g_app.reset()` (everything dies here), print any SDL error, `IMG_Quit(); SDL_Quit();`.
- **Lessons for the harvest:** (13) a single owning aggregate plus reverse-declaration-order
  destruction reproduces a hand-ordered teardown with zero ordering code — declare in reverse of
  the desired death order and let `~T` do it; (14) when a borrowed global alias must stay valid for
  legacy call sites, bind it to the owner (a small RAII `cOwned` wrapper) so creation sets it and
  destruction clears it in lock-step, instead of hand-pairing `make_unique`/`get`/`reset`/`=
  nullptr`; (15) `re.sub`'s replacement string treats `\n` as an escape, so a C `"%s\n"` in a
  replacement silently becomes a real newline inside the string literal — pass a callable (or
  double-escape) when the replacement contains backslashes.
- **Caveat — not yet play-tested.** The smoke run proves the process starts, runs 30 s and exits
  clean with no leaked child, but it never quits from a menu/level/editor the way a player does.
  Confirming a clean quit from each of those is the maintainer's gate (listed under next steps).

### Phase G — one-off AddressSanitizer run (DONE 2026-10-15; clean on both running sessions)

- Throwaway, no committed build flag (`tasks/adhoc/apply-cpp-standard/` is not involved; the driver
  was a scratch script). In a disposable container: `dnf install -y libasan` (the base image ships
  CEGUI but not `libasan`, so a stock `-fsanitize=address` link fails with "cannot find
  libasan.so"), copy the tree to container-tmp, `./configure --prefix=/usr/local
  CXXFLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O1" LDFLAGS=-fsanitize=address`,
  `make -j`, `make install` (data must be installed or CEGUI scheme loading aborts the run), then
  the two headless smoke scenarios with `ASAN_OPTIONS=detect_leaks=0:abort_on_error=1`.
- **Result: both scenarios (menu and `--level cave_1`) ran the full 30 s, ps audit clean, zero
  AddressSanitizer reports.** The ownership changes introduce no heap-use-after-free or
  heap-buffer-overflow in a running session.
- **Caveat — ASan did not exercise the teardown.** `smoke_headless.sh` hard-kills the process at
  the timeout (`timeout -s KILL`), so the game never reaches `Exit_Game` / `~cApplication`. ASan
  therefore validates the running session, not the Phase E reverse-declaration-order destruction.
  That path is the maintainer's play-test (quit from menu/level/editor), listed under next steps.

### Phase I — harvest into the shared C++ docs (DONE 2026-10-15; staged in both repos)

- **Home chosen: `cpp-ownership-migration.md`, not `cpp-coding-standard.md`.** The coding standard
  already carried the ownership *rules* (value-vs-`unique_ptr`, `.get()` borrowing, `auto_ptr`
  removed, Pimpl/incomplete-type, ownership-through-the-API, `using Base::` overload hiding) — I
  wrote them there while first studying smc, so re-adding them would duplicate. What this pass
  earned is *method*, which belongs in the migration doc that already exists from the September pass.
- Added a "The second pass — applying the coding standard (don't over-apply `unique_ptr`)" section:
  the `.get()`-everywhere smell → value member; the three patterns (conditional-ownership flag →
  owning member + raw view + two overloads with `using Base::`; a manager that borrows what it hands
  out; N global singletons → one owning aggregate destroyed in reverse declaration order with an
  alias-binding wrapper, CEGUI teardown into `~cVideo`); the compiler-as-oracle const-loop pass; the
  `re.sub` replacement-escape codemod gotcha; and the "a `-1` sentinel wants `optional` but that is
  its own task" scope note. Also flagged the teardown-not-smoke-verifiable caveat in the doc itself.
- Propagated byte-identical to all three copies (`runClaudeInContainer/tasks/reference/`,
  `runCrushInContainer/tasks/reference/`, `runCrushInContainer/client/entrypoint/dotfiles/.config/crush/reference/`;
  `md5sum` equal), staged by path in both repos.

### Phase H — docs (DONE 2026-10-15; staged)

- smc `CLAUDE.md`: fixed the `.clang-format` path (it is `smc/.clang-format`, the doc had a doubled
  `smc/smc/`), added a bullet pointing at the shared `cpp-coding-standard.md`, summarizing what the
  standard pass changed (ownership through the APIs, value-vs-`unique_ptr`, `cOwned<T>` reverse-order
  teardown), and stating the **error strategy** (exceptions only around startup/CEGUI; `nullptr` /
  `bool` returns elsewhere).
- `tasks/reference/smc/ownership-and-lifetime.md`: the SUPERSEDED banner now records both passes
  (the `modernize-cpp-3` branch is merged, not "not yet merged") and points at the task + migration
  doc. `tasks/reference/refactoring-to-modern-cpp.md`: a forward-pointer note that the second pass
  refined its recorded `unique_ptr` shapes (some → values, singletons → `cOwned<T>`).
- **Owed, not done here:** a full rewrite of `ownership-and-lifetime.md`'s L1/L2 body (it still
  describes raw pointers) — that is the pre-existing "re-sync when the branch merges" job, larger
  than this task and partly predating it; flagged in the banner rather than done.

### Final gate (DONE 2026-10-15)

- **Full nested `make image` from a clean tree:** CEGUI 0.7.9 and the game both compiled and
  installed, image committed as `localhost/smc:latest`, exit 0.
- **Smoke against the freshly built image:** both scenarios (menu, `--level cave_1`) exit 137
  (alive at the 30 s deadline), ps audit clean.
- **Codemods at a fixpoint:** re-running `fix_wave12.py` and `transfer_ownership.py` left
  `git diff -- smc/src` empty — the committed source is the scripts' fixed point.
- **End-state metrics (tracked `.cpp`/`.h`):** `.release()` 0; `make_unique` 276; observation loops
  161 `const auto&` vs 6 `auto&` that genuinely mutate; `.get()` 178, each a deliberate borrow at a
  use site. The carefully filtered non-CEGUI `new`/`delete` counts are in the per-phase entries
  above (a bare grep overcounts both — it catches comment prose and CEGUI's own `new`/`delete`
  protocol); the remaining `delete`s are CEGUI's fake-system / logger / editor-texture sites.

### Still open (next steps)

- **Play-test — maintainer only, required to close.** Quit from a menu, a level and the editor to
  verify the Phase E reverse-order teardown; the smoke and ASan hard-kill before `Exit_Game`.
- **Full L1/L2 body rewrite of `tasks/reference/smc/ownership-and-lifetime.md` — optional, own
  task.** Flagged in its banner; predates this task.
- **Play-test — maintainer only.** Quit from a menu, a level and the editor to verify the Phase E
  teardown; the smoke and ASan cannot (they hard-kill before `Exit_Game`).

## Notes / decisions

- Measure before and after, in this doc: `new` (non-CEGUI), `delete`, `.release()`, `.get()`,
  `make_unique`, `for (auto&` vs `for (const auto&`. The targets are 0 / ~15 (CEGUI only) / 0 /
  ~45 / everything owning / inverted.
- **Decisions (William Emerison Six <billsix@gmail.com>, 2026-10-15):** (1) split the
  conditional-ownership pairs into an owning `unique_ptr` + raw view; (2) AddressSanitizer is a
  one-off throwaway run, no build flag; (3) `#define` → `constexpr` stays a separate task;
  (4) **stage only, never commit — and that is the standing default for every task, not a
  per-task choice** (the September "commit yourself" grant was a one-off). Each phase below ends
  with `git add` by path and a ledger line here of the files it touched, so the maintainer can
  commit per phase from the index if they want bisectable history.
- Per-phase gate: the game rebuilt from the mounted tree (`build.sh game` in the `smc` image)
  plus `tools/smoke_headless.sh` in the same container; the full nested `make image` once at the
  end (CEGUI is unchanged, so the incremental game build is the honest per-phase check).
- Phases A–C are compiler-driven and smoke-visible (a wrong transfer crashes at startup or on
  first collision); D and E are the ones whose failure mode is a leak or a teardown crash the
  smoke cannot see — that is why the ASan run (G) and the play-test are in the plan.

## Open questions

None open — all four below were answered 2026-10-15 (see Decisions): 1 → (b), 2 → throwaway run,
3 → separate task, 4 → stage only.

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
