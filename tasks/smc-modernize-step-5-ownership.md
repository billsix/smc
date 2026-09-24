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

- **5.1 done** (this commit): the queue no longer hand-deletes any request; ownership transfers
  into a `unique_ptr` at `Add` time. Requests that persist across frames (`m_render_count > 1`) are
  unaffected; typeless (`REND_NOTHING`) requests are still dropped immediately (now freed by the
  adopting `unique_ptr` on return, not a manual `delete`).

## Open questions

None.
