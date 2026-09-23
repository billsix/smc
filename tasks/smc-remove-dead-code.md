# Remove verified dead code

**Status:** proposed — needs go-ahead (found in the source analysis, 2026-09-23)
**Priority:** 6 · **Difficulty:** 3 (mechanical, but re-verify each removal against a fresh grep)
**Do before:** `tasks/smc-modernize-cpp.md` (deleting before rewriting means less to modernize).

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
