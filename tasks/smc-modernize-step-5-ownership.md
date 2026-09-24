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
- [ ] 5.2 `cObject_Manager<T>` base → `vector<unique_ptr<T>>` for the *simple* managers (not
  cSprite_Manager): update the base API + the ~63 `->objects` read sites. Fixes both dead bugs by
  construction.
- [ ] 5.3 `cSprite_Manager` storage → `unique_ptr`, preserving the player exemption + auto-destroy
  + z-order semantics (highest risk; own commit).
- [ ] 5.4 `Copy()` → `unique_ptr<cSprite> Clone() const override` across the sprite hierarchy.
- [ ] 5.5 Savegame pointer graph → values / `unique_ptr`.
- [ ] 5.6 Remaining owning `new`/`delete` in the touched areas → `make_unique`.

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

1. Line-point ownership under a `unique_ptr` base: make `cLayer` a non-owning
   `vector<cLayer_Line_Point_Start*>` (recommended — matches today's clear-only Delete_All), or
   keep it a manager and have `cSprite_Manager` borrow instead? (Needed before 5.2; my
   recommendation is to make `cLayer` non-owning.)
