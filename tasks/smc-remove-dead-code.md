# Remove verified dead code

**Status:** IN PROGRESS 2026-09-23 (branch `pre-modernization-fixes`) — a verified, build-green
first pass is done; a documented remainder is deferred (below). NOT archived.
**Priority:** 6 · **Difficulty:** 3 (mechanical, but re-verify each removal against a fresh grep)
**Do before:** `tasks/smc-modernize-cpp.md` (deleting before rewriting means less to modernize).

## Done this pass (each re-verified zero-caller by fresh grep, compiles green)

- `cObject_Manager::Delete(size_t)` and `Switch_Array_Num` (obj_manager.h) — both dead + buggy.
- `string_to_double`, `Get_Ground_Type_Name` (property_helper.{h,cpp}).
- `Delete_Dir`, `Get_File_Size` (filesystem.{h,cpp}) — kept `Delete_Dir_And_Content` (used).
- `cPreferences::Apply_Audio` (preferences.{h,cpp}).
- `cCampaign::Save` (campaign_manager.{h,cpp}).
- `cAudio::Resume_Sound`, the `Fadeout_Sounds(ms, channel, overwrite)` overload (kept the
  filename overload, used), `Set_Music_Position`, `Is_Music_Paused`, `Halt_Sounds`;
  `cSound_Manager::Delete_Sounds` (audio.*, sound_manager.*).
- Second pass (verified zero-caller, compiles green): `cVideo::Get_Pixel`, `cGL_Surface::Save`
  (kept `Copy`, used), `cImage_Manager::Copy(path)` (kept `Get_Pointer`), `cSprite_Manager::Copy`,
  the unused `REND_TEXT` render-type enum value.

## CORRECTION to the ledger (verify, don't trust)

**`editor_level_enabled` / `editor_world_enabled` are NOT dead** — the analysis ledger was wrong.
Fresh grep shows both are written: `editor_level_enabled` in `level_editor.cpp` Enable/Disable,
`editor_world_enabled` in `world_editor.cpp` Enable/Disable. Their reader branches are live. The
README ledger has been corrected. This is why every remaining item below must be re-verified before
removal, not taken from the ledger on faith.

## Deferred (with reasons) — remainder for a careful follow-up

- **The `SMC_RENDER_THREAD_TEST` block** (`video.cpp` `Render_From_Thread`,
  `Make_GL_Context_*`, `Render_Finish` body, `m_render_thread`) — remove together with
  `boost::thread` in `tasks/smc-modernize-cpp.md` step 3, so it goes once (per that coordination).
- **Virtuals / interface methods** (`cLevel_Player::Copy` returns NULL, `Action_Stop_Shoot`,
  `cLevel::Process_Input`, `cLevel_Manager::Init`) — removing an override changes dispatch; needs
  per-item proof it is never called polymorphically before removal. Not safe as a blind delete.
- **`cMenu_Options::Change_*_Setting`** (~250 lines) — the gui analysis called it dead, but given
  the ledger error above it must be re-verified (its `m_active`-guard reasoning) before removing.
- **Lower-value micro-removals** (unused `core/math` methods — `GL_Vector` ops, `cSize_Float`,
  `GL_line::Get_Intersection`/`Distance_to_Line`, `GL_point::rotate3d`, `GL_rect::Get_Rect*`;
  `cVideo::Get_Pixel`, `cGL_Surface::Save`, `cImage_Manager::Copy`/`operator[]`, `REND_TEXT`;
  `cSprite_Manager::Copy`/`operator[]`; `cMenu_Item::m_is_quit`; the `cMenuCore` Up stubs;
  `cMenu_Start::Level_Select`; `cAudio::m_load_count`) — header/isolated, each needs a fresh
  zero-ref check; low urgency.
- `game_res_w/h` → `constexpr`: an optimization, not a deletion; left as-is.

## BLUF

The source analysis verified a list of functions, members, globals and branches with no caller in
`src/`. Delete them (re-confirming each is still unreferenced at removal time), so the modernization
rewrites less code. The full list is the "Dead code to delete" ledger in
`tasks/reference/smc/README.md`; this task tracks executing it.

## Context

- Source of truth: the live/dead ledger in `tasks/reference/smc/README.md`, cross-referenced from
  each subsystem doc's "Dead or suspicious code" section (verified by grep + read, 2026-09-23).
- Headline items: `cObject_Manager::Switch_Array_Num` and `Delete(size_t)` (both no-caller; the
  latter buggy); `cSprite_Manager::Copy`/`operator[]`/the delayed `Delete_All` branch; `cCampaign::Save`;
  `string_to_double`, `Get_Ground_Type_Name`, `Delete_Dir`, `Get_File_Size`; the unused `core/math`
  methods (`GL_Vector` norm/normalize/dot/…, `cSize_Float`, `GL_line::Get_Intersection`/`Distance_to_Line`,
  `GL_point::rotate3d`, `GL_rect::Get_Rect`/`Get_Rect_pos`); the dead `SMC_RENDER_THREAD_TEST` block
  in `video/` (`Render_From_Thread`, `Make_GL_Context_*`, `Render_Finish` body, `m_render_thread`,
  and the `m_default_buffer`/`m_rgb_size`/`m_double_buffer` write-only fields) — note this overlaps
  the Boost-drop in `smc-modernize-cpp.md` step 3, coordinate so it is removed once;
  `cVideo::Get_Pixel`, `cGL_Surface::Save`, `cImage_Manager::Copy`/`operator[]`, `REND_TEXT`, the 6
  unsubscribed particle-editor handlers; the dead `cAudio` methods and `cSound_Manager::Delete_Sounds`
  (buggy)/`m_load_count`/`operator[]`; `cPreferences::Apply_Audio`; `cLevel_Player::Copy`/`Action_Stop_Shoot`,
  `cLevel::Process_Input`/`m_next_level_filename`/`m_delayed_unload`, `cLevel_Manager::Init`; the whole
  `cMenu_Options::Change_*_Setting` (~250 lines), `cMenu_Item::m_is_quit`, the `cMenuCore` Up-event
  stubs, `cMenu_Start::Level_Select`.
- Dead globals/branches: `editor_level_enabled`/`editor_world_enabled` (never written → dead
  readers in 14 files); `cAudio::m_debug` (never true → 9 unreachable `printf`s). `game_res_w/h` are
  never reassigned — make them `constexpr` rather than delete (they are read widely).

## Plan

- [ ] Re-grep each item at removal time (`rg -n '\bSymbol\b' src`) — a lead from 2026-09-23 is not a
      licence to delete blind; confirm zero callers now.
- [ ] Remove in coherent commits by subsystem (one commit per file/area), each message naming what
      went and the grep that proved it dead.
- [ ] `game_res_w/h` → `constexpr`; the `editor_level_enabled`/`editor_world_enabled` dead branches
      simplified (the readers become unconditional or drop the check).
- [ ] Coordinate the `SMC_RENDER_THREAD_TEST` removal with `smc-modernize-cpp.md` step 3 (Boost drop)
      so `boost::thread` and its dead code go together, once.

## Verification

Container build green after each commit; headless smoke run passes; `rg` shows the removed symbols
gone with no dangling references.

## Open questions

None.
