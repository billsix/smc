# Modernize step 5 — ownership: raw owning pointers → `unique_ptr` / value

**Status:** in-progress — branch `modernize-cpp-2` (off `master`), William Emerison Six
<billsix@gmail.com>, 2026-09-24
**Priority:** 4 · **Difficulty:** 8
**Part of:** `smc-modernize-cpp.md` (umbrella step 5)
**Depends on:** steps 0–4 (merged to master). Gate: `make image` + `tools/smoke_headless.sh`.

## BLUF

Replace hand-managed owning `new`/`delete` with `unique_ptr` (or values), one **distinct,
bisectable commit per owner** — the maintainer play-tests the whole branch at the end and bisects
if a regression shows, so each commit must build, link, and pass the headless smoke on its own.
Done = no owner in the touched areas hand-`delete`s, the game still builds+smokes green, and the
two `cObject_Manager` bugs are gone by construction. Grounded in
`tasks/reference/smc/ownership-and-lifetime.md`.

## Context

- The verified ownership map is `tasks/reference/smc/ownership-and-lifetime.md`. Key owners:
  `cObject_Manager<T>` (public `vector<T*> objects`, read at ~63 sites) and its 13 subclasses; the
  render queue's per-frame `new c*_Request`; the ~40 `virtual cX* Copy() const` sprite clones; the
  savegame pointer graph. The 24 process-global singletons are the SEPARATE next step (step 6,
  `cApplication`) — not here.
- **cSprite_Manager caveat:** it has custom lifetime (`m_disallow_managed_delete` for the player;
  `m_auto_destroy` lazy-free-on-next-Add; the z-order epsilon scheme). Converting its storage to
  `unique_ptr` changes *when* a sprite is freed, so that owner needs its own careful commit and is
  the highest gameplay risk in this step.

## Plan (each item = its own commit; build + smoke green before moving on)

- [x] **5.1 Render queue owns its requests** (`video/renderer.{h,cpp}`): `RenderList` →
  `vector<unique_ptr<cRender_Request>>`; `Add` adopts the raw pointer (so the 61 `Add(new c*_Request)`
  call sites are untouched); `Clear` frees by erase (`std::erase_if` / `clear`), no hand-`delete`;
  `zpos_sort` + the render/fake-render loops updated. Self-contained. Build + smoke green.
- [x] **5.2a `cLayer` un-derived from `cObject_Manager`** (world_layer.{h,cpp}): `cLayer` now holds
  a plain non-owning `LayerLineList objects` (matches today — the sprite manager owns the line
  points; `cLayer::Destroy`→`Delete(this,false)` and `Delete_All` are remove-without-free). This
  removes the double-ownership blocker so the base flip can make everything else `unique_ptr`.
  Behavior-identical (`objects` stays `vector<raw*>`); build + smoke green. Confirmed
  `cWorld_Sprite_Manager::Add` also feeds the SAME pointer to the sprite manager (owner) and the
  layer (borrower).
- [x] **5.2/5.3 `cObject_Manager<T>` base → `vector<unique_ptr<T>>`** (done together — the 12
  subclasses share the one typed member, so the flip is atomic; 28 files). Base: `Add` adopts
  (`emplace_back(raw)`), `Delete` erases (frees) or `release()`s when `delete_data=false`,
  `Delete_All`=`clear()`, `Get_Pointer` returns `.get()`, `Get_Array_Num` `find_if` over
  `unique_ptr`. `cSprite_Manager`: auto-destroy `reset()`, `Move_To_Front/Back` move-out/erase/
  reinsert, `m_disallow_managed_delete` `release()` before erase, `Get_Objects_sorted` `.get()`
  copy. `cObjectCollisionType` predicates → lambdas (retires `binary_function`/`bind2nd`, gone in
  C++17). Collision-vector transfers (movingsprite) → `make_move_iterator`; the
  `Collision_Check(&mgr->objects)` alias → a lambda called from two loops. `cLayer` was un-derived
  in 5.2a. ~95 read loops mechanically `auto&`'d (codemod `fix_objects_loops.py`), residue `.get()`
  by hand (compiler-driven). Compiles + links; smoke green.
  - **Runtime double-free found by the smoke + gdb (fixed):** `Add_Collisions` MOVES collisions
    from the temporary `col_list` into the sprite's raw-owning `m_collisions`, then
    `col_list->objects.clear()`. With `.get()` the borrowed pointer went into `m_collisions` (which
    frees it) AND `clear()` freed it too. Fix: `col.release()` — transfer ownership. **Lesson: a
    container-to-container move that relied on a raw `vector::clear()` NOT freeing must become an
    explicit `release()`/`move` when the source becomes owning.** (harness: `debug_doublefree.sh`.)
- [x] **5.4 `Copy()` returns `std::unique_ptr<cSprite>`** (branch `modernize-cpp-3` off the merged
  master; 69 files). The 34 cSprite-hierarchy `Copy()` signatures (impl + decl) rewritten by
  `fix_copy_clone.py` (excludes `cGL_Surface`, a texture not a sprite); each body's `return <var>;`
  wrapped by `fix_copy_returns.py` (`return std::unique_ptr<cSprite>(var)`) — the local stays raw so
  no body use breaks, and the `new` is owned by the returned `unique_ptr` the instant it returns.
  The ~10 call sites `.release()` into the raw-adopting `Add`/save vectors (ownership flow
  unchanged). Kept the name `Copy` (rename to `Clone` is cosmetic and would collide with the
  descriptive-names task). Builds + smoke green. (A `make_unique` polish inside the bodies is
  optional follow-up; ownership is already expressed by the return type.)
- [x] **5.5 Savegame graph → `unique_ptr`** (savegame.{h,cpp}). The owning save lists
  (`Save_LevelList`, `Save_OverworldList`, `Save_Level_ObjectList`, `Save_Overworld_WaypointList`)
  and the two spawned-sprite lists became `vector<unique_ptr<…>>`; destructors → `= default`;
  `new cSave_*` → `make_unique` + `std::move` into the vectors; the parser→struct handoffs use
  `swap` (both are unique_ptr vectors now) and the spawned-objects load `release()`s each into the
  level's sprite manager. Builds + links; smoke green (smoke doesn't load a save, so save/load
  correctness is for the maintainer's play-test). NOTE: the shared `cSprite_List` alias stayed raw
  (it's a non-owning view elsewhere); the save's owning spawn lists use `vector<unique_ptr<cSprite>>`
  directly. **Nested-podman op note:** the RAM store (32G tmpfs) hit 100% from this session's many
  image rebuilds and wedged podman (prune couldn't write); recovered with
  `rm -rf /var/lib/containers/storage` (it's ephemeral) → full rebuild.
- [ ] 5.6 Remaining owning `new`/`delete` in the touched areas → `make_unique`.
- [x] **Harvest learnings DONE (2026-09-24):** wrote `cpp-ownership-migration.md` (new) + extended
  `print-debugging.md` (compiler-as-oracle for a type migration) + a pointer in
  `code-style-conventions.md` in `runClaudeInContainer/tasks/reference/`, and mirrored the two docs
  into `runCrushInContainer/tasks/reference/`. Staged in both repos (not committed — the maintainer
  commits those). Original note:
- [ ] **(reference) Harvest learnings to `~/.claude/reference/` (cross-project):**
  the "compiler as oracle" method for a large type migration (flip the shared type, fix the base +
  known hazards, then build-iterate on what the compiler flags — the refactor analogue of the
  instrumentation-driven / "tool as oracle" lesson in `print-debugging.md`); the mechanical-vs-
  ownership-hazard split (recon first); the clang-tidy warts on old code (loop-convert's
  container-named locals; use-equals-default botching copy ctors / changing copy-assign); and the
  raw→`unique_ptr` migration idioms (Add adopts via `emplace_back(raw)`; erase frees;
  remove-without-free = `release()` then erase; reorder = move-out/erase/reinsert; `std::erase_if`;
  `find_if` with a lambda over `unique_ptr&` + `.get()`; `binary_function`/`bind2nd` gone in C++17 →
  lambdas). **Write to the sandbox repos where these docs live (maintainer 2026-09-24):**
  canonical home is `runClaudeInContainer/tasks/reference/` (19 docs, mounted to
  `~/.claude/reference/`) — extend `print-debugging.md` with the compiler-as-oracle refactor method
  and add a new `cpp-ownership-migration.md` (+ a pointer line in `code-style-conventions.md`); then
  MIRROR into `runCrushInContainer/tasks/reference/` (it mirrors only a subset today — 15 docs, no
  `print-debugging.md` yet — but the maintainer wants these there too). Both repos are workable
  (runClaude origin = Pi; runCrush github = billsix).

## Notes / decisions

- **5.1 done** (`8ac4af3`): the queue no longer hand-deletes any request; ownership transfers
  into a `unique_ptr` at `Add` time. Requests that persist across frames (`m_render_count > 1`) are
  unaffected; typeless (`REND_NOTHING`) requests are still dropped immediately (now freed by the
  adopting `unique_ptr` on return, not a manual `delete`).

- **5.2 base conversion DEFERRED — it is not mechanical** (recon 2026-09-24, below). All 12
  subclasses share the one typed `cObject_Manager<T>::objects` member, so flipping it to
  `vector<unique_ptr<T>>` is atomic and pulls in genuine ownership hazards, not just iterator
  churn. **Do the range-for pass (umbrella step 8) FIRST** — it converts the ~95 read-only
  `->objects` loops to `auto&` range-for, which compile against either `T*` or `unique_ptr<T>`,
  shrinking the base flip to the hazards alone. Then resolve the hazards below.

### Recon: the `cObject_Manager` conversion surface (2026-09-24)

118 qualified `.objects`/`->objects` sites. ~95 are read-only iteration (category a), ~14 are
size/empty/clear (e). The dangerous minority:

- **Double-ownership (double-free risk).** `cLayer::Add` (world_layer.cpp:301,306,312) puts the
  same `cLayer_Line_Point_Start*` into BOTH `cLayer::objects` and `cSprite_Manager` (via
  `m_overworld->m_sprite_manager->Add`). Today `cLayer::Delete_All` is clear-only
  (world_layer.cpp:388-390) precisely so only `cSprite_Manager` frees them — i.e. **cSprite_Manager
  is the owner, cLayer borrows.** Under a `unique_ptr` base, `cLayer` would own+free them too.
  **Design decision needed:** make `cLayer` non-owning (a plain `vector<cLayer_Line_Point_Start*>`,
  not a `cObject_Manager`), or otherwise break the shared base. `cWorld_Sprite_Manager` also
  re-adds into `cSprite_Manager` (world_sprite_manager.cpp:30) — check its ownership the same way.
- **Reorder while holding a raw copy of an owned element (dangle).** `cSprite_Manager::Move_To_Front`
  / `Move_To_Back` (sprite_manager.cpp:102,119-121,133,150-152): `first = objects.front()` then
  `erase` (which under unique_ptr frees `first`) then `insert(first)`. Rewrite with `std::move` /
  `std::rotate`.
- **auto-destroy in-place swap** (sprite_manager.cpp:53-59): `*itr = sprite; delete obj;` — becomes
  a `unique_ptr` move-assign (the old is freed by the assignment).
- **`m_disallow_managed_delete` erase-without-free** (sprite_manager.cpp:172-183): the player is
  erased but not freed → must become `.release()`, not plain erase.
- **Whole-vector copy** `new_objects = objects;` (sprite_manager.cpp:267, Get_Objects_sorted) — a
  `vector<unique_ptr>` won't copy into a `cSprite_List` (`vector<cSprite*>`); becomes a `.get()`
  loop. `cSprite_List` and the other `*_List` aliases stay **raw** (they are non-owning views).
- **Address-of-vector alias** `objects = &m_sprite_manager->objects;` (movingsprite.cpp:615) — the
  local's type (`cSprite_List*`) no longer matches; rework the collision-check to iterate directly.
- **Move-transfer between two collision vectors** (movingsprite.cpp:221-224, 305-308): range-insert
  + clear between `cObjectCollisionType` vectors → move-iterators.
- **Get_Pointer return type** (base obj_manager.h:72): keep returning `T*` (via `.get()`) so
  `cImage_Manager`/`cSound_Manager` `Get_Pointer(size_t)` wrappers and `cSprite_Manager::operator[]`
  are unaffected.
- **Borrowed-pointer consumers** (non-owning, safe once the base returns/holds correctly): the
  editor/mouse selection lists (`m_selected_objects[i]->m_obj`, mouse.cpp), `Get_Colliding_Objects`
  / `Get_Objects_sorted` result lists, savegame's `Copy()` deep-copies. No manager pointer is freed
  by these; they just must not be treated as owners.
- **No `Delete(ptr, false)` callers** anywhere — the remove-without-free semantics live only in the
  `m_disallow_managed_delete` loop and `cLayer::Delete_All`.

## Open questions

None. (The line-point ownership question is resolved: `cLayer` is now non-owning — 5.2a — matching
the pre-existing runtime behavior, so no design change, just made explicit.)
