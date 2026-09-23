# SMC source reference set — the map (L0)

> **Provenance:** the game's own source, `smc/src` at commit `d438165` (2026-09-23: the SDL 2 port
> applied; upstream's last commit is 2014, tag `import/github-fluxy-master`). Third-party code
> (`third_party/cegui-0.7.9`, SDL 2, Boost) is out of scope and appears only as the seams the game
> calls. Built by `tasks/archive/2026/09/23/smc-source-analysis-reference-set.md` (ten parallel readers, then
> synthesis; every durable claim verified against the source). Companions outside this set:
> `../building-2010-era-smc-today.md` (how it builds/runs today, the SDL 2 port) and
> `../history-provenance.md` (where `master` came from). Re-sync check: `git -C .. rev-parse HEAD`
> against the SHA above; if the tree moved, re-verify counts before trusting them.

The game is **73,459 lines of 2010-era C++** across ten subsystems. The one idea to hold in mind:
identity is a **runtime type tag** (`m_type`, 58 values), not the C++ type, so gameplay branches on
tag comparisons + guarded downcasts rather than virtual dispatch (0 `override` in the whole game).
The one structural debt: **24 global singletons** with hand-ordered `new`/`delete` and a family of
owning `vector<T*>` managers. Read the six cross-cutting docs first; the per-subsystem docs are the
verified detail.

## Status board

| Doc | Layer | What it covers |
|---|---|---|
| [object-model](object-model.md) | L1+L2 | the sprite hierarchy, the two type-tag enums, `Copy()` cloning, collision dispatch |
| [ownership-and-lifetime](ownership-and-lifetime.md) | L1+L2 | the 24 singletons' init/teardown, `cObject_Manager<T>` + its two verified bugs, sprite ownership |
| [game-loop-and-modes](game-loop-and-modes.md) | L1+L2 | the frame, `Game_Mode`, the deferred `Game_Action` string bus, modal loops |
| [data-formats](data-formats.md) | L2 | level/world/savegame/preferences/.settings XML, the CEGUI XML seam, version migrations |
| [input-flow](input-flow.md) | L1 | SDL event → CEGUI → mode → player; the joystick→keyboard bridge; the key-state adapter |
| [editors](editors.md) | L2 | `cEditor`, level/world editors, level settings, the CEGUI subscribeEvent census |
| [core](core.md) | L2 | main/init/teardown, game_core, obj_manager, sprite_manager, camera, editor, property/filesystem/math |
| [objects](objects.md) | L2 | the sprite bases + every non-enemy object |
| [enemies](enemies.md) | L2 | `cEnemy` + the 13 enemies + the turtle/boss duplication |
| [level](level.md) | L2 | `cLevel`, `cLevel_Manager`, and `cLevel_Player` (the largest state machine) |
| [video](video.md) | L2 | `cVideo`, the fixed-function GL inventory, the render queue, textures, fonts, particles |
| [gui](gui.md) | L2 | the menu system, HUD, dialogs, clipboard, the CEGUI 0.7 API surface |
| [overworld](overworld.md) | L2 | `cOverworld`, waypoints/layers, the world player and editor |
| [input](input.md) | L2 | keyboard, mouse (+editor selection), joystick |
| [user](user.md) | L2 | preferences and savegames (the pointer object graph) |
| [audio](audio.md) | L2 | `cAudio`, the sound-channel pool, music, `cSound_Manager` |

## L0 capsules (one paragraph each)

- **core** — `main.cpp` is the process: it `new`s the 24 singletons, runs the frame loop, and pumps
  SDL events. `game_core` holds the global mode/action state and the string command bus. `obj_manager`
  is the owning-`vector<T*>` template every manager derives from (with two dead-but-real bugs).
  `sprite_manager` owns a level's sprites with a float-epsilon z scheme. `editor`, `camera`,
  `property_helper` (number/enum ↔ string), `filesystem` (the only Boost user) and `math` round it out.
- **objects** — the `cCollidingSprite → cSprite → cMovingSprite → cAnimated_Sprite` spine and every
  non-enemy object (boxes, pickups, platforms, paths, text box, ball). Home of the `Copy()` clone idiom
  (14 classes) and the `m_type`/`m_sprite_array` tags every subsystem branches on.
- **enemies** — a flat family of 13 enemies off `cEnemy`, plus the one boss. Nearly identical method
  shapes; `cTurtleBoss` duplicates 22 methods of `cTurtle`. The biggest dedup target.
- **level** — `cLevel` (XML load/save, owns the sprite/background/animation managers), `cLevel_Manager`
  (the level list + update/draw orchestration + sub-level warps), and `cLevel_Player`, the game's
  largest state machine (power-up types, movement states, collision responses, a 200-line modal death
  sequence). 503 float literals; 26 input-poll sites.
- **video** — `cVideo` brings up the SDL window + GL context + CEGUI, and does all texture work. The
  renderer is a per-frame z-sorted queue of heap `*_Request` objects drawn in **immediate-mode
  fixed-function GL** (no VBOs; nothing core-profile-clean). A dead `boost::thread` render experiment.
- **gui** — the CEGUI menu system (main/start/options/savegames/credits), the HUD (which also holds
  game logic like the 100-coins-1-up rule), modal dialog boxes with nested event loops, and the exact
  CEGUI 0.7 API surface that pins the dependency.
- **overworld** — the world map: `cOverworld`, waypoints with lock/access, the line-segment walk graph,
  the world player (simpler than the level player; shares only the protocol, not the code), and the
  world editor.
- **input** — keyboard (CEGUI injection + hotkeys + the scancode adapter), mouse (also the entire
  editor selection/copy/mover model), joystick (synthesizes keyboard events from axes/hat; no hotplug).
- **user** — `cPreferences` (44 settings, `~/.smc/config.xml`) and `cSavegame` (a hand-managed pointer
  object graph, object state matched back by position+type).
- **audio** — `cAudio` over SDL_mixer: a ≤10-entry channel-object pool, filename-string sounds (no id
  constants), poll-based music looping, and a finished-callback that touches game state from the mixer
  thread.

## Live / dead ledger

Everything below is verified (grep + read). "Dead" = no caller found in `src/`.

**Dead code to delete (safe):**
- `cObject_Manager::Switch_Array_Num` (no caller; would not compile) and `Delete(size_t)` (no caller; buggy) — `core/obj_manager.h`.
- `cSprite_Manager::Copy(unsigned)`, `operator[]`, the `Delete_All(delayed=1)` branch — `core/sprite_manager`.
- `cCampaign::Save`; `string_to_double`, `Get_Ground_Type_Name`; `Delete_Dir`, `Get_File_Size` — `core/`.
- `GL_Vector`'s norm/normalize/dot/angle/cross/rotate/round; `cSize_Float`; `GL_line::Get_Intersection`/`Distance_to_Line`; `GL_point::rotate3d`; `GL_rect::Get_Rect`/`Get_Rect_pos` — `core/math` (`GL_Vector` is a bare 2-float carrier in mouse.cpp).
- `cVideo::Get_Pixel`, `Render_From_Thread`, `Make_GL_Context_Current/Inactive`, `Render_Finish` body, `m_render_thread`, `m_default_buffer`, `m_rgb_size`, `m_double_buffer`; `cGL_Surface::Save`; `cImage_Manager::Copy(path)`/`operator[]`; `REND_TEXT`; 6 unsubscribed particle-editor handlers — `video/` (all behind the disabled `SMC_RENDER_THREAD_TEST` or unreferenced).
- `cAudio::Resume_Sound`, `Fadeout_Sounds(ms,channel,overwrite)`, `Set_Music_Position`, `Is_Music_Paused`, `Halt_Sounds`; `cSound_Manager::Delete_Sounds` (buggy), `m_load_count`, both `operator[]` — `audio/`.
- `cPreferences::Apply_Audio`; `cLevel_Player::Copy` (returns NULL), `Action_Stop_Shoot` (empty); `cLevel::Process_Input`, `cLevel_Manager::Init` (empty); `cLevel::m_next_level_filename`/`m_delayed_unload` (never set); `cMenu_Options::Change_*_Setting` (~250 lines, unreachable); `cMenu_Item::m_is_quit` (written, never read); `cMenuCore::Key_Up`/`Joy_Button_Up`/`Mouse_Up` stubs; `cMenu_Start::Level_Select` (todo body).

**Dead globals / branches:** `editor_level_enabled`, `editor_world_enabled` never written → every
`if(editor_level_enabled)` reader is dead (14 files); `game_res_w/h` never reassigned (constants);
`cAudio::m_debug` never set true → 9 audio `printf`s unreachable.

**Never freed (leak):** `pFramerate`.

**Live but fragile (keep, flagged for the modernization):** the whole `cObject_Manager<T>` family;
the 24-singleton hand-ordered teardown; the `Game_Action` string bus; the ~216 fire-and-forget CEGUI
subscriptions; the type-tag dispatch; the `Finished_Sound` cross-thread vector walk; object-state
save/restore matched by position; the "active level" `posx!=0 && posy!=0` sentinel.

**Real bugs found (not just style)** — see each subsystem doc: the `Draw_Animation` CAPE→SMALL
condition that can never be true (a costume-size skip); `Load_Images` putting the fall sprite in the
jump slot for four costumes; `Joystick_Name_Click`'s wrong window name → uncaught exception; the
mixer-thread data race in `Finished_Sound`; `Fadeout_Sounds(filename)` fading all channels via
channel −1; `Toggle_Sounds` silencing music; `Apply_Max_Texture_Size` not preserving aspect ratio;
`main.cpp` arg-parse `option = i + i` typo. These are candidates for their own `proposed` fix tasks,
independent of the modernization.

Related: [[object-model]], [[ownership-and-lifetime]], [[game-loop-and-modes]], [[data-formats]],
[[input-flow]], [[editors]].
