# Replace the `*_Manager` classes with modern ownership shapes, and fix the stored-pointer lifetimes (done 2026-11-07)

**Play-test owed** by the maintainer (the list at the end). The smoke run cannot see gameplay,
teardown, save/load, or the editor.

## What was decided

- **Replace, not just remove** (William Emerison Six <billsix@gmail.com>, 2026-11-07): modern C++
  should make lifetime "managers" much simpler. The agent's design input went into the shared C++
  standard (github.com/billsix/runClaudeInContainer `tasks/reference/cpp-coding-standard.md`,
  section "Stored borrows and 'manager' classes"):
  - raw `T*`/`T&` parameters are correct (Core Guidelines `F.7`, `R.30`);
  - the hazard is stored pointers whose target can die first;
  - a container class must earn its existence with an invariant, hold its storage privately by
    composition, and be named for that invariant.
- The task first required a maintainer play-test between phases. After phase 1 the maintainer lifted
  that: "just go ahead, I'll try to play test it later" (2026-11-07). Each phase still passed the gates,
  and the play-test items were collected into one list.
- What became of each class, with the rationale, is now in the reference doc:
  `tasks/reference/refactoring-to-modern-cpp.md`, "The third pass".

## What was done (branches `std-swaps-managers-cmake`, then `reformat-and-managers`)

1. **Survey (phase 1).**
   - `cGL_Surface*` members are safe as raw pointers: a GL-context texture reload
     (`Restore_Textures`) reloads each surface in place.
   - `cAnimation_Manager` stays a class: 44 `Add` sites, a per-scene `Update`/`Draw`, and the "active"
     pointer switching between level, overworld, and menu.
   - `cBackground_Manager` was the one pure container. It became `cLevel::m_backgrounds` (commit
     `1d980d9`, codemod `backgrounds_to_vector.py`).
2. **Un-derive (phase 2, `e886d1d`).**
   - `cObject_Manager<T>` and `core/obj_manager.h` were deleted. The nine subclasses hold a private
     `Owning_Vector<T>` (new `core/owning_vector.h`).
   - The 63 outside `->objects` loops became `for (auto* x : mgr->Objects())`.
   - The collision lists move collisions through `Take_All`/`Append`.
   - Codemod `underive.py`, then compiler-driven fixes.
   - Traps:
     - clang rejects inline `Add`/`Delete_All` where `cLevel`/`cOverworld` are incomplete, so those
       bodies live in the `.cpp`;
     - a lone `virtual` on `cSound_Manager` had to go once the virtual base destructor was gone
       (`-Wdelete-non-abstract-non-virtual-dtor`).
   - Only 2 of the ~17 `-Woverloaded-virtual` suppressions were the managers'. Both were removed;
     the other 15 belong to the sprite hierarchy.
3. **Pure containers (phase 3, `7773923`).**
   - `cCampaign_Manager` stayed a class, against the plan's guess: it is a loader that merges the user
     and game campaign directories, a by-name lookup, and an application singleton
     (`cOwned<cCampaign_Manager>`).
   - Its unused `Delete_All` went.
4. **Caches (phase 4, `0d932b4`).** Image and sound lookup by filename uses an
   `unordered_map<string, T*>` index:
   - `try_emplace` keeps the first object added under a name, as the old linear scan did;
   - it is exact because each cache has one `Add` site, the filename is set before `Add`, and no
     managed object is renamed afterwards;
   - the owning vector stays for the whole-cache passes.
5. **Stored sprite references (phase 5, `0d95521`).**
   - Six members became `cSprite_Handle<T>` (new `core/sprite_handle.{h,cpp}`):
     `cMovingSprite::m_ground_object`, `cObjectCollision::m_obj`, `cSelectedObject::m_obj`,
     `cMouseCursor::m_active_object` and `m_last_clicked_object`, and
     `cLevel_Player::m_active_object`.
   - The mechanism is a registry, not slot generations:
     - every `cSprite` holds a never-reused serial, and a handle reads as `nullptr` once its sprite
       is freed, even if a new sprite reuses the address;
     - slot generations did not fit because the store reorders its vector and these members point
       across several stores and at the players.
   - Before this, `cSprite_Manager::Add` freed a destroyed sprite on slot reuse, and nothing nulled
     these members.
   - The `= nullptr` sites stayed: each is a game rule applied at `Destroy()`, which does not free.
   - Cost, measured with temporary counters over 30 s of `cave_1`: about 130 lookups per frame.
6. **Fold (phase 5, `5f8e4bd`).**
   - `cWorld_Sprite_Manager`'s one `Add` override (index waypoints, register layer-line starts)
     became the `m_on_add` hook, which `cOverworld` sets. The store has no virtuals now.
   - Proof: a temporary print at the end of `cOverworld::Load` gave identical waypoint / line /
     sprite counts for all five worlds before (a scratch `git worktree` of `0d95521`) and after.
7. **Renames (phase 6, `222e7d7`, `0027e3c`, `83db992`, type name only):**
   - `cSprite_Manager` → `cSprite_Store`;
   - `cImage_Manager` → `cTexture_Cache`;
   - `cSound_Manager` → `cSound_Cache`.

   The optional renames (`cFont_Manager`, `cResource_Manager`, the level and overworld managers)
   were not made. `tasks/reference/smc/README.md` carries a rename note, because that set is a
   snapshot of the older code.
8. **Phase 7** (never-null `T*` → `T&`, opportunistic) was not done. **Phase 8:** recorded in
   `tasks/reference/refactoring-to-modern-cpp.md`, "The third pass", and in `CLAUDE.md`.

## Verification

Each phase passed:

- gcc and clang `-Werror` with 0 warnings (`tools/compiler_gate.sh`);
- `build.sh game` followed by `tools/smoke_headless.sh`, run on the menu, with
  `SMC_ARGS="--level cave_1"`, and with `--world world_1` (exit 137 at the deadline, `ps` audit
  clean).

The tree is clang-format clean.

## Play-test list (owed by the maintainer)

- **Levels:**
  - enemies, items, collisions, stomping, and falling into a pit;
  - walking and jumping on moving platforms, and a platform or box vanishing under Maryo;
  - carrying a turtle shell, and dying or finishing the level while carrying it.
- **Level visuals:** the backgrounds draw, and the main menu's background fades.
- **HUD:** points, time, lives, gold, and the item box.
- **Sound:** sounds and music play.
- **Options:** a resolution change, which reloads textures, keeps every image correct.
- **Overworld:** the map loads, Maryo walks the paths between waypoints, and levels can be entered.
- **Menus:** the campaign list and the world list fill.
- **Save/load:** saving and loading a game, both in a level and on the overworld.
- **Level editor:**
  - placing objects; selecting, dragging, and clicking them; deleting the selected object;
  - deleting a shell the player is carrying;
  - the level-settings screen adds and deletes background images and edits the gradient colours.
- **World editor:** adding a waypoint and a line.
