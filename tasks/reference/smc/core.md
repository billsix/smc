# Source analysis — `core/` (incl. `core/filesystem/`, `core/math/`)

Tree read: `smc/src/core/` at commit `d438165 update to SDL2` (working tree clean). All paths below are
relative to `smc/src/`. Scope = the files named in the brief (main, game_core, editor, obj_manager,
sprite_manager, camera, campaign_manager, property_helper, filesystem/*, math/*) = **7 879 lines**;
the other `core/` files (collision, file_parser, framerate, i18n, global_basic/global_game) are listed in
§1 for completeness and only skimmed. Every `rg` was run from `smc/src/`; counts exclude `third_party/`.

## 1. Files and roles

| file | lines | role | status (evidence) |
|---|---|---|---|
| `core/main.cpp` | 757 | `main()`, `Init_Game`, `Exit_Game`, `Handle_Input_Global`, `Update_Game`, `Draw_Game` — the process entry, all global construction/teardown, the frame loop | live; `main.h` included by 9 files (camera.cpp calls `Draw_Game` from `cCamera::Move_to_Position_Gradually`) |
| `core/main.h` | 53 | prototypes of the five functions above | live |
| `core/game_core.cpp/.h` | 640/135 | global state (`Game_Mode`, `Game_Action*`, `game_exit`, scales, `editor_*_enabled`, `pActive_Camera/Player`), the deferred-action dispatcher `Handle_Game_Events`, mode enter/leave, `Preload_Images/Sounds`, `Write_Property`, `Relocate_Image` | live; `game_core.h` is the most-included header of the game: 67 includers (58 outside core) |
| `core/obj_manager.h` | 188 | `template<class T> cObject_Manager` — a `vector<T*>` owner with Add/Delete/Get | live; 13 subclasses (§3) |
| `core/sprite_manager.cpp/.h` | 402/180 | `cSprite_Manager : cObject_Manager<cSprite>` — the per-level/world/menu sprite list, z-order bookkeeping, collision driver | live; 24 includers |
| `core/camera.cpp/.h` | 472/131 | `cCamera` — scroll position, limits, player-follow offsets, gradual moves | live; `new cCamera` in level_manager.cpp:36, level_settings.cpp:50, menu.cpp:121, world_manager.cpp:39 |
| `core/campaign_manager.cpp/.h` | 263/103 | `cCampaign`, `cCampaign_Manager : cObject_Manager<cCampaign>`, `cCampaign_XML_Handler` (CEGUI XMLHandler) | live but thin: only gui/menu_data.cpp uses it (`pCampaign_Manager` 4 files / 9 lines); `cCampaign::Save` has **no caller** (dead) |
| `core/editor.cpp/.h` | 1778/266 | `cEditor` base (CEGUI XMLHandler): item/menu listboxes, keyboard/mouse editing, copy/select; plus `cEditor_Object_Settings_Item`, `cEditor_CEGUI_Texture`, `cEditor_Item_Object`, `cEditor_Menu_Object` | live; subclassed by `level/level_editor.h:cEditor_Level` and `overworld/world_editor.h:cEditor_World` |
| `core/property_helper.cpp/.h` | 925/137 | string↔number conversion, enum↔name tables, string trim/replace, Win32 UTF-8↔UCS-2 | live; only 2 direct includers but re-exported via `game_core.h` (`int_to_string` 195 call sites) |
| `core/filesystem/filesystem.cpp/.h` | 281/87 | free functions over `boost::filesystem` + POSIX/Win32 | live; 28 includers; the **only** Boost.Filesystem user in the game |
| `core/filesystem/resource_manager.cpp/.h` | 89/51 | `cResource_Manager` = one string `user_data_dir` + mkdir of the 6 user sub-dirs | live; `user_data_dir` read at 24 sites |
| `core/math/rect.h` | 195 | `GL_rect` (float x,y,w,h) + `Intersects` | live; 102 uses outside math |
| `core/math/point.h` | 134 | `GL_point` + `rotate`, `rotate3d`, `distance` | live (20 uses); `rotate3d` only in 3 commented-out lines (img_settings.cpp:129,141,153), `distance` only used inside line.h |
| `core/math/line.h` | 127 | `GL_line` + `Intersects`, `Get_Intersection`, `Distance_to_Line`, templates `pow2`, `cl_min` | live-ish: `GL_line` 10 uses (world_layer, video Draw_Line, renderer); `Get_Intersection`/`Distance_to_Line` **no callers**; `pow2`/`cl_min` used only inside line.h |
| `core/math/size.h` | 78 | `cSize_Int`, `cSize_Float` | `cSize_Int` live (21 uses); `cSize_Float` **only forward-declared** (global_game.h:452), never instantiated |
| `core/math/utilities.cpp/.h` | 69/82 | `Clamp`, `Get_Random_Float`, `Get_Power_of_2`, `Is_Valid_Number`, `Is_Float_Equal` | live; 17 includers (`Is_Float_Equal` 62, `Get_Random_Float` 30) |
| `core/math/vector.cpp/.h` | 99/157 | `GL_Vector` (x,y,z,w) ClanLib port | **effectively dead**: 4 uses, all in `input/mouse.cpp:Get_Copy_Object_Base` which only reads `.x/.y`; `norm/normalize/dot/angle/cross/rotate/round` have **zero** callers |
| `core/global_basic.h` | 73 | std includes, `debug_print`, `using std::vector` etc. | live (61 includers). Contains the SDL2-port `typedef SDL_Keycode SDLKey;` **pasted 4 times** (lines 51–62) — a sed artefact of commit d438165 |
| `core/global_game.h` | 467 | version macros, DATA_DIR/`*_DIR` macros, all the game-wide unscoped enums, forward decls | live |
| `core/collision.cpp/.h` | 314/219 | `cObjectCollision`, `cObjectCollisionType : cObject_Manager<cObjectCollision>` | live (other reader's scope) |
| `core/file_parser.cpp/.h` | 131/51 | `cFile_parser` line tokenizer | live (other reader's scope) |
| `core/framerate.cpp/.h` | 234/132 | `cFramerate`, `cPerformance_Timer`, `Correct_Frame_Time` | live (34 includers) |
| `core/i18n.cpp/.h` | 155/43 | gettext wrappers `_()`, `N_()`, `UTF8_()` | live (48 includers) |

All of these are in `Makefile.am` (lines 14–50); nothing in `core/` is unbuilt.

## 2. Public surface

**Classes/functions other subsystems use** (verified by `rg`): `cObject_Manager<T>` (13 subclasses),
`cSprite_Manager` (every level/world/menu/hud owner), `cCamera`, `cEditor` (2 subclasses),
`cEditor_Object_Settings_Item` (objects/sprite.cpp:1487 `new`s one per editor row; `sprite.h:539`
typedef list), `cEditor_Item_Object`/`cEditor_Menu_Object` (4 uses in level/world editors),
`cCampaign_Manager` (menu_data.cpp), `cResource_Manager`, all of `filesystem.h`, all of
`property_helper.h`, `Handle_Game_Events`/`Clear_Input_Events` (8 files)/`Preload_*`/`Write_Property`
(37 files)/`Relocate_Image` (52 call sites, level/overworld loaders), `Draw_Game` (camera.cpp only).
`Handle_Input_Global`, `Update_Game`, `Init_Game`, `Exit_Game` have **no callers outside main.cpp**.
`Leave_Game_Mode`/`Enter_Game_Mode` are called **only** from `Handle_Game_Events`.

**Globals defined in this subsystem** (`rg -l -w` files / `rg -c -w` lines, whole `src/`):

| global | defined (`= …`) | new/delete | files / lines |
|---|---|---|---|
| `bool game_exit` | game_core.cpp:43 | — | 7 / 10 (set only in main.cpp:528 on `SDL_QUIT`; readers: level_player, menu_data, generic, overworld, world_player) |
| `GameMode Game_Mode` | game_core.cpp:44 | — | 14 / 88; written **only** in `Enter_Game_Mode` (game_core.cpp:308) |
| `GameModeType Game_Mode_Type` | game_core.cpp:45 | — | 9 / 18; written in savegame ×2, level_editor, menu_data ×3, overworld, main |
| `GameAction Game_Action` | game_core.cpp:46 | — | 10 / 59; `Game_Action = GA_*` at 54 sites (menu_data 25, level_player 10) |
| `XMLAttributes Game_Action_Data_{Start,Middle,End}` | game_core.cpp:47–49 | — | 10/56, 9/84, 10/39 |
| `void *Game_Action_ptr` | game_core.cpp:50 | — | 3 / 9; only level_player.cpp:3353–3539 stores a `cLevel_Exit*` |
| `int game_res_w/h` | game_core.cpp:52–53 (800×600) | — | 17/80, 21/82; **never reassigned** (`rg "game_res_[wh] ="` → only the definitions) — effectively constants |
| `bool game_debug`, `game_debug_performance` | game_core.cpp:55–56 | — | 14/18, 5/6 |
| `SDL_Event input_event` | game_core.cpp:58 | — | 9 / 72 (shared poll buffer) |
| `float global_upscalex/y, global_downscalex/y` | game_core.cpp:60–63 | — | 11/25, 11/19, 7/11, 6/8 |
| `bool editor_enabled`, `editor_level_enabled`, `editor_world_enabled` | game_core.cpp:65–67 | — | 27/75, 14/23, 5/14; `editor_enabled` written in level.cpp:471, level_settings.cpp:219, level_editor.cpp:77/122, menu.cpp:604, world_editor.cpp:72/95, overworld.cpp:475. `editor_level_enabled`/`editor_world_enabled` are **never assigned** anywhere (`rg "editor_(level|world)_enabled *="` → 0) — they stay `0` forever, so every `if( editor_level_enabled )` reader is a dead branch |
| `cCamera *pActive_Camera` | game_core.cpp:69 | not owned; points at a manager's `m_camera` (level.cpp:461, level_manager.cpp:41, level_settings.cpp:217, menu.cpp:598, overworld.cpp:421) | 42 / 175 |
| `cSprite *pActive_Player` | game_core.cpp:70 | not owned (main.cpp:302, level.cpp:463, menu.cpp:600, overworld.cpp:423) | 13 / 34 |
| `cCampaign_Manager *pCampaign_Manager` | campaign_manager.cpp:259 | new main.cpp:298 / delete main.cpp:424 | 4 / 9 |
| `cResource_Manager *pResource_Manager` | resource_manager.cpp:85 | new main.cpp:250 / delete main.cpp:506 | 12 / 32 |

The 22 other `pX` singletons `new`ed in main.cpp are each `extern`ed and defined (`= NULL`) in their own
subsystem header/.cpp (audio.h:221, video.h:289/292/293, font.h:74, framerate.h:126, renderer.h:276–277,
preferences.h:215, img_manager.h:122, sound_manager.h:92, img_settings.h:101, level_player.h:329,
level_manager.h:86, hud.h:72, level_editor.h:83, world_editor.h:69, mouse.h:277, keyboard.h:97,
joystick.h:104, world_player.h:141, world_manager.h:84, menu.h:178, savegame.h:252). No `new`/`delete`
of any of them exists outside main.cpp (`rg` → only video.cpp:1014 `pRenderer_current = new_render`,
a pointer swap, not an allocation).

## 3. Ownership and lifetime

### main.cpp construction (`Init_Game`, main.cpp:244–344) — exact `new` order

1. `pResource_Manager` (250) 2. `pVideo` (251) 3. `pAudio` (252) 4. `pFont` (253) 5. `pFramerate` (254)
6. `pRenderer = new cRenderQueue(200)` (255) 7. `pRenderer_current` (256) 8. `pPreferences` (257)
9. `pImage_Manager` (258) 10. `pSound_Manager` (259) 11. `pSettingsParser` (260)
— then the non-`new` init stage: `Set_User_Directory`, `Init_CEGUI_Fake`, `pPreferences->Load`,
`I18N_*`, `Delete_CEGUI_Fake`, `Init_User_Directory`, `Init_SDL/Init_Video/Init_CEGUI/Init_CEGUI_Data`
(these create `pGuiRenderer`/`pGuiSystem` inside video.cpp), `pFont->Init`, `pFramerate->Init`,
`pAudio->Init` (266–296) —
12. `pCampaign_Manager` (298; its ctor `Load()`s every `.smccpn`) 13. `pLevel_Player = new cLevel_Player(NULL)` (299; `m_disallow_managed_delete = 1`, `pActive_Player = pLevel_Player`)
14. `pLevel_Manager` (303; its ctor `new`s `m_camera` and `pActive_Level = new cLevel()`, sets `pActive_Camera`)
— `pPreferences->Apply`, `Loading_Screen_Init`, `Init_Image_Cache` —
15. `pHud_Manager` (319) 16. `pLevel_Editor` (321) 17. `pWorld_Editor` (325) 18. `pMouseCursor` (326)
19. `pKeyboard` (327) 20. `pJoystick` (328) 21. `pOverworld_Player` (331) 22. `pOverworld_Manager` (332)
23. `pMenuCore` (337) 24. `pSavegame` (338) — then `Preload_Images(1)`, `Preload_Sounds(1)`.

### main.cpp teardown (`Exit_Game`, main.cpp:346–520) — exact `delete` order

`pPreferences->Save()`, `pLevel_Manager->Unload()`, `pMenuCore->m_handler->m_level->Unload()`, then:
1. `pAudio` (358) 2. `pLevel_Player` (364) 3. `pHud_Manager` (370) 4. `pSound_Manager` (376)
5. `pLevel_Editor` (382) 6. `pWorld_Editor` (388) 7. `pPreferences` (394) 8. `pSavegame` (400)
9. `pMouseCursor` (406) 10. `pJoystick` (412) 11. `pKeyboard` (418) 12. `pCampaign_Manager` (424)
13. `pOverworld_Manager` (430) 14. `pOverworld_Player` (436) 15. `pLevel_Manager` (442) 16. `pMenuCore` (448)
17. `pRenderer` (454) 18. `pRenderer_current` (460) 19. `pGuiSystem->destroy()` + `delete rp; delete logger` (468–471)
20. `pGuiRenderer->destroy(*pGuiRenderer)` (476) 21. `pVideo` (482) 22. `pImage_Manager` (488)
23. `pSettingsParser` (494) 24. `pFont` (500) 25. `pResource_Manager` (506) — then `IMG_Quit`, `SDL_Quit`.

**Where teardown disagrees with construction** (reverse-of-construction would be 24→1):
- `pAudio` (built 3rd) is destroyed **first**, before `pSound_Manager` (4th delete) and before any sprite
  that might still play a sound during unload — the two `Unload()` calls run before it, so this is
  order-by-hand, not RAII.
- `pLevel_Player` (built 13th) is deleted 2nd, **before** `pLevel_Manager` (15th delete) whose levels'
  sprite managers still hold a pointer to it (only `m_disallow_managed_delete` in
  `cSprite_Manager::Delete_All` keeps them from double-deleting it).
- `pHud_Manager` deleted 3rd while `pLevel_Manager`/`pMenuCore` (which own the sprite managers it draws into) live on until 15/16.
- `pLevel_Editor`/`pWorld_Editor` deleted 5/6 while `pMouseCursor` (their `pMouseCursor->…` collaborator, 9th) and the level/world managers (15, 13) outlive them; `pKeyboard`/`pJoystick` deleted 10/11 while `pMenuCore` (16) can still receive events.
- `pRenderer`/`pRenderer_current` (built 6/7) go at 17/18 — before `pVideo` (21), consistent; but
  `pGuiSystem`/`pGuiRenderer` are created inside `pVideo->Init_CEGUI()` yet destroyed by main.cpp, not by `~cVideo` (asymmetric ownership).
- `pFont` (built 4th) deleted 24th, `pFramerate` (built 5th) is **never deleted** (leaked; `rg "delete pFramerate"` → 0).
- Every delete is guarded by `if( pX )` although none can be null here; teardown is 160 lines of the same 5-line stanza ×25.

### `cObject_Manager<T>` (obj_manager.h:27–182)

Interface: `virtual void Add(T*)`, `virtual bool Delete(size_t, bool delete_data=1)`, `virtual bool Delete(T*, bool delete_data=1)`, `virtual void Delete_All()`, `virtual T *Get_Pointer(size_t) const`, `bool Switch_Array_Num(T*,T*)`, `int Get_Array_Num(T*) const`, `size()`, `empty()`, and the **public** `vector<T*> objects` (accessed directly at 63 sites outside the header — `->objects.begin()` loops everywhere). Subclasses: `cSprite_Manager`, `cCampaign_Manager`, `cObjectCollisionType` (core), `cLevel_Manager`, `cBackground_Manager`, `cHud_Manager`, `cSound_Manager`, `cImage_Manager`, `cAnimation_Manager`, `cOverworld_Manager`, `cLayer` (overworld/world_layer.h:131, multiply inherits with `CEGUI::XMLHandler`).

**Bug 1 — `Delete(size_t)` erases, then deletes the shifted element (VERIFIED, obj_manager.h:40–54):**
```cpp
virtual bool Delete( size_t array_num, bool delete_data = 1 )
{
    if( array_num < objects.size() )
    {
        objects.erase( objects.begin() + array_num );   // removes element N; N+1.. shift down
    }
    if( delete_data )
    {
        delete objects[array_num];                        // deletes the element that is NOW at N (formerly N+1)
    }                                                     // — or reads past the end if N was the last one
    return 1;
}
```
The pointer that was erased is never deleted (leak); the next one is deleted while still in the vector
(dangling). If `array_num >= size()` the erase is skipped and `objects[array_num]` is an out-of-bounds
read. **Live impact: none today** — no caller passes an index (`rg "Delete\( *(i|n|num|index|array_num|identifier|pos|[0-9]+) *[,)]"` → 0 matches); every `Delete(` call passes a `T*`. As a virtual member of a class template it is still compiled into every subclass vtable (GCC instantiates virtuals with the class), so it is reachable through any `cObject_Manager<T>*`.

**Bug 2 — `Switch_Array_Num` assigns raw pointers to iterators (VERIFIED, obj_manager.h:110–145):**
```cpp
typename vector<T *>::iterator itr1 = std::find( objects.begin(), objects.end(), obj1 );
…
typename vector<T *>::iterator itr2 = std::find( objects.begin(), objects.end(), obj2 );
…
// todo: does that work ?
itr1 = obj2;
itr2 = obj1;
return 1;
```
It never swaps anything: it (tries to) reassign the local iterators, not `*itr1`/`*itr2`. With libstdc++'s `__normal_iterator<T**, …>` the assignment from a `T*` does not even compile (its converting ctor is `explicit` and takes `T**`). It survives only because **no code calls it** (`rg Switch_Array_Num` → the definition only) and a non-virtual member of a class template is never instantiated unless used. Correct body: `std::iter_swap( itr1, itr2 )`.

Also: `Delete(T*)` (57–81) and `Delete(size_t)` `delete` even when the object was **not** in the vector (the erase is conditional, the delete is not), and both unconditionally `return 1`. `Get_Array_Num` returns `int` from `std::distance` (narrowing).

### `cSprite_Manager` (sprite_manager.h/.cpp)

- **Ownership**: it owns every `cSprite*` in `objects` except those flagged `m_disallow_managed_delete` (sprite.h:521; set for `pLevel_Player` in main.cpp:300). `~cSprite_Manager` → `Delete_All()` (cpp:37–40). `Delete_All(delayed=0)` first **erases** (without deleting) the unmanaged ones (cpp:206–220), then `cObject_Manager::Delete_All()`; `Delete_All(1)` only calls `obj->Destroy()` on each (sets `m_auto_destroy`) — **no caller uses the delayed form** (`rg "Delete_All\( *(1|true)"` → 0), so that branch is dead.
- **`Add`** (cpp:42–71): assigns z via `Set_Pos_Z`, then **reuses the slot of the first `m_auto_destroy` sprite** (`*itr = sprite; delete obj; return;`) before appending — so array order is not insertion order, and a destroyed sprite is freed lazily by the next `Add` rather than by an update pass.
- **z-order**: `m_z_pos_data[type]`/`m_z_pos_data_editor[type]` are `vector<float>(100)` indexed by `SpriteType` (values go up to 100 = `TYPE_MOUSECURSOR` → `m_z_pos_data[100]` would be **one past the end**; only reachable if a mouse-cursor sprite were added, which does not happen). `Set_Pos_Z` bumps a sprite's z by `0.000001f` above the running max for its type (cpp:83–119) — the float-epsilon z-stacking scheme. Animations/particle emitters are exempt.
- **`Move_To_Front`/`Move_To_Back`** (cpp:121–187): erase + re-insert at index 0 / end, then set z to `Get_First/Get_Last(type)->m_pos_z ∓ 0.000001f` (`Get_First/Get_Last` are O(n) scans by type). Two `// fixme : should not happen but it does` guards for a sprite not found in its own manager. Callers: editor.cpp:753/769 (numpad ±), sprite.cpp:1310 (`Set_Massive_Type`).
- **Typed lookups**: there is a single flat vector; `Get_Size_Array(ArrayType)` (cpp:385–398) counts `m_sprite_array == x` by linear scan — called 8× per HUD debug update (hud.cpp:1079–1105). `Get_from_Position` (savegame.cpp:386, menu_data.cpp:3620), `Get_Objects_sorted` (mouse.cpp:391, copies the whole vector then `std::sort`), `Get_Colliding_Objects` (movingsprite.cpp:416).
- **Dead**: `Copy(unsigned)` and `operator[]` — no callers (`rg "[Mm]anager(->|\.)Copy\( *[0-9a-z_]+ *\)|manager\)\["` → 0).
- `Update_Items*`/`Draw_Items`/`Handle_Collision_Items` are the per-frame drivers called from level.cpp/overworld.cpp/menu.

### `cCamera` (camera.h/.cpp)

Not owned by core: each mode owner `new`s one (`cLevel_Manager::m_camera`, `cLevel_Settings::m_camera`, `cMenuCore::m_handler->m_camera`, `cOverworld_Manager::m_camera`) and deletes it in its dtor; `pActive_Camera` is a non-owning alias. It holds a non-owning `m_sprite_manager`. `Move_to_Position_Gradually` (cpp:147–178) is a **nested frame loop**: it calls `Draw_Game()`, `pVideo->Render()`, `pFramerate->Update()` itself until the camera arrives (level_manager.cpp:422/470) — a re-entrant mini game loop inside a manager. `Update_Position` (cpp:439–468) fans out to `pMouseCursor` and every sprite's `Update_Valid_Draw` on **every** `Set_Pos*`. `m_default_limits = GL_rect(0,0,20000,-4000)` (h negative: y grows downward, limits point up).

### `cCampaign_Manager`

Owns `cCampaign` objects (`Delete_All` in dtor, cpp:96–99). `Load_Campaign` (cpp:156–170) builds a `cCampaign_XML_Handler` on the heap just to steal its `m_campaign` and delete it. `Load` (cpp:101–154) erases from `game_files` while iterating (`game_files.erase( game_itr ); break;` — safe only because of the `break`). `cCampaign::Save` is dead (no caller).

### Editor object ownership (`cEditor`)

`cEditor` owns: the CEGUI window tree loaded from `editor.layout` (`m_editor_window`, destroyed in `Unload`, cpp:283–291), `m_tagged_item_objects` (prototype `cSprite*`s created by the subclass's `Get_Object` during XML parse, deleted in `Unload`, cpp:294–299), `m_tagged_item_images` (`cImage_Settings_Data*` returned by `pSettingsParser->Get`, deleted in `Unload`, cpp:302–307). Each listbox row is a `cEditor_Item_Object` **owned by CEGUI's Listbox** (`addItem`, `resetList` frees them) and that item owns a **copy** of the prototype sprite (`sprite_obj`, deleted in its dtor, cpp:118–121) plus a `cEditor_CEGUI_Texture` wrapper whose `cleanupOpenGLTexture` override (cpp:77–90) deliberately does **not** `glDeleteTextures` — the GL texture belongs to `cGL_Surface`. `Activate_Item` (cpp:1343–1379) copies the row's sprite again and hands it to `m_sprite_manager->Add` — from then on the level owns it. `m_sprite_manager` is non-owning and re-pointed by `Set_Sprite_Manager` on every level/world switch.

## 4. The game loop and modes

**`GameMode`** (global_game.h:106–113): `MODE_NOTHING, MODE_LEVEL, MODE_OVERWORLD, MODE_MENU, MODE_LEVEL_SETTINGS`. **`GameModeType`**: `MODE_TYPE_DEFAULT | MODE_TYPE_LEVEL_CUSTOM` (custom = level loaded outside a world; used by savegame/menu to decide return targets). **`GameAction`** (123–132): `GA_NONE, GA_ENTER_MENU, GA_ENTER_WORLD, GA_ENTER_LEVEL, GA_DOWNGRADE_PLAYER, GA_ACTIVATE_LEVEL_EXIT, GA_ENTER_LEVEL_SETTINGS`.

**Deferred-action mechanism** (game_core.cpp:75–141): any subsystem sets `Game_Action = GA_*` and stuffs
string key/values into three `CEGUI::XMLAttributes` bags (`_Start`, `_Middle`, `_End`) plus an untyped
`Game_Action_ptr`. `Handle_Game_Events` (called once per frame at the top of `Update_Game`, main.cpp:680)
loops `while( Game_Action != GA_NONE )`: it **copies** all five into locals, clears the globals (so a
handler may queue a follow-up action), then:
- `GA_DOWNGRADE_PLAYER`: Start → `pLevel_Player->DownGrade_Player(0, middle["downgrade_force"])` → Middle → End.
- `GA_ACTIVATE_LEVEL_EXIT`: Start → `static_cast<cLevel_Exit*>(Game_Action_ptr)->Activate()` → Middle → End.
- all `GA_ENTER_*`: Start → `Leave_Game_Mode(new)` → Middle → `Enter_Game_Mode(new)` → End.

`Handle_Generic_Game_Events` (143–281) is the string-keyed interpreter: keys `music_fadeout, reset_save,
unload_levels, unload_menu, load_menu(+menu_exit_back_to, menu_start_current_level), enter_world,
world_player_waypoint, new_level, load_level(+load_level_entry), load_savegame, play_music(+music_loops,
music_force, music_fadein), screen_fadeout(+_speed), screen_fadein(+_speed), activate_level_entry,
activate_editor`. Enum values travel as decimal strings (`intToString(EFFECT_OUT_BLACK)`, main.cpp:212)
— a stringly-typed command bus with no schema; misspelled keys are silently ignored.

`Leave_Game_Mode` (283–301) / `Enter_Game_Mode` (303–330) dispatch on the current/new mode to
`pActive_Overworld->Leave/Enter`, `pActive_Level->Leave/Enter`, `pMenuCore->Leave/Enter`,
`pLevel_Editor->m_settings_screen->Leave/Enter`; `Enter_Game_Mode` is the **only writer of `Game_Mode`**.
Both are `if/else if` chains with no final `else` (`MODE_NOTHING` falls through silently).

**Main loop** (main.cpp:218–234): `while( !game_exit ) { Update_Game(); Draw_Game(); pVideo->Render(); pFramerate->Update(); }`.
`Update_Game` (655–718): fps cap (`Correct_Frame_Time(100)` in menu without vsync, else `m_video_fps_limit`) → `pVideo->Render_Finish()` if an action is pending → `Handle_Game_Events` → `while(SDL_PollEvent(&input_event)) Handle_Input_Global(&input_event)` → `pMouseCursor->Update` → `pAudio->Resume_Music(); pAudio->Update()` → mode dispatch (`pLevel_Manager->Update`, `pActive_Overworld->Update`, `pMenuCore->Update`, `pLevel_Editor->m_settings_screen->Update`) → `Gui_Handle_Time()`.
`Draw_Game` (720–753): same mode dispatch to `Draw`, then `pMouseCursor->Draw()`, then `PERF_DRAW_MOUSE` timer. Both are `if/else if` chains without default; both early-return on `game_exit`.

`Handle_Input_Global` (522–653): SDL2-ported switch — `SDL_QUIT` sets `game_exit=1` + `Clear_Input_Events`; `SDL_WINDOWEVENT_SIZE_CHANGED` → `pGuiSystem->notifyDisplaySizeChanged`; `SDL_WINDOWEVENT_FOCUS_LOST` pauses music and **blocks in `SDL_WaitEvent(NULL)`** until any event; `SDL_TEXTINPUT/KEYDOWN/KEYUP` → `pKeyboard`; joystick events → `pJoystick`; `default` → `pMouseCursor->Handle_Event`, then per-mode editor/menu `Handle_Event`. Returns `bool` as `0/1` (never consumed — no callers outside main).
`Clear_Input_Events` (game_core.cpp:332–344): drains `SDL_PollEvent` and resets keyboard/mouse/joystick key state; called from 6 other files to swallow input across fades.

Argument parsing (main.cpp:108–179) has two bugs: line 142 `for( unsigned int option = i + i; i < arguments.size(); i++ )` — `option` is `2*i` (typo for `i + 1`) and never advances, so `--debug game` reads `arguments[2*i]` (out of range for `-d` not at index 1) and re-processes the same token until `i` runs out; line 168 `arguments[1] == "--world"` tests index 1 instead of `i`.

## 5. The editor (`core/editor.cpp`)

`cEditor` is both a `CEGUI::XMLHandler` (parses `editor_items` XML → `Handle_Item` → subclass `Get_Object` prototypes; and `editor_menu` XML → `Handle_Menu` → `Add_Menu_Object`) and the runtime controller. Hooks into the mode owners: `cEditor_Level`/`cEditor_World` override `Init`, `Enable`, `Disable`, `Key_Down`, `Set_Sprite_Manager`, `Activate_Menu_Item`, `Get_Object`, and the `Function_*` menu verbs (level_editor.h:27–80, world_editor.h:26–70); `Handle_Input_Global` routes SDL events to `pLevel_Editor/pWorld_Editor->Handle_Event`; `cLevel::Update/Draw`, `cOverworld::Update/Draw` and `cLevel_Manager` call `pLevel_Editor->Update/Draw/Process_Input` (16 call sites across level.cpp ×6, overworld.cpp ×7, level_manager.cpp ×2, world_manager.cpp ×1). `Function_Exit` (cpp:1617) fakes an **F8 keypress** through `pKeyboard->Key_Down( SDLK_F8 )` to close the editor.

CEGUI usage: `subscribeEvent` **7×** in editor.cpp (2 tab-content `EventMouseEnters`, 2 listbox `EventMouseEnters`, 2 `EventSelectionChanged`, 1 help-window close `EventClicked`) out of 216 across src; plus `WindowManager::getSingleton()` ×13, `loadWindowLayout("editor.layout")`, `createWindow("TaharezLook/FrameWindow"|"StaticText")` for the F1 help box (cpp:650–714, 45 lines of literal help text), `ImagesetManager::create/destroy` for per-row preview thumbnails, custom `ListboxItem` subclasses (`cEditor_Item_Object` draws an image + text, `getPixelSize`/`draw` overrides), `CEGUI::PropertyHelper::stringToColour`. All `static_cast<>` from `CEGUI::Window*`/`ListboxItem*` to concrete types; all windows looked up by **string name** (`"tabcontrol_editor"`, `"editor_menu"`, `"editor_items"`, `"editor_help_window"`).

Object menus: `Add_Menu_Object` parses a tag string with prefixes `function:`/`header` by `find()==0` and `erase(0,N)` (magic 9 and 6); `Load_Item_Menu` splits `;`-separated tags by hand (cpp:1118–1138) then, for every tagged image-settings entry and every prototype sprite, runs `Is_Tag_Available` (a **recursive** substring matcher, cpp:1663–1689) per tag; matches create a fresh `cSprite` (images) or `object->Copy()` (prototypes) and `Add_Item_Object` guesses `m_sprite_array`/`m_type` from `m_massive_type` if unset. `Key_Down` (cpp:619–919) is a 300-line `if/else if` keymap (Ctrl+N/S/Shift+S, F1, Home, N/P, Ctrl+G, KP±, fast-copy keys, pixel-move keys, Ctrl+A/Shift+A/V/Insert/X/C/R, Del, O); note Ctrl+Shift+S (637) is unreachable because Ctrl+S (632) matches first. `Handle_Item(const XMLAttributes &attributes)` ignores its parameter and reads `m_xml_attributes` (cpp:1745–1766).

## 6. Data/property helpers and filesystem

**XML layer**: SMC does not own an XML API — every loader is a `CEGUI::XMLHandler` subclass that copies `<property name= value=>` pairs into a `CEGUI::XMLAttributes` bag and reads them back with `getValueAsString/Integer/Float/Bool` (37 files); writers use `CEGUI::XMLSerializer` via `Write_Property` (game_core.cpp:618–627; overloads for `CEGUI::String/int/unsigned/Uint64/long/float` in game_core.h:105–126, using `CEGUI::PropertyHelper::intToString/uintToString/floatToString` — 92 `CEGUI::PropertyHelper::` lines across src). Newlines are round-tripped as literal `<br/>` (`Write_Property` ↔ `xml_string_to_string`). Replacing CEGUI's XML means touching all 37 handler classes, not just this file.

**property_helper** conversions: `int_to_string`/`int64_to_string`/`long_to_string` = `ostringstream` (195/1/1 call sites); `float_to_string` (cpp:103–251) is a 150-line BSD-licensed copy of stringencoders' `modp_dtoa` with a 64-byte `char str[64]` and a `strreverse` (72 call sites); `string_to_*` go through `template from_string` + `istringstream` (`string_to_int` 62, `string_to_float` 78, `string_to_int64` 4, `string_to_long` 1, **`string_to_double` 0 — unused**); `string_to_version_number` (1 caller, preferences.cpp:472); `Time_to_String` = `strftime` into `char str_time[60]` (2 callers). Enum↔name: 6 `switch`-based `*_Name` and 7 `if/else if strcmp`-chain `*_Id` functions (`Get_Direction_Id` alone is 18 branches, 32 callers; `Get_Ground_Type_Name` has **0** callers). `Get_Sprite_Color` (1 caller). Win32-only `ucs2_to_utf8`/`utf8_to_ucs2` (3/18 sites, all under `#ifdef _WIN32`).

**String helpers — one real bug**: `string_trim_from_begin` (h:46–49) is implemented as `str.erase( str.find_last_not_of( search ) + 1 )`, i.e. it trims the **end**; `string_trim_from_end` (cpp:48–62) takes its argument **by value and returns** the trimmed copy, so the call `string_trim_from_end( str, search );` inside `string_trim` (h:57–61) and in file_parser.cpp:74 **discards the result**. Net effect: `string_trim` (callers level.cpp:99, world_manager.cpp:53) and `cFile_parser::Parse_Line` trim trailing characters only; leading spaces survive. The five `todo : use boost::algorithm::…` comments (h:30–55) are never acted on — Boost.StringAlgo is **not** used anywhere.

**Boost call sites (exact, whole src)**: filesystem.cpp:19 `#include "boost/filesystem.hpp"`, :20 `namespace fs = boost::filesystem;`, :53/:55 `fs::status(fs::path(…)).type()` + :58 `fs::regular_file|fs::symlink_file`, :65/:67/:70 same for `directory_file`, :97/:99 `fs::remove_all`, :124 `fs::create_directory`, :132/:134 `fs::create_directories`, :178/:180 `fs::path`, :182/:185 `fs::directory_iterator`, :189 `dir_itr->path().filename().string()`, :192 `fs::is_directory`, :245 `fs::temp_directory_path().generic_string()`. Outside this subsystem: video.h:41 `<boost/thread/thread.hpp>`, video.h:260 `boost::thread m_render_thread`, video.cpp:76/:1026 (the `SMC_RENDER_THREAD_TEST` experiment, off by default). So `<filesystem>` replaces **all** Boost use in core, and `std::thread` the rest of the game's.

**filesystem.cpp shape**: every function is `#ifdef _WIN32 … utf8_to_ucs2 … #else …` twice-written (9 `fixme : boost should use a codecvt_facet` comments); the non-Windows branches mix boost (`status`, `remove_all`, `create_directory`) with raw POSIX (`remove`, `rmdir`, `rename`, `stat`, `getenv("HOME")`). Call-site counts (outside the file): `Trim_Filename` 52, `File_Exists` 35, `Get_Directory_Files` 12 (recursive, builds `dir + "/" + name` strings, catches `std::exception` per entry), `Dir_Exists` 4, `Convert_Path_Separators` 4, `Delete_File` 3, `Create_Directory` 3, `Rename_File`/`Create_Directories`/`Delete_Dir_And_Content`/`Get_Temp_Directory`/`Get_User_Directory` 1 each, **`Delete_Dir` 0, `Get_File_Size` 0** (dead). `Trim_Filename( std::string, bool keep_dir, bool keep_end )` is the poor man's `path::filename()/stem()`; `Get_User_Directory` returns `$HOME/.smc/` (no XDG). A `<filesystem>` port touches these 15 functions plus `cResource_Manager` (string concatenation of `user_data_dir + USER_*_DIR` at 24 sites, all assuming a trailing `/`).

## 7. Math (`core/math/*`)

Present: `GL_rect` (rect.h; `Intersects(x,y)`, `Intersects(rect)`, `Get_Rect` → `SDL_Rect`, `Get_Rect_pos`, `Get_pos_middle`, `clear`, arithmetic ops, hand-written copy-assignment), `GL_point` (point.h; `rotate`, `rotate3d`, `distance`, ops, `#define M_PI`), `GL_line` (line.h; `Get_Intersection`, `Intersects`, `Distance_to_Line`, `pow2`, `cl_min`), `cSize_Int`/`cSize_Float` (size.h), `GL_Vector` (vector.h/.cpp, 4-component ClanLib port), `utilities.h` (`Clamp`, `Get_Random_Float`, `Get_Power_of_2`, `Is_Float_Equal`, `Is_Valid_Number` + a global-namespace functor `nondigit` in utilities.cpp:26).

Used (lines outside `core/math/`): `GL_rect` 102, `Intersects(` 29, `Is_Float_Equal` 62, `Get_Random_Float` 30, `cSize_Int` 21, `GL_point` 20, `Is_Valid_Number` 11, `GL_line` 10 (world_layer.cpp:480–540 `line_1.Intersects(&map_line)`, video/renderer `Draw_Line`), `Get_Power_of_2` 6, `Clamp` 5, `Get_pos_middle` 2, `.rotate(` 2, `M_PI` 6 (but **re-`#define`d** in renderer.cpp:26 and movingsprite.h:24 as well).

Unused: `GL_Vector` methods `norm/normalize/dot/angle/cross/rotate/round` (0 each; the class is only a 2-float carrier in mouse.cpp:701–746 — could be `GL_point`), `cSize_Float` (0 instantiations; only the fwd decl), `GL_line::Get_Intersection` 0, `Distance_to_Line` 0, `GL_point::rotate3d` (3 commented-out lines only), `GL_point::distance` (used only inside `Distance_to_Line`), `GL_rect::Get_Rect`/`Get_Rect_pos` 0 (SDL_Rect conversions, dead since SDL 1.2 blitting went away), `pow2`/`cl_min` (internal to line.h; duplicate `std::min`). Every `==` uses `Is_Float_Equal` with tolerance 1e-4 while `Set_Pos_Z` stacks sprites 1e-6 apart — the tolerance is 100× the z-spacing.

## 8. Old-pattern counts (scope = the 27 files, 7 879 lines; `S` = that list; `rg -n PATTERN $S | wc -l`)

| pattern | `rg` | count |
|---|---|---|
| raw `new` | `\bnew [A-Za-z_:]+` | 47 |
| raw `delete` | `\bdelete(\[\])? ` | 43 |
| `NULL` | `\bNULL\b` | 73 |
| `typedef` | `\btypedef\b` | 3 (+4 duplicated `typedef SDL_Keycode SDLKey` in global_basic.h) |
| iterator for-loops | `for\(.*::(const_)?iterator` | 40 (46 iterator declarations total) |
| `virtual` vs `override` | `\bvirtual\b` / `\boverride\b` | 42 / **0** |
| unscoped `enum` | `^\s*enum [A-Za-z_]+` | 1 in scope (`Camera_movement`, camera.h:26) — plus 17 in global_game.h; `enum class` 0 |
| `#define` | `^\s*#define` | 17 in scope; global_game.h/global_basic.h add ~35 (`DATA_DIR`, `*_DIR`, version, `debug_print`) |
| `bool x = 0/1` | `\bbool [A-Za-z_]+ = [01]\b` | 24 declarations; `return [01];` 74 (bool functions returning ints) |
| `sprintf`/char buffers | `sn?printf\(` / `\bchar [A-Za-z_]+\[[0-9]+\]` | 0 / 3 (`char path[1024]` main.cpp:66, `char str[64]` property_helper.cpp:119, `char str_time[60]` :418); `printf(` 41 (all logging is `printf`) |
| C-style casts | `\((const )?(int|float|…|std::string|const CEGUI::utf8 ?\*)\)[ (A-Za-z_]` | 7 (e.g. `(char)(48 + …)` property_helper.cpp:213/228/241, `(const std::string)ex.getMessage()` campaign_manager.cpp:206, `(std::string)getenv(…)` filesystem.cpp:271/273, `(const CEGUI::utf8*)filename` :197); `static_cast<` 79, `reinterpret_cast<` 1 |
| `( void )` parameter lists | `\( void \)` | 120 |
| `std::string` by-value params | `\( std::string [a-z_]+` | 14 |
| `using namespace` | | 1 (`using namespace SMC;` main.cpp:51) |
| `rand()` / `time( NULL )` / `strftime`+`localtime` | | 2 / 2 / 1 |
| todo / fixme / hack | `(?i)todo` etc. | 12 / 15 / 5 (listed in §9) |

## 9. Dead or suspicious code

- **Dead functions**: `cObject_Manager::Switch_Array_Num` (0 callers, would not compile — §3); `cSprite_Manager::Copy(unsigned)` and `operator[]`; `cSprite_Manager::Delete_All(delayed=1)` branch; `cCampaign::Save`; `string_to_double`; `Get_Ground_Type_Name`; `Delete_Dir`; `Get_File_Size`; `GL_Vector::{norm,normalize,dot,angle,cross,rotate,round}`; `GL_line::{Get_Intersection,Distance_to_Line}`; `GL_point::rotate3d`; `GL_rect::{Get_Rect,Get_Rect_pos}`; `cSize_Float`; `cEditor::Get_Object` base body (`return NULL`, cpp:1381–1385, both subclasses override).
- **Dead globals**: `editor_level_enabled`, `editor_world_enabled` never written (§2) — 37 reader lines in 14 files are dead branches; `game_res_w/h` never written (could be `constexpr`).
- **Never freed**: `pFramerate` (no `delete pFramerate` anywhere).
- **Duplicated blocks**: `global_basic.h:51–62` (SDL2 typedef ×4); `#ifdef _WIN32` twin bodies ×9 in filesystem.cpp; `Copy_Direction(list)` first/last search written twice for X and Y (editor.cpp:1393–1446); `Process_Input` camera moves = 4× the same 8-line shift/no-shift block (editor.cpp:546–589); `Preload_Images` = 80 `push_back` literals incl. 40 gold-piece frames that are `1..10` × `{"", "_falling"}` × `{yellow, red}` (game_core.cpp:377–459); `Preload_Sounds` 70 more literals; 7 `if( str.compare("x") == 0 ) return ENUM_X;` chains in property_helper.cpp (cpp:468–871, ~330 lines) that are inverse tables of the 6 `switch` `*_Name` functions; `Get_First`/`Get_Last` identical bodies except `<`/`>`; `cCamera::Set_Pos/Set_Pos_X/Set_Pos_Y` triplicated guard; main.cpp `Exit_Game` 25 identical `if(p){delete p;p=NULL;}` stanzas.
- **Commented-out code**: game_core.cpp:388 (`feather_1.png`), :521 (`maryo_au.ogg`), filesystem.cpp:263–267 (Win32 `WriteConsole` debug), point.h:56–57 `/*+ 0.5*/`, :74, vector-style `//tmp.z`, world_editor.h:63/66 (`//virtual void Function_Save_as`, `//void Function_Settings`).
- **todo/fixme (all 32 lines, `rg -n -i "todo|fixme|hack" $S`)**: main.cpp:62 `remove this apple hack`; game_core.cpp:336 `keep Windowmanager quit events ?`, :520 `create again`; obj_manager.h:140 `does that work ?` (it does not); camera.cpp:358 `remove hackiness` (fixed-scroll kill: `DownGrade_Player(1,1)` from inside the camera); sprite_manager.cpp:143/177 `should not happen but it does`; editor.h:52 CEGUI-0.7.5 texture workaround; editor.cpp:232 (CEGUI mouse-enter), :411 (fade-out can fire enter event), :1597 `todo : clear`; campaign_manager.cpp:44 (`imbue`), :195 (utf8 → CEGUI::String); property_helper.h:30/36/44/51/55 (use boost algorithms), .cpp:105/108 (NaN hack); filesystem.cpp ×9 `codecvt_facet`; vector.cpp:74 `quick hack`.
- **Suspicious logic**: `main.cpp:142` `option = i + i` and `:168` `arguments[1]` (§4); `editor.cpp:637` unreachable Ctrl+Shift+S; `editor.cpp:1148` `array_tags[tag_pos]` indexed before the `tag_pos >= size()` check moves it (safe only because `Is_Tag_Available` is called first with `tag_pos` < size); `string_trim` family (§6); `Get_from_Position` `check_pos == 2` condition `x != sx && y != sy` (sprite_manager.cpp:277) is `&&` where the intent reads as `||`; `Set_Pos_Z` indexes `m_z_pos_data[sprite->m_type]` with a 100-entry vector while `TYPE_MOUSECURSOR = 100`; `Handle_Input_Global` `FOCUS_LOST` blocks the whole loop in `SDL_WaitEvent(NULL)` (main.cpp:550); `cCamera::Move_to_Position_Gradually` re-enters the render loop (camera.cpp:147–178); `cCampaign_XML_Handler` ctor parses XML and swallows `CEGUI::Exception` leaving a half-built `m_campaign` that `Load_Campaign` still returns; `m_last_saved = string_to_int64(...)` assigns `Uint64` into `time_t`; `Get_Array_Num` returns `int` from `ptrdiff_t`.

## 10. Modern-C++ hot spots (top five)

1. **`Exit_Game`/`Init_Game` manual singleton lifetime → RAII / `std::unique_ptr` members of one `cApplication`** (main.cpp:244–520, 25 `new` + 25 guarded `delete` stanzas, 1 leak, destruction order hand-ordered and wrong in ≥5 places). Replace the 24 `extern cX *pX` globals with owned members constructed in dependency order; destructors then run in reverse automatically. Sites: main.cpp only for the alloc/free, but the `extern` declarations live in 22 headers and are read at ~1 000 lines across src, so do it as `pX` → accessor first, ownership second.
2. **`cObject_Manager<T>` (`vector<T*>` + manual `delete`) → `std::vector<std::unique_ptr<T>>` with `std::erase_if`/`std::iter_swap`** (obj_manager.h, 13 subclasses; fixes both verified bugs by construction; 63 direct `->objects` loops become range-for). The `m_disallow_managed_delete` escape hatch in `cSprite_Manager::Delete_All` (sprite_manager.cpp:206–220) becomes "the player is never in the vector" instead of "in the vector but unowned".
3. **`Game_Action` + three `CEGUI::XMLAttributes` string bags → a `std::variant` of typed action structs** (game_core.cpp:75–281; 54 `Game_Action = GA_*` sites in 9 files, ~180 `Game_Action_Data_*` lines). Every key in `Handle_Generic_Game_Events` becomes a field; enum-as-decimal-string (`intToString(EFFECT_OUT_BLACK)`) disappears; `Game_Action_ptr` (`void*` → `cLevel_Exit*`) becomes a typed member. Also converts the 4 `if/else if` mode chains (`Handle_Game_Events`, `Leave/Enter_Game_Mode`, `Update_Game`, `Draw_Game`) into `switch` over `enum class GameMode` with a `default`.
4. **`boost::filesystem` + `#ifdef _WIN32` twins + string-concatenated paths → `std::filesystem::path`** (filesystem.cpp: 15 functions, 26 `fs::` lines, 9 Win32 branches; `Trim_Filename` ×52 → `path::filename()/stem()`; `resource_manager.cpp` + 24 `user_data_dir + USER_*_DIR` concatenations → `path /` ; `Get_Directory_Files` → `recursive_directory_iterator`). Drops the game's only Boost dependency besides the off-by-default `boost::thread`.
5. **property_helper conversions and enum tables → `std::to_chars/from_chars`, `std::format`, and constexpr name tables** (`float_to_string` 150-line dtoa copy with `char[64]` → `std::to_chars` (72 sites); `int_to_string` ×195 / `string_to_int` ×62 / `string_to_float` ×78 → `std::to_string`/`from_chars`; 7 `compare(...) == 0` chains ≈330 lines → one `constexpr std::array<std::pair<Enum, std::string_view>>` per enum with a shared lookup template, also giving `*_Name` for free; `Time_to_String` `char[60]` → `std::format("{:%Y-%m-%d %H:%M:%S}")`). Fix `string_trim` while there (or drop it for `std::string_view` trimming).

Honourable mentions: 42 `virtual` / 0 `override` (add `override`, mark `cEditor` hooks `final` where appropriate); 120 `( void )`; `bool` returned as `0/1` at 74 sites; `printf` logging ×41 → one logger; `GL_Vector`/`cSize_Float`/dead `GL_line` methods → delete; `Preload_Images` gold-piece list → generated with a loop or `std::format`.

## 11. Cross-references

**This subsystem depends on** (includes from scope files, `rg '#include "\.\./'` grouped): `input/` 11 (keyboard, mouse, joystick — game_core, editor, camera, sprite_manager, main), `overworld/` 10 (overworld, world_manager, world_editor, world_player), `level/` 10 (level, level_player, level_editor, level_manager), `video/` 8 (video, renderer, font, animation, img_settings, color), `gui/` 8 (menu, menu_data, hud, generic), `objects/` 5 (sprite, movingsprite, level_exit), `audio/` 5, `user/` 4 (preferences, savegame). Third-party seams: CEGUI (`XMLAttributes`, `XMLSerializer`, `XMLHandler`, `PropertyHelper`, `WindowManager`, `Listbox*`, `OpenGLTexture`, `ProgressBar`, `System`, `Logger`), SDL2 (`SDL_Event`, `SDL_PollEvent`, `SDL_WaitEvent`, `SDL_GetTicks`, `SDL_Rect`, `IMG_Quit`), Boost.Filesystem (filesystem.cpp only), libintl via i18n.h, CoreFoundation (`__APPLE__` block in main.cpp). `game_core.h` transitively pulls `objects/sprite.h` and `camera.h` into all 67 of its includers — the main reason "everything includes everything".

**Depends on this subsystem**: everything — `game_core.h` (67 includers), `global_basic.h` (61), `i18n.h` (48), `framerate.h` (34), `filesystem.h` (28), `sprite_manager.h` (24), `math/utilities.h` (17), `obj_manager.h` (12), `resource_manager.h` (11), `main.h` (9: camera.cpp plus level/overworld/gui/user callers of `Draw_Game`?—no: `rg` shows only camera.cpp calls into main.h's functions; the other 8 include it without using it), `camera.h` (9), `math/rect.h` (6), `editor.h` (4: level_editor, world_editor, sprite.cpp, mouse), `campaign_manager.h` (3), `math/size.h` (3), `math/point.h` (3), `math/vector.h` (2: mouse.h + vector.cpp), `math/line.h` (1: world_layer). Editor ↔ `objects/sprite` is bidirectional: sprite.cpp `new`s `cEditor_Object_Settings_Item` rows (sprite.cpp:1487) while editor.cpp calls `sprite->Editor_*`/`Copy`/`Set_Sprite_Manager`; camera ↔ level_player is bidirectional (camera.cpp reads `m_velx/m_vely/m_no_vel*_counter` and calls `DownGrade_Player`).
