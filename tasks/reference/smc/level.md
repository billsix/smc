# `level/` — verified source analysis

Scope: `smc/src/level/` (12 files, 9 592 lines). Every claim is anchored `file:Symbol` (line numbers
are from the checkout at HEAD `d438165 update to SDL2`; `git status` on `level/` is clean). "Live"
means a caller was found by `rg` outside the definition; "dead" means none was found.

## 1. Files and roles

| File | Lines | Role | Status |
|---|---|---|---|
| `level.h/.cpp` | 218 / 1 980 | `cLevel` (a CEGUI `XMLHandler`): owns the three managers, XML load/save, enter/leave, update/draw, input dispatch; plus the free factory `Create_Level_Object_From_XML` (level.cpp:1218-1972, 750 lines of version-migration + `new`). | live |
| `level_manager.h/.cpp` | 92 / 521 | `cLevel_Manager : cObject_Manager<cLevel>`: level list, path lookup, per-frame `Update`/`Draw` orchestration, `Finish_Level`, `Goto_Sub_Level`. | live |
| `level_player.h/.cpp` | 335 / 4 585 | `cLevel_Player : cAnimated_Sprite`: Maryo — the movement/power-up state machine, item holding, fireballs, collision responses, death sequence. | live |
| `level_settings.h/.cpp` | 100 / 612 | `cLevel_Settings`: the editor's CEGUI "level settings" screen (`MODE_LEVEL_SETTINGS`). | live (editor only) |
| `level_background.h/.cpp` | 138 / 409 | `cBackground` (gradient or tiled parallax image layer) + `cBackground_Manager`. | live |
| `level_editor.h/.cpp` | 89 / 513 | `cEditor_Level : cEditor`: level-specific editor menu functions, object-state cycling, `Get_Object` → factory. | live (editor only) |

Dead/vestigial *inside* live files (evidence in §10): `cLevel::Process_Input` (empty body), `cLevel_Manager::Init` (empty), `cLevel::m_next_level_filename`/`m_delayed_unload` (never set), `cLevel_Player::Copy` (returns NULL), `cLevel_Player::Action_Stop_Shoot` (empty), `level_editor.cpp:214` unreachable `else if`.

## 2. `cLevel`

**Owns** (all raw `new` in `cLevel::cLevel` level.cpp:74-76, `delete` in `~cLevel` level.cpp:90-92):
- `m_sprite_manager` (`cSprite_Manager*`) — every level object; `Unload` calls `m_sprite_manager->Delete_All()` last (level.cpp:269).
- `m_background_manager` (`cBackground_Manager*`) — layer 0 is always a `BG_GR_VER` gradient added in the ctor (level.cpp:79-82) and re-added after `Delete_All` in `Unload` (level.cpp:247-250); `cLevel_Settings` and `elementEnd` rely on `Get_Pointer(0)` being that gradient (level.cpp:1153, level_settings.cpp:175).
- `m_animation_manager` (`cAnimation_Manager*`).
- Settings fields (level.h:166-191): `m_engine_version` (**-1 = not loaded**, `Is_Loaded` level.cpp:1085), `m_last_saved`, `m_author`, `m_version`, `m_musicfile`+`m_valid_music`, `m_description`, `m_difficulty` (0..100), `m_land_type`, player start pos/direction, `m_camera_limits`, `m_fixed_camera_hor_vel`.
- **Not owned**: the player. `pLevel_Player` is a process-global created in `core/main.cpp:299` with a NULL sprite manager and re-pointed at each level's manager by `cLevel::Set_Sprite_Manager` (level.cpp:448). `cLevel::Save` writes `pLevel_Player->m_start_pos_x` (level.cpp:349), not the level's own `m_player_start_pos_x` — saving a non-active level would record the wrong player position.

**Load** (`cLevel::Load` level.cpp:164): resolves via `pLevel_Manager->Get_Path`, `Unload()`, then `CEGUI::System::getSingleton().getXMLParser()->parseXMLFile(*this, filename, "…/Level.xsd", "")` (level.cpp:184/186) — `cLevel` *is* the SAX handler. Only `.smclvl` is accepted; anything else prints "Unsupported Level format" (level.cpp:200). After parse, `obj->Init_Links()` over all sprites (level.cpp:207-212).

**XML schema** (`<level>` root, level.cpp:303-372; every element is a bag of `<property name= value=>` children collected by `elementStart` level.cpp:1096 and consumed in `elementEnd` level.cpp:1104):
- `information`: `game_version`, `engine_version`, `save_time`.
- `settings`: `lvl_author`, `lvl_version`, `lvl_music`, `lvl_description`, `lvl_difficulty`, `lvl_land_type`, `cam_limit_{x,y,w,h}`, `cam_fixed_hor_vel`.
- `background` (0..n): `type` (int of `BackgroundType`), gradient colours or image/pos/speed/const_vel (`cBackground::Load_From_XML` level_background.cpp:67).
- `player`: `posx`, `posy`, `direction`.
- Object elements, gated by `cLevel::Is_Level_Object_Element` (level.h:135, a 20-string `||` chain that the factory comment says to "keep in synch" level.cpp:1969).

**Factory `Create_Level_Object_From_XML`** (level.cpp:1218) — element → class, all `new`ed here:
| element | attr `type` | class | file:line |
|---|---|---|---|
| `sprite` | – | `cSprite` | level.cpp:1308 (plus up to 5 extra `Copy()`d tiles for v<31/v<32 sliders, 1405-1445) |
| `enemystopper` | – | `cEnemyStopper` | 1462 |
| `levelexit` | – | `cLevel_Exit` | 1485 |
| `level_entry` | – | `cLevel_Entry` | 1498 |
| `box` | `bonus`/`gold`/`empty`/`invisible` | `cBonusBox` | 1515/1531/1548/1558 |
| `box` | `spin` | `cSpinBox` | 1535 |
| `box` | `text` | `cText_Box` | 1539 |
| `item` / `powerup` | `goldpiece`/`mushroom`/`fireplant`/`jstar`/`moon` | `cGoldpiece`/`cMushroom`/`cFirePlant`/`cjStar`/`cMoon` | 1581-1597 |
| `moving_platform` | – | `cMoving_Platform` | 1623 |
| `falling_platform` | – | `cMoving_Platform` (attrs rewritten: `speed=0`, `touch_time`, `shake_time=12`) | 1675 |
| `enemy` | `eato`/`furball`/`turtle`/`turtleboss`/`flyon`/`thromp`/`rokko`/`krush`/`gee`/`spika`/`static`/`spikeball` | `cEato`/`cFurball`/`cTurtle`/`cTurtleBoss`/`cFlyon`/`cThromp`/`cRokko`/`cKrush`/`cGee`/`cSpika`/`cStaticEnemy`/`cSpikeball` | 1757-1819 |
| `sound` | – | `cRandom_Sound` | 1837 |
| `particle_emitter` | – | `cParticle_Emitter` (`Set_Spawned(0)`) | 1871 |
| `path` | – | `cPath` | 1888 |
| `global_effect` | – | `cParticle_Emitter` (legacy, ~20 attrs renamed, clip rect, camera-based) | 1956 |
| `ball` | – | `cBall` | 1967 |
Unknown box/item/enemy `type` → `printf` warning and fall-through to `return NULL` (1562/1601/1823). **`music` is in `Is_Level_Object_Element` but has no factory branch** → silently dropped (returns NULL, no warning). Callers of the factory: `cLevel::elementEnd` (level.cpp:1197), `cEditor_Level::Get_Object` (level_editor.cpp:289), `user/savegame.cpp:1018`.

**Version handling**: `level_engine_version = 40` (`core/global_game.h:214`). `elementEnd("information")` (level.cpp:1111) multiplies float versions `< 3` by 10 (pre-1.7 files stored `1.9` etc). Migrations are `if( engine_version < N )` blocks: 10/20 (player y ±58/−48, level.cpp:1175-1183), 21/22 (global_effect attrs), 22/23 (pipe/hill positions via 24-way `filename.compare` chains, level.cpp:1334-1378), 25 (image relocations, gumba→furball, rex→krush, thromp distance), 26-29, 31/32 (slider tiles), 34, **35 (y origin moved: every `posy` −600, at 12 separate sites)**, 36 (`motion`→`camera_motion`), 37 (fire_1 → particles), 38 (`file`→`image`). `Relocate_Image` (external helper) does the path rewrites.

**Save** (`cLevel::Save` level.cpp:272): forces the file into the user level dir, writes with `CEGUI::XMLSerializer`, skips `m_spawned || m_auto_destroy` sprites (level.cpp:362), each object's `Save_To_XML`.

**Enter/Leave** (called only from `core/game_core.cpp:313/291` inside `Enter_Game_Mode`/`Leave_Game_Mode`):
- `Enter` (level.cpp:451): `Set_Sprite_Manager()` (re-points HUD, mouse, editor, camera, player at this level's manager, level.cpp:437-449) → `pActive_Camera = pLevel_Manager->m_camera`, `pActive_Player = pLevel_Player`, `pActive_Animation_Manager = m_animation_manager` → disable world editor → show level-editor window if enabled → camera `Update_Position` (editor) or `Center` → play music if `m_valid_music` (note the warning printf reads the global `pActive_Level->m_musicfile`, not `this`, level.cpp:502) → `pHud_Manager->Update_Text()` → `pFramerate->Reset()`.
- `Leave` (level.cpp:512): no-op unless `Game_Mode == MODE_LEVEL`; resets camera limits, mouse; returns early for level→level; fades music unless going to menu/settings; `pJoystick->Reset_keys()`; hides editor window.

**Update / Draw order** — owned by `cLevel_Manager::Update` (level_manager.cpp:222): `pActive_Level->Process_Input()` (empty) → `pLevel_Editor->Process_Input()` → `pActive_Level->Update()` → `pLevel_Editor->Update()` → `pHud_Manager->Update()` → `pLevel_Player->Update()` → (if `!editor_enabled`) `pLevel_Player->Collide_Move(); Handle_Collisions()` → `pActive_Level->Update_Late()` → (if `!editor_enabled`) `m_sprite_manager->Handle_Collision_Items()` → `pActive_Camera->Update()`; a `pFramerate->m_perf_timer[PERF_*]->Update()` after each step. `cLevel::Update` (level.cpp:551): delayed-unload check, `m_next_level_filename` check (dead), then backgrounds → `m_sprite_manager->Update_Items()` → `m_animation_manager->Update()`; with the editor on, only `TYPE_PARTICLE_EMITTER` objects update (level.cpp:582-590). `cLevel_Manager::Draw` (level_manager.cpp:287): `Clear_Screen` → `Draw_Layer_1` (backgrounds, sprites, animations; level.cpp:604) → `pLevel_Player->Draw()` → `Draw_Layer_2` (only the ghost-mode full-screen modulate rect, level.cpp:635-664) → HUD → editor.

**Reload**: there is no `Reload` method; `cEditor_Level::Function_Reload` (level_editor.cpp:484) does `pActive_Level->Load(Trim_Filename(...)); pActive_Level->Init()`.

**`pActive_Level`** (defined level.cpp:1976) is written in exactly three places: `cLevel_Manager::cLevel_Manager` (level_manager.cpp:45, "always have one level around for the sprite manager"), `cLevel_Manager::Unload` (level_manager.cpp:78, back to `objects.front()`), `cLevel_Manager::Set_Active` (level_manager.cpp:138). Switching happens through the deferred `Game_Action` mechanism: a producer sets `Game_Action = GA_ENTER_LEVEL` and stuffs `Game_Action_Data_{Start,Middle,End}` (CEGUI `XMLAttributes` used as string maps), and `core/game_core.cpp:Handle_Generic_Game_Events` runs `pLevel_Manager->Load/New` + `Set_Active` + `level->Init()` on the `load_level`/`new_level` key (game_core.cpp:191-243), between `Leave_Game_Mode` and `Enter_Game_Mode`. Producers in this dir: `Goto_Sub_Level` (level_manager.cpp:497-509), `Finish_Level` (323), `cEditor_Level::Function_{New,Load,Delete,Settings}`, `cLevel_Settings::Exit`, `DownGrade_Player` (level_player.cpp:435-462), `Action_Interact(INP_EXIT)` (3585).

## 3. `cLevel_Manager`

Owns `objects` (inherited `vector<cLevel*>`, `Delete_All` in dtor level_manager.cpp:53) and `m_camera` (`new cCamera(NULL)` level_manager.cpp:36, deleted :54; becomes `pActive_Camera` if none yet). Ctor creates the permanent first `cLevel` (:45-48).
- `Get_Path(filename&, check_only_user_dir)` (level_manager.cpp:158): mutates its argument. Trims to bare name, tries `<user_data_dir>/<USER_LEVEL_DIR>/<name>.smclvl`, then `.txt`, then (unless user-only) `DATA_DIR/GAME_LEVEL_DIR/<name>.smclvl`, then `.txt`; on failure resets to the bare name and returns 0. User dir wins over game dir. `.txt` is accepted here but rejected by `cLevel::Load` (level.cpp:198-202) — the old format's lookup survives its loader.
- `New` (:95): refuses if `Get_Path(…,1)` finds a user file; `new cLevel`, `level->New(filename)` (which auto-names `new_N.smclvl`, N≤99, level.cpp:105-130), `Add`. `Load` (:115): `Get(filename)` by trimmed-name compare, else `new cLevel; level->Load(); Add()` — **added even when `Load` fails** (return value ignored), leaving an unloaded `cLevel` in the list.
- `Unload` (:62): deletes all but `objects.front()` (erase-while-iterating with `objects.end() - 1` bound), re-points `pActive_Level`, keeps the player's sprite manager valid, then `pActive_Level->Unload()`. Uses `pLevel_Manager->m_camera` instead of `m_camera` (:65).
- `Finish_Level` (:323): custom level → `GA_ENTER_MENU` with `menu_start_current_level`; world level → `pActive_Overworld->Goto_Next_Level(); GA_ENTER_WORLD`; both add `unload_levels=1`, random fade effects, optional `courseclear.ogg`. Callers: `objects/level_exit.cpp:352`, `enemies/furball.cpp:392`, `enemies/bosses/turtle_boss.cpp:375`, `core/game_core.cpp:241`.
- `Goto_Sub_Level` (:357): same-level entry → synchronous: `Release_Item(1,1)`, `Reset(0)` ("fixme: reset does not release active_object correctly" :369), teleport to `cLevel_Entry`, then a **nested render loop** (`while(1)` :427-458 calling `pAudio->Update`, `Draw_Game`, `pVideo->Render`, `pFramerate->Update`) that walks the camera along a `cPath_State`; other level → `GA_ENTER_LEVEL` with `load_level`/`load_level_entry`/`activate_level_entry`.

## 4. `cLevel_Player`

Base: `cAnimated_Sprite` (`m_state` is the inherited `Moving_state`, `objects/movingsprite.h:32`). Globals: `pLevel_Player` (level_player.cpp:4581; `new` in `core/main.cpp:299`, `delete` :364). `m_default_pos_{x,y}` = 200, −300 (:49-50); `power_jump_delta = 1000` ms (:47).

**`Maryo_type`** (level_player.h:28): `DEAD=0, SMALL, BIG, FIRE, ICE, CAPE, GHOST`. `m_maryo_type_temp_power` holds the real type while GHOST is active (`Set_Type` :1875), or the pending type when `temp_power=1`.

**`Moving_state`** used: `STA_STAY, STA_WALK, STA_RUN, STA_FALL, STA_FLY, STA_JUMP, STA_CLIMB`; `STA_OBJ_LINKED` is only ever assigned to the *held turtle* (`Get_Item` :3248) — `Set_Moving_State` refuses to leave it (:1912) but the player never enters it. **Ducking is not a state**: `m_ducked_counter` (ms, `Uint32`) overlays `STA_STAY` (`Start_Ducking` :1060-1086).

**Type diagram** (edges = the function that performs the change; all go through `Set_Type(Maryo_type,…)` :1792):
```
                 Get_Item(MUSHROOM_DEFAULT) :3088          Get_Item(FIREPLANT) :3127
   SMALL ────────────────────────────────▶ BIG ◀──────── FIRE ◀──────┐
     ▲ ▲                                     │  Get_Item(MUSHROOM_DEFAULT)      │ Get_Item(FIREPLANT)
     │ │   Get_Item(MUSHROOM_BLUE) :3166      │  from FIRE :3100 (fire→box)     │ from SMALL/BIG/ICE
     │ └──────────────────────────────────▶ ICE ◀────────────────────────────────┘
     │            DownGrade_Player(delayed=0, !force) :226  (any of BIG/FIRE/ICE/CAPE/GHOST → SMALL, 2.5 s invincible)
     │
   DEAD ◀── DownGrade_Player(force || SMALL) :236 ── (death sequence, then → SMALL :452 or menu/reset_save)
   GHOST ◀── Get_Item(MUSHROOM_GHOST) :3205 from any non-SMALL (SMALL needs Change_Size room) ── 10 s (m_ghost_time)
         └── Update :2223 on expiry → Set_Type(m_maryo_type_temp_power)
   CAPE  ◀── ONLY cLevel::Key_Down F2 with game_debug (level.cpp:681) or a savegame (user/savegame.cpp:316/320);
             no feather item exists (`rg TYPE_FEATHER` = 0 hits; Set_Type(SpriteType) :1768 has no cape branch)
   Cheat "K+I+D" → SMALL (level.cpp:770); "G+O+D" toggles m_god_mode (level.cpp:756)
```

**Movement diagram** (edges = function that calls `Set_Moving_State`, :1904):
```
   STAY ──Move_Player :556──▶ WALK ──Update_Walking (|velx|>10 for 1 s) :720──▶ RUN
    ▲ ▲   ◀──Hold :142 / Update_Staying velx==0 :816 / Move_Player blocked :498──┘  │
    │ │                                                                           │ Start_Jump :1342 (CAPE, RUN, |velx|>14, jump held)
    │ └── Set_On_Ground :172/:178 (from any airborne state; velx→WALK else STAY)  ▼
    │     Start_Ducking :1085 / Stop_Ducking :1117                          FLY ──Stop_Flying :979──▶ FALL (+parachute)
    │                                                                        ▲   (Handle_Collision_Massive :4400/4415/4458/4480 when |velx|<5,
    │                                                                        │    Handle_out_of_Level :4512/4518, Action_Stop_Jump :3628,
   JUMP ──Update_Jump jump_power<=0 :1463──▶ FALL                            │    Set_Type non-temp :1830)
    ▲   ──Handle_Collision_Enemy/Massive DIR_TOP/UP :4295/:4356──▶ FALL      │
    │                                                                        │
    └── Start_Jump :1360 ◀── Update_Jump_Keytime :1286 (m_force_jump, or m_up_key_time && (ground|god|CLIMB))
   CLIMB ◀── Start_Climbing :1192 (Action_Interact UP :3380, Handle_Collision_Massive climbable :4329, Start_Jump refuses if jump_power>5)
   CLIMB ──Update_Climbing lost contact :1240──▶ FALL ;  CLIMB ──Start_Jump (up key not held)──▶ JUMP
   FALL  ◀── Update_Falling :1056 (catch-all: airborne and not JUMP/CLIMB/FLY), Start_Falling :1026 (down on halfmassive), Reset :2126
```
`Set_Moving_State` exit actions: leaving FALL → `Parachute(0)`; FLY → `Change_Size` + fall image + `m_rot_z=0`; JUMP → reset jump vars; RUN → reset walk/particle timers. Entry actions: STAY → `vely=0` if on ground; FALL → `Reset_On_Ground`, walk_count/jump_power=0; FLY/JUMP → `Reset_On_Ground`; CLIMB → zero velocity; after FLY set → `Release_Item(1)`.

**Per-frame `Update`** (:2169, early-out if `editor_enabled`): `Update_Anti_Stuck` (unless ducked) → `Update_Jump_Keytime` → `Update_Jump` → `Update_Climbing` → `Update_Falling` → `Update_Walking` → `Update_Running` → `Update_Ducking` → `Update_Staying` → `Update_Flying` → throw/shoot counters → ghost timer (→ `Set_Type` on expiry, :2223) → invincible timer + flicker `m_invincible_mod` → god/star particle emitter (:2256-2320) → `Update_Item` → walk-frame counter (five ground-type `switch`es and per-type frame counts, :2326-2504, with three commented-out branches) → `Set_Image_Num(Get_Image() + m_direction)` (:2509) → `Update_Kill_Multiplier`. Collisions are run afterwards by `cLevel_Manager::Update` (`Collide_Move`, `Handle_Collisions`).

**Input reads.** Two paths. (a) Edge-triggered: `cLevel::Key_Down/Key_Up/Joy_Button_Down/Joy_Button_Up` (level.cpp:676-939) translate `pPreferences->m_key_*`/`m_joy_button_*` into `Action_Interact(INP_*)` / `Action_Stop_Interact(INP_*)` / `Action_Jump` / `Action_Shoot`. (b) Level-triggered polling inside the physics: 26 direct reads of `pKeyboard->m_keys[pPreferences->m_key_{left,right,up,down,jump,action}]` OR `pJoystick->m_{left,right,up,down}` / `pJoystick->Button(...)` (`rg -c` = 26 in level_player.cpp), e.g. `Set_On_Ground` :182, `Update_Walking` :676, `Update_Flying` :881-891, `Update_Climbing` :1208-1221, `Start_Jump` :1329, `Update_Jump` :1423-1445, `Update_Item` :1523, `Get_Vel_Modifier` :3272, `Validate_Collision` :4050/4110-4131, `Handle_Collision_Massive` :4326. Joystick *axis* input is additionally converted into synthetic keyboard presses by `input/joystick.cpp:198-276` (`pKeyboard->Key_Down(pPreferences->m_key_up)` etc.), which is why `Validate_Collision` :4109 says "joystick events are sent as keyboard keys" and only reads the keyboard there — the two branches are not symmetric (button reads use `pJoystick->Button`, axis reads use both).

**Collision dispatch.** `Validate_Collision` (:3893) is a `switch(obj->m_type)` inside an `if/else` on `m_massive_type`: MASSIVE → `TYPE_FLYON, TYPE_GEE, TYPE_ROKKO, TYPE_SPIKA, TYPE_EATO, TYPE_TURTLE, TYPE_BALL, TYPE_BONUS_BOX, TYPE_SPIN_BOX` (default BLOCKING); HALFMASSIVE → down key falls through, else blocking only from top; CLIMBABLE → INTERNAL unless flying; PASSIVE → `TYPE_BONUS_BOX/SPIN_BOX/TEXT_BOX` (semi-massive invisible), `TYPE_LEVEL_EXIT` (**calls `Action_Interact` from inside the predicate** :4114-4135), `TYPE_FIREPLANT/GOLDPIECE/MOON` (INTERNAL if active). `Handle_Collision_Enemy` (:4163): star-invincible kill except `TYPE_SPIKA/EATO/THROMP` from above, `TYPE_ROKKO`, `TYPE_TURTLE_BOSS`, `TYPE_FURBALL_BOSS`; frozen enemies shatter (particle grid); top hit → FALL; then `Send_Collision`. `Handle_Collision_Massive` (:4306): climbable start, ignore `TYPE_BALL`, forward items, DIR_UP head-bump (wall_hit sound + particles only for `ARRAY_MASSIVE`, `dynamic_cast<cMovingSprite*>` to inherit platform vely :4358), DIR_DOWN landing hack (`m_jump_power < 6` :4427), L/R stop + box `Activate_Collision` when flying (`TYPE_BONUS_BOX/SPIN_BOX` :4452/4474). Other type tags special-cased: `Update_Walking` :670 refuses `TYPE_EATO/SPIKA/ROKKO/STATIC_ENEMY` as ground unless invincible; `Update_Staying` :821 walks on a `TYPE_SPIKA`; `Update_Item`/`Release_Item`/`Get_Item` are `TYPE_TURTLE`-only (`Update_Item` :1545 "other items here..." never came); `Action_Interact` scans `TYPE_LEVEL_EXIT` four times (UP/DOWN/LEFT/RIGHT, :3326-3550, near-identical loops); `Ball_Clear` :3848 `TYPE_BALL` with `m_origin_type == TYPE_PLAYER`. Full tag census in §7.

**Ducking**: `Start_Ducking` needs STAY/WALK/RUN on MASSIVE ground, records `m_duck_direction` from `m_velx` (the 720a789 fix) so `Start_Jump` :1377-1393 only lets you hop *backwards* out of a hole; power jump after 1 s ducked (`Action_Jump` :3295: `m_force_jump`, +2 power). `Stop_Ducking` refuses if standing up would collide (:1101). **Climbing**: `Is_On_Climbable` probes a 4×4 rect at the feet centre (:1244); up/down/left/right set fixed velocities (±2/±4 × modifier); refuses to climb above the top (:1230). **Flying** (CAPE only): entered from RUN by `Start_Jump`; ±15 x, ±5 y, rotation clamped ±70° (:907); leaving sets a parachute (`m_gravity_max` 25→10, :982). **Item holding**: `Get_Item(TYPE_TURTLE, 0, base)` :3242 links a standing shell (`STA_OBJ_LINKED`, `MASS_PASSIVE`), `Load_Images` switches to `_holding` sprites; `Update_Item` :1479 repositions it each frame; `Release_Item` :1563 kicks it (direction/up/opposite-when-flying), then a 25-step search for a free spot (:1694-1731) and downgrades the shell if still blocked. Released automatically on duck, climb, fly, level-change, and when the action key is lifted (`Action_Stop_Interact(INP_ACTION)` :3637).

**Death / respawn**: `DownGrade_Player(delayed=1)` (:196) defers through `GA_DOWNGRADE_PLAYER` (game_core.cpp:96-102 re-enters with `delayed=0`). Non-forced, non-small → SMALL + 2.5 s invincibility + `pHud_Itembox->Request_Item()`. Otherwise `Set_Type(MARYO_DEAD)`, `pHud_Lives->Add_Lives(-1)`, then a **synchronous 200-line modal animation** (:254-426) with its own `SDL_PollEvent` loops, `SDL_Delay(500/300)`, `goto animation_end` (:278/289/317/329), a game-over sprite + emitter when `m_lives < 0`, then `Game_Action` = menu (`reset_save`), custom-level start menu, or `Set_Type(MARYO_SMALL,0,0); GA_ENTER_WORLD` (:452-453), all with `unload_levels=1`. Respawn = the next `load_level` → `cLevel::Init` → `pLevel_Player->Set_Pos(start); Reset()` (level.cpp:418-422). `Reset_Save` (:2100) resets worlds, unloads levels, 3 lives, HUD. Falling out of the bottom → `Handle_out_of_Level(DIR_BOTTOM)` :4544 forces `m_invincible = 0; DownGrade_Player(1,1)` unless god mode (which bounces `vely=-35`).

**Level exit/entry**: `Action_Interact` UP/DOWN/LEFT/RIGHT find an intersecting `cLevel_Exit` of matching `m_exit_type`/`m_direction` and set `Game_Action = GA_ACTIVATE_LEVEL_EXIT; Game_Action_ptr = level_exit` (:3352-3364, 3416, 3478, 3538); `game_core.cpp:104-111` calls `level_exit->Activate()`, which calls `pLevel_Manager->Finish_Level` or `Goto_Sub_Level` (objects/level_exit.cpp:352/357). Entry: `load_level_entry` teleports + hides the player (game_core.cpp:214-233), `activate_level_entry` → `Goto_Sub_Level("", entry, CAMERA_MOVE_NONE)` → `entry->Activate()` (level_manager.cpp:487) for the warp animation. `INP_EXIT` (Esc / joy exit) → `GA_ENTER_MENU` with `menu_exit_back_to=MODE_LEVEL` (:3585).

**God mode / cheats**: `m_god_mode` (level.cpp:756 "G+O+D" chord, checked via `pKeyboard->m_keys` while *any* key event arrives) → no downgrade (:198), jump anywhere (`Start_Jump_Keytime` :1275, `Update_Jump_Keytime` :1284), bottom-bounce, light-blue star particles (:2297). F2 → CAPE (debug build flag only), F3 → spawns a star emitter regardless of `game_debug` (level.cpp:684-702), F4 → fade effect, "K+I+D" → SMALL.

## 5. `cLevel_Settings`

Entered via `cEditor_Level::Function_Settings` → `GA_ENTER_LEVEL_SETTINGS` → `game_core.cpp:328 pLevel_Editor->m_settings_screen->Enter()`; left by `Leave()` (game_core.cpp:299), which **is the save step**. Owns `m_camera` (`new` level_settings.cpp:50, `delete` :59) and the CEGUI window tree loaded from `level_settings/main.layout` + three tab layouts (:73-80), destroyed in `Unload` (:295-296 `removeChildWindow` + `destroyWindow`). All widgets are fetched by **global CEGUI window name** (`wmgr.getWindow("editbox_level_filename")` etc.), 40+ lookups.

Edits: level filename (rename + optional save prompt, :243-250), music (:252-258), author, version, difficulty (spinner ↔ slider kept in sync by `Spinner_Difficulty_Changed`/`Slider_Difficulty_Changed` :587-608, name via `Get_Difficulty_Name`), land type (combobox over `LLT_LAST`), description, camera limit w/h and fixed horizontal velocity (written straight into `pLevel_Manager->m_camera` and copied back to the level :271-277), last-save time (read-only), gradient colours (six editboxes, live-tinted :462-473), and the image background layer list (listbox keyed by `pos_z` text, user-data = `cBackground*`; add/delete/select/update :351-569; max 10 layers :502). `Update_BG_Image` validates the image path with `File_Exists` and clears it if missing (:555-559). `Update`/`Draw` are near-empty (`// uhm...` :305; draws a black rect :314).

**`subscribeEvent`: 21** in `Init` (level_settings.cpp:98-196), all `CEGUI::Event::Subscriber(&cLevel_Settings::X, this)`; the returned `Event::Connection` is discarded in every case and nothing is ever disconnected — the connections die with `destroyWindow`. Owners: `Spinner_Difficulty_Changed` 1, `Slider_Difficulty_Changed` 1, `Add_Background_Image` 1, `Delete_Background_Image` 1, `Button_Apply` 1, `Set_Background_Image` 1, `Update_BG_Image` 9, `Update_BG_Colors` 6.

## 6. `level_background`

`cBackground` (level_background.h:42) is a plain class, not a sprite. `BackgroundType` (h:30) is an unscoped enum with **non-contiguous values serialized raw into XML** (`BG_NONE=0, BG_IMG_TOP=3, BG_IMG_BOTTOM=1, BG_IMG_ALL=2, BG_GR_VER=103, BG_GR_HOR=104`; `Write_Property(stream,"type",m_type)` level_background.cpp:99). Two modes: gradient (`Draw_Gradient` :334 — a single `Draw_Rect` if both colours equal, else a `Draw_Gradient` whose start colour is bent toward colour 2 as the camera rises above y = −0.1, :348-360) or image (`Draw` :257 — parallax `posx = m_pos_x − camera.x·0.2·speed_x`, y likewise ·0.3, then `while` loops to tile horizontally and, for `BG_IMG_ALL`, vertically; skipped when `pPreferences->m_level_background_images` is off). `Update` :244 only advances `m_pos_{x,y}` by `const_vel·2·speed_factor`. `m_image_1` comes from `pVideo->Get_Surface` (cached, not owned; the dtor is empty :40-43). `Set_Type(const std::string&)` :149 is the inverse of `Get_Type_Name` :378 (the only `switch` outside level_player).

`cBackground_Manager : cObject_Manager<cBackground>` (h:127) adds nothing; `Delete_All` in its dtor deletes the layers. Owned by `cLevel` (§2). `cLevel::Draw_Layer_1` iterates it in insertion order (level.cpp:609), so z-order is by `m_pos_z` only through the renderer.

## 7. Type tags and dispatch

Every `m_type`/`m_sprite_array`/`m_massive_type` test in this directory:
| Site | Form | Set | Downcast |
|---|---|---|---|
| level.cpp:429, :586 (`Init`, `Update`) | `if == TYPE_PARTICLE_EMITTER` | open (1 case) | `static_cast<cParticle_Emitter*>` (:431) guarded by the tag |
| level.cpp:1068 (`Get_Entry`) | `!= TYPE_LEVEL_ENTRY` | open | `static_cast<cLevel_Entry*>` :1073 guarded |
| level_manager.cpp:446 (`Goto_Sub_Level`) | `== TYPE_PARTICLE_EMITTER` | open | `static_cast<cParticle_Emitter*>` :448 guarded |
| level_editor.cpp:96 (`Enable`) | `m_sprite_array == ARRAY_ENEMY` | open | `static_cast<cMovingSprite*>` :98 (array tag as class proof) |
| level_editor.cpp:151/156 (`Key_Down` END) | `m_sprite_array != ARRAY_ACTIVE`, `== TYPE_LEVEL_EXIT` | open | none |
| level_editor.cpp:190-197 (`Key_Down` M) | `Is_Basic_Sprite()` / `== TYPE_MOVING_PLATFORM` | open | none |
| level_editor.cpp:301-326 (`Switch_Object_State`) | 5-way `if` on `TYPE_PASSIVE→FRONT_PASSIVE→MASSIVE→HALFMASSIVE→CLIMBABLE` + `TYPE_MOVING_PLATFORM` by `m_massive_type` | closed cycle with `else return 0` | none |
| level_player.cpp:670 (`Update_Walking`) | `TYPE_EATO\|SPIKA\|ROKKO\|STATIC_ENEMY` | open | none |
| level_player.cpp:821 (`Update_Staying`) | `== TYPE_SPIKA` | open | `static_cast<cMovingSprite*>` :823 |
| level_player.cpp:1473, :1545, :1590, :1736 (item code) | `== TYPE_TURTLE` | open | `static_cast<cTurtle*>` :1473/:1592/:3255, `static_cast<cEnemy*>` :1547 (guarded by `ARRAY_ENEMY` + `TYPE_TURTLE`) |
| level_player.cpp:1768 (`Set_Type(SpriteType)`) | 5-way `if` on `TYPE_PLAYER/MUSHROOM_DEFAULT/MUSHROOM_BLUE/MUSHROOM_GHOST/FIREPLANT` | open, silent fall-through | none |
| level_player.cpp:1887 (`Set_Type`) | `dynamic_cast<cBaseBox*>(m_ground_object)` | — | unguarded RTTI probe |
| level_player.cpp:3080-3242 (`Get_Item`) | 9-way `if` on item `SpriteType` | open, silent fall-through | none |
| level_player.cpp:3342/3404/3467/3527 (`Action_Interact`) | `== TYPE_LEVEL_EXIT` | open | `static_cast<cLevel_Exit*>` guarded |
| level_player.cpp:3848 (`Ball_Clear`) | `== TYPE_BALL` | open | `static_cast<cBall*>` guarded |
| level_player.cpp:3903 (`Validate_Collision`, massive) | `switch` 9 cases + `default: break` → BLOCKING | open by design | `static_cast<cTurtle*>` :3987, `<cBall*>` :4006, `<cBaseBox*>` :4018, all inside their `case` |
| level_player.cpp:4082 (`Validate_Collision`, passive) | `switch` 7 cases + default | open | `static_cast<cBaseBox*>` :4088, `<cLevel_Exit*>` :4104 |
| level_player.cpp:4171 (`Handle_Collision_Enemy`) | `static_cast<cEnemy*>(m_sprite_manager->Get_Pointer(collision->m_number))` | — | **unguarded** — relies on the caller having routed by `ARRAY_ENEMY` |
| level_player.cpp:4185-4200 | `TYPE_SPIKA\|EATO\|THROMP`, `ROKKO`, `TURTLE_BOSS`, `FURBALL_BOSS` | open | none |
| level_player.cpp:4336/4342/4452/4474 (`Handle_Collision_Massive`) | `TYPE_BALL`; `TYPE_FIREPLANT\|GOLDPIECE\|MOON`; `TYPE_BONUS_BOX\|SPIN_BOX` | open | `static_cast<cBaseBox*>` guarded; `dynamic_cast<cMovingSprite*>` :4358 |
| level_player.cpp:4370/4484 | `collision->m_array == ARRAY_MASSIVE/ARRAY_ACTIVE` | open | none |
The ground-type `switch (m_ground_object->m_image->m_ground_type)` at :592/:683/:2351 are on `GROUND_*` (image metadata), all with `default`. Only `Switch_Object_State` and `Get_Type_Name` are closed; every `m_type` switch is an open set with a permissive default.

## 8. CEGUI seams

`subscribeEvent` count in `level/`: **22** — 21 in `cLevel_Settings::Init` (§5) and 1 in `cLevel_Player::Editor_Activate` (level_player.cpp:4563, `Combobox::EventListSelectionAccepted` → `Editor_Direction_Select`; the combobox is `wmgr.createWindow("TaharezLook/Combobox","editor_player_direction")` and handed to the base-class editor via `Editor_Add`). `rg -n "Connection|disconnect|removeEvent" level/` = 0 hits: no connection is stored or disconnected anywhere. Other CEGUI coupling: `cLevel` *inherits* `CEGUI::XMLHandler` (level.h:34) and uses `CEGUI::XMLAttributes` as its property bag and `CEGUI::XMLSerializer` for saving; `Game_Action_Data_*` maps are `CEGUI::XMLAttributes` (level_manager.cpp:332-354 etc.); `CEGUI::PropertyHelper::{int,float,colour}ToString` for number formatting (≈35 sites in level.cpp, 4 in level_settings.cpp).

## 9. Old-pattern counts (`cd smc/src/level`)

| Pattern | Command | Count |
|---|---|---|
| raw `new` (lines) | `rg -c "\bnew\b" *.cpp` | 95 (level.cpp 48, level_player.cpp 23, level_settings.cpp 11, level_manager.cpp 10, level_editor.cpp 3) |
| raw `delete` | `rg -c "\bdelete\b" *.cpp` | 30 (player 16, level 6, settings 3, manager 3, editor 2) |
| `NULL` | `rg -c "\bNULL\b" *` | 69 (level_player.cpp 42) |
| `typedef` | `rg -n typedef *` | 0 |
| explicit iterator loops | `rg -c "::iterator" *.cpp` | 24 (level 8, player 9, manager 3, editor 3, settings 1) |
| `virtual` / `override` | `rg -c "\bvirtual\b" *.h` / `rg -c override *` | 34 / **0** |
| unscoped enums | `rg -n "^enum" *.h` | 3 (`Maryo_type`, `Maryo_imgpos`, `BackgroundType`) |
| `#define` (non-guard) | `rg -n "^#define" *` | 2 (`LEVEL_DEFAULT_MUSIC`, `LEVEL_DEFAULT_BACKGROUND`) |
| `return 0;`/`return 1;` in bool functions | `rg -c "return [01];" *` | 67 lines; plus every `bool` parameter defaults as `= 0`/`= 1` (e.g. level.h:49/111/117, level_player.h:78-215) |
| `sprintf` / `char buf[]` | `rg -n "sprintf\|char [a-z_]+\["` | 0 |
| C-style casts | `rg -n "\((int\|float\|Uint8\|...)\)"` | 3 (level.cpp:184 `(const CEGUI::utf8*)`, :193/:200 `(const std::string)`); `static_cast` 158, `dynamic_cast` 2, `reinterpret_cast` 3 |
| magic numbers | `rg -o "[0-9]+\.[0-9]+f" level_player.cpp \| wc -l` | 503 float literals in level_player.cpp alone (+≈95 ints ≥2); e.g. jump 17/4/0.59/0.12, gravity 2.8/25/10, walk 0.6/1.2/10, fly 15/5/70 |
| `printf` diagnostics | `rg -c printf *.cpp` | level.cpp 8, level_manager.cpp 2, level_background.cpp 2 (all "Warning :"/"Error :") |

## 10. Dead or suspicious code

**Dead / vestigial** (verified by grep):
- `cLevel::Process_Input` (level.cpp:667): body is `if(!editor_level_enabled){ // none }`; called every frame from level_manager.cpp:225.
- `cLevel_Manager::Init` (level_manager.cpp:57): empty; called from main.cpp:329.
- `cLevel::m_next_level_filename` and `Unload(delayed=1)`/`m_delayed_unload`: only cleared/tested (level.cpp:166, 559, 553); `rg "m_next_level_filename|Unload\( *1 *\)"` finds no writer — the deferred-load path in `cLevel::Update` is unreachable.
- `cLevel::Mouse_Down`/`Mouse_Up` (level.cpp:838-872): unreachable `return 1;` after an if/else that both return.
- `level_editor.cpp:214`: `else if( key == SDLK_m && pMouseCursor->m_hovering_object->m_obj )` can never run — the preceding branch `else if( key == SDLK_m )` (:169) always wins.
- `cLevel_Player::Copy` returns NULL (:122); `Action_Stop_Shoot` is empty (:3682); `Reset_Position` has a single caller (`Reset` :2123).
- Commented-out code: level.cpp:686-688 (F3 alternatives), :1280-1283 (hill relocations "not available"), level_manager.cpp:424-425, level_player.cpp:2385/2390/2400 (walk-count branches with empty bodies left behind), :2403-2406, :2482-2488, :2730-2740, :2743-2746, :2749-2759, :4040-4043 ("temporarily disabled"), :4499-4500, :4522-4526 & :4536-4543 (`limit_top_blocks`/`limit_bottom_blocks` — a removed level field).
- `todo`/`fixme`/`hack` (8): level.cpp:182 (utf8 workaround), :285 (`imbue`), level_manager.cpp:369 ("reset does not release active_object correctly"), level_settings.cpp:565 (listbox sort not updated), level_player.cpp:1334 ("is this needed ?"), :3234 (star music), :4314 (**moving-platform collision direction wrong → player stuck**, "easily reproducible with a low framerate"), :4424 ("hack : only pick up after some time of jumping").

**Duplicated logic**:
- The four `Action_Interact` level-exit scans (level_player.cpp:3326-3550): same 20-line loop four times differing only in `m_direction`/velocity sign.
- `Validate_Collision` `TYPE_LEVEL_EXIT` (:4106-4138) re-implements the same direction gating a second time and calls `Action_Interact` recursively.
- `Load_Images` (:2791-3059): six ~40-line blocks that differ only in the directory name (and in whether throw/run/fly slots exist) — a table of `{type, dir, has_run, has_throw, has_fly}` would replace 270 lines.
- `Update` walk-frame logic (:2326-2504): three parallel `if (SMALL) … else if (BIG) … else` ladders whose branches are identical (`0.3f` vs `0.3f`, `>= 8.0f` three times).
- Migration `posy − 600` for `engine_version < 35`: the identical 7-line block appears 12 times in `Create_Level_Object_From_XML` (level.cpp:1286, 1454, 1466, 1490, 1503, 1569, 1615, 1667, 1747, 1829, 1844, 1880) and twice more in `elementEnd`.
- Pipe/hill relocation: 24 chained `filename.compare(DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/…")` (level.cpp:1336-1363).
- `Change_Size` (:1988): horizontal and vertical halves are copy-pastes.
- Keyboard vs joystick: every poll site repeats `pKeyboard->m_keys[pPreferences->m_key_X] || pJoystick->m_X` (26 sites); `Start_Jump` :1329 and `Update_Jump` :1423 repeat the full three-term jump-held expression.
- `cLevel::Key_Down` vs `Joy_Button_Down` and `Key_Up` vs `Joy_Button_Up` (level.cpp:676-939): the same `INP_*` mapping twice; the keyboard `Key_Up` handles 6 inputs, the joystick 3.
- `overworld/world_player.cpp` (862 lines) duplicates the *shape* (`Load_Images`, `Set_Type(Maryo_type)`, `Action_Interact`/`Action_Stop_Interact`, `Update`/`Draw`) but not the physics; it has 0 direct `pKeyboard->m_keys` reads (`rg -c` = 0 vs 26 here), so the two players share only the `Maryo_type` enum and the `INP_*` protocol, not code.

**Bug smells actually seen** (quoted):
1. `Draw_Animation` (level_player.cpp:2623): `else if( ( maryo_type_old == MARYO_BIG || maryo_type_old == MARYO_FIRE || maryo_type_old == MARYO_ICE || new_mtype == MARYO_CAPE || maryo_type_old == MARYO_GHOST ) && new_mtype == MARYO_SMALL )` — the fourth term should be `maryo_type_old == MARYO_CAPE`; as written it can never be true together with `new_mtype == MARYO_SMALL`, so a CAPE→SMALL downgrade skips `Change_Size( 5.0f, 12.0f )`.
2. `Load_Images` puts the *fall* sprite in the jump-right slot for four costumes: `Add_Image( pVideo->Get_Surface( "maryo/fire/fall_right" + special_state + ".png" ) );` under `// jumping` (:2907; same at :2947 ice, :2987 flying, :3039 ghost) while `maryo/{fire,ice,flying,ghost}/jump_right.png` and `jump_right_holding.png` exist on disk (`ls data/pixmaps/maryo/*/`). Small and Big use `jump_right` correctly (:2832, :2867).
3. `cLevel_Settings::Set_Background_Image` (:403): `CEGUI::Editbox *editbox = static_cast<CEGUI::Editbox *>(wmgr.getWindow( "combo_bg_image_type" ));` — the window is a `Combobox` (created as such :148); the cast is a lie that only works because just `setText` (a `Window` method) is called.
4. `cLevel_Manager::Load` (level_manager.cpp:125-128): `level->Load( filename ); Add( level ); return level;` — a failed load is still added and returned non-NULL; `game_core.cpp:209` then treats it as success and `Init`s an empty level (`Is_Loaded()` false, so `Init` no-ops, but `Set_Active` already switched `pActive_Level`).
5. `Get_Image` (:2704-2710): `for( unsigned int i = 0; i < imgnum; i++ ) { if( imgnum % 2 ) { imgnum--; } }` — an O(n) way to write `imgnum &= ~1u`, repeated three times (:2718, :2766).
6. `Add_Kill_Multiplier` (:3866-3873): `if( static_cast<int>( m_kill_multiplier * 10 ) % 5 == 0 )` on a float accumulated by `+= 0.1f` — the "every fifth kill" test depends on float rounding; the comment says "add 1 live" but the code does `pHud_Goldpieces->Add_Gold( 100 )`.
7. `Handle_Collision_Enemy` (:4227-4234): `if( DIR_LEFT ) { m_velx = 0; } else if( DIR_RIGHT ) { m_velx = 0; }` — identical branches.
8. `cLevel::Enter` (level.cpp:502) warns with `pActive_Level->m_musicfile` rather than `m_musicfile`; `cLevel_Manager::Unload` (:65) uses `pLevel_Manager->m_camera` inside the object itself — both only correct while `this == the global`.
9. `Validate_Collision` is a predicate with side effects: it calls `Action_Interact(INP_UP/DOWN/RIGHT/LEFT)` (:4114-4135), which can set `Game_Action` during collision resolution.
10. Three synchronous modal loops that re-enter the renderer from inside game logic: `DownGrade_Player` (:270-422, own `SDL_PollEvent`, `SDL_Delay`, `goto`), `Draw_Animation` (:2632-2668, `SDL_Delay(120)`×7), `Goto_Sub_Level` (level_manager.cpp:427-458).
11. `cLevel::Key_Down` cheats (level.cpp:756/770) test `pKeyboard->m_keys[SDLK_g] && … [SDLK_d]` against the *current* key set on any key-down, and F3 spawns particles in release builds (only F2 checks `game_debug`).
12. `Set_Type(SpriteType)` (:1768) and `Get_Item` (:3061) silently ignore unknown item types (no default branch) — e.g. `TYPE_MUSHROOM_LIVE_1` passed to `Set_Type` does nothing.
13. `elementEnd` (level.cpp:1141) `Set_Difficulty( m_xml_attributes.getValueAsInteger(...) )` and `cLevel_Settings::Leave` (:264) pass an `int`/`float` into a `Uint8` parameter — values >255 wrap before the `> 100` check.

## 11. Modern-C++ hot spots (top five)

1. **Ownership → smart pointers / value members.** `cLevel`'s three managers (`new`/`delete` level.cpp:74-92), `cLevel_Manager::m_camera` and its `cLevel` list (`cObject_Manager<T>` = `vector<T*>` + manual `Delete_All`), `cLevel_Settings::m_camera`, `cEditor_Level::m_settings_screen`, every `cObjectCollisionType *col_list = …; … delete col_list;` pair (12 in level_player.cpp: :474/491, :1006/1017/1022, :1098/1105/1109, :1254/1264/1269, :1535/1559, :1691-1756 with a `NULL` sentinel, :2001/2011/2015, :2051/2061/2065) → `std::unique_ptr` / by-value `std::vector<std::unique_ptr<cLevel>>`. The factory's 30 `return new cX(...)` → `std::unique_ptr<cSprite>`. Sites: 95 `new`, 30 `delete`, 69 `NULL`.
2. **The XML factory → data-driven table + one migration pipeline.** `Create_Level_Object_From_XML` (750 lines) is an `if/else` string ladder with the `posy−600` block pasted 12× and `Is_Level_Object_Element` maintained by hand. Replace with `std::unordered_map<std::string_view, Creator>` (closes the `music` gap) and a list of `{min_version, element_filter, lambda}` migrations applied before construction; `switch`-free and testable. Sites: level.cpp:135-147, 1218-1972.
3. **Type-tag switches → virtual/visitor on the sprite hierarchy.** `Validate_Collision` (2 `switch`es, 16 cases, 5 downcasts), `Handle_Collision_Enemy/Massive` (8 tag tests, 1 unguarded `static_cast<cEnemy*>`), `Update_Walking/Staying`, `Update_Item/Release_Item/Get_Item` (`TYPE_TURTLE` only, 7 sites). A `virtual Col_Valid_Type Validate_Player_Collision(const cLevel_Player&) const` on `cSprite` (with `Is_On_Top` passed in) removes every downcast; `enum class` for `Maryo_type`, `BackgroundType` (whose raw ints are the file format — keep a serialization map), `Maryo_imgpos`.
4. **Input polling → an input-state struct sampled once per frame.** 26 `pKeyboard->m_keys[pPreferences->m_key_X] || pJoystick->m_X` sites, plus the joystick→synthetic-keyboard bridge; replace with `struct PlayerInput { bool left,right,up,down,jump,action; }` filled in `cLevel_Manager::Update` and passed to `cLevel_Player::Update(const PlayerInput&)`; `Validate_Collision` then cannot read keys and its `Action_Interact` side effect has to move out (bug 9). Also collapses the `Key_Down`/`Joy_Button_Down` twin ladders (level.cpp:676-939) into one `INP_*` table.
5. **Synchronous modal sequences → state-machine/coroutine steps.** `DownGrade_Player` (:254-462), `Draw_Animation` (:2632-2668), `Goto_Sub_Level`'s path fly-through (level_manager.cpp:427-458) each own an inner render loop with `SDL_Delay`/`goto`. Model them as a `DeathSequence`/`TransformSequence` object advanced by the normal `Update`/`Draw` (or a C++20 coroutine) so input, framerate, and rendering stay in one loop. Secondary: `Load_Images` 6×40-line ladder → a `constexpr` table (§10 duplicates); `for(...::iterator itr = …)` ×24 → range-for; the 503 float literals in level_player.cpp → named `constexpr` tuning constants.

## 12. Cross-references

**Depends on** (includes/uses): `core/` (`game_core.h` globals `Game_Action*`, `Game_Mode*`, `editor_enabled`, `editor_level_enabled`, `game_debug`; `sprite_manager`, `camera`, `framerate`, `obj_manager`, `main.h` `Draw_Game`/`Clear_Input_Events`, `filesystem`, `resource_manager`, `i18n`, `math/utilities`), `objects/` (`movingsprite`/`animated_sprite` bases, `level_exit`, `level_entry`, `box`/`bonusbox`/`spinbox`/`text_box`, `goldpiece`, `powerup`, `star`, `enemystopper`, `moving_platform`, `path`, `ball`), `enemies/` (12 headers + `bosses/turtle_boss.h` for the factory; `turtle.h` for item holding), `video/` (`video`, `renderer`, `animation`, `font`, `gl_surface`), `audio/` (`audio`, `random_sound`), `input/` (`keyboard`, `mouse`, `joystick`), `user/` (`preferences`, `savegame`), `gui/` (`hud` `pHud_*`, `menu`, `generic` `Box_Question`/`Box_Text_Input`), `overworld/` (`overworld`, `world_manager`, `world_editor`), and CEGUI (`XMLHandler`, `XMLParser`, `XMLSerializer`, `XMLAttributes`, `WindowManager`, `Spinner/Slider/Editbox/Combobox/Listbox/PushButton/TabControl/MultiLineEditbox`, `PropertyHelper`).

**Depended on by** (files including a `level/*.h`, by dir): `objects/` 14, `enemies/` 12 + `bosses/` 1, `core/` 6, `overworld/` 4, `gui/` 3, `input/` 3, `user/` 2, `audio/` 1. Concretely: `core/main.cpp` creates/destroys `pLevel_Player`, `pLevel_Manager`, `pLevel_Editor` (:299-321, :364-442) and drives `pLevel_Manager->Update()/Draw()` (:701/:733); `core/game_core.cpp` calls `Enter/Leave`, `Load/New/Set_Active/Unload/Finish_Level/Goto_Sub_Level`, `DownGrade_Player(0,…)`, `Reset_Save`; `input/keyboard.cpp:79/189`, `input/mouse.cpp:251/317`, `input/joystick.cpp:426/504` dispatch to `pActive_Level->Key_*/Mouse_*/Joy_Button_*` and `input/keyboard.cpp:213` to `pLevel_Editor->m_settings_screen->Key_Down`; `user/savegame.cpp` loads levels, re-uses `Create_Level_Object_From_XML` (:1018) and restores `m_maryo_type` (:316/320); `gui/menu.cpp:243`/`menu_data.cpp:137` draw `Draw_Layer_1` as the menu backdrop and `menu_data.cpp:3117` pre-loads levels; enemies/objects reach `pLevel_Player` (23 files) for `DownGrade`, `Add_Kill_Multiplier`, `Get_Item`, `m_invincible*`, `Action_Jump(1)` and `pLevel_Manager` (`Finish_Level`, `m_camera`).
