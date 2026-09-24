# Ownership and lifetime — the 24 singletons, cObject_Manager, and who frees what (L1 + L2)

Cross-cutting synthesis over `core/main.cpp`, `core/obj_manager.h`, `core/sprite_manager`, and
every subsystem's manager. Provenance: commit `d438165`. Baseline: a game that predates smart
pointers — every owner is a raw pointer or a `vector<T*>`, freed by hand.

## L1 — the mental model

There are **24 process-global singletons**, each an `extern cX *pX` declared in its subsystem
header and defined `= NULL` there. `core/main.cpp:Init_Game` `new`s all 24 in a hand-chosen order;
`Exit_Game` `delete`s them in another hand-chosen order. There is no RAII: lifetime is 25 `new`
stanzas and 25 `if(p){delete p;}` stanzas, 160 lines apart. Most collection owners derive from one
template, `cObject_Manager<T>` — a public `vector<T*>` with manual `delete`. **The single highest-leverage
modernization is to make one `cApplication` own these as `unique_ptr` members in dependency order,
and to make every manager a `vector<unique_ptr<T>>`** ([[modernization-plan]]).

## L2 — the mechanisms

### The 24 singletons, construction order (`Init_Game`)

1 `pResource_Manager` · 2 `pVideo` · 3 `pAudio` · 4 `pFont` · 5 `pFramerate` · 6 `pRenderer` ·
7 `pRenderer_current` · 8 `pPreferences` · 9 `pImage_Manager` · 10 `pSound_Manager` ·
11 `pSettingsParser` — then the non-`new` init stage (user dir, SDL, video, the real CEGUI system,
`pPreferences->Load` under a temporary "fake" CEGUI on a NullRenderer) — 12 `pCampaign_Manager` ·
13 `pLevel_Player` (with a NULL sprite manager, `m_disallow_managed_delete = 1`) · 14 `pLevel_Manager`
(its ctor `new`s `m_camera` and the permanent first `cLevel`, sets `pActive_Camera`) · 15 `pHud_Manager` ·
16 `pLevel_Editor` · 17 `pWorld_Editor` · 18 `pMouseCursor` · 19 `pKeyboard` · 20 `pJoystick` ·
21 `pOverworld_Player` · 22 `pOverworld_Manager` · 23 `pMenuCore` · 24 `pSavegame`.

`pPreferences` (8) must come after `pVideo`/`pAudio` (2,3) because its ctor's `Reset_Video`/`Reset_Audio`
write fields that actually live in `pVideo`/`pAudio` (`m_geometry_quality`, `m_texture_quality`,
`m_sound_volume`, `m_music_volume` — four "preferences" with no field in `cPreferences`).

### Teardown order (`Exit_Game`) and where it is wrong

Teardown is NOT reverse-of-construction. Verified hazards (from `core.md`):
- `pFramerate` (built 5th) is **never deleted** — a leak (`rg "delete pFramerate"` → 0).
- `pAudio` (built 3rd) is deleted **first**, before `pSound_Manager`; two `Unload()` calls run first to make that safe.
- `pLevel_Player` (built 13th) is deleted **2nd**, before `pLevel_Manager` (15th) whose sprite managers still point at it — safe only because of `m_disallow_managed_delete`.
- `pGuiSystem`/`pGuiRenderer` are created inside `pVideo->Init_CEGUI()` but destroyed by `main.cpp`, not `~cVideo` (asymmetric ownership; `~cVideo` is empty — no `SDL_DestroyWindow`).

This hand-ordering is precisely what `unique_ptr` members of a `cApplication` fix by construction:
destruction runs in reverse declaration order automatically, and the leak disappears.

### `cObject_Manager<T>` (core/obj_manager.h) — 13 subclasses, two real bugs

Interface: `Add(T*)`, `Delete(size_t)`, `Delete(T*)`, `Delete_All()`, `Get_Pointer(size_t)`,
`Switch_Array_Num`, `Get_Array_Num`, and a **public** `vector<T*> objects` (read directly at ~63
sites — `->objects.begin()` loops everywhere). Subclasses: `cSprite_Manager`, `cWorld_Sprite_Manager`,
`cCampaign_Manager`, `cObjectCollisionType`, `cLevel_Manager`, `cBackground_Manager`, `cHud_Manager`,
`cSound_Manager`, `cImage_Manager`, `cAnimation_Manager`, `cOverworld_Manager`, `cLayer`.

Both bugs are **verified by reading the source, and dead-by-nonuse today** (checked independently
in this session, and by the `core/` reader):

- **`Delete(size_t)`** erases element N (shifting N+1… down) and *then* `delete objects[array_num]`,
  i.e. it deletes the element that shifted into slot N, not the one it removed — a leak plus a
  dangling delete, or an out-of-bounds read if N was last. **No caller passes an index** (every
  `Delete(` call passes a `T*`); it survives as a compiled vtable entry.
- **`Switch_Array_Num`** does `itr1 = obj2; itr2 = obj1;` (assigning a `T*` to a `vector<T*>::iterator`)
  under a `// todo: does that work ?`. It swaps nothing, and with libstdc++ would not even compile —
  it survives only because it is a non-virtual template member with **no caller anywhere**. Correct
  body: `std::iter_swap(itr1, itr2)`.

Replacing the base with `std::vector<std::unique_ptr<T>>` + `std::erase`/`std::iter_swap` fixes both
by construction and turns the 63 `->objects` loops into range-for.

### `cSprite_Manager` specifics

Owns every `cSprite*` except those with `m_disallow_managed_delete` (the player). z-order is a
float-epsilon scheme: `m_z_pos_data[type]` (a 100-entry `vector<float>`) tracks the running max per
`SpriteType`, and `Set_Pos_Z` stacks each new sprite `0.000001f` above it. `Add` reuses the first
`m_auto_destroy` slot before appending (so array order ≠ insertion order, and destroyed sprites are
freed lazily by the next `Add`). Typed queries (`Get_Size_Array`, `Get_from_Position`,
`Get_Objects_sorted`) are linear scans over the single flat vector.

### Non-owning "active" aliases

`pActive_Camera`, `pActive_Player`, `pActive_Level`, `pActive_Overworld`, `pActive_Animation_Manager`
are **borrowed pointers** re-pointed on every mode switch (by `cLevel::Enter`, `cOverworld::Enter`,
`cMenuCore::Enter`). Cameras are owned by their mode manager (`cLevel_Manager::m_camera`,
`cLevel_Settings::m_camera`, `cMenuCore::…m_camera`, `cOverworld_Manager::m_camera`), not by `core`.

Related: [[game-loop-and-modes]], [[object-model]], [[core]], [[modernization-plan]].
