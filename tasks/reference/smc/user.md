# `src/user/` — preferences and savegames (verified source analysis)

Scope: `smc/src/user/{preferences,savegame}.{h,cpp}` (2475 lines total) at commit `d438165`
("update to SDL2"). Every claim below is anchored `file:Symbol` (or `file:line` where a line is
the only anchor). Counts come from `rg` run in `smc/src/user/` unless stated. Paths below are
relative to `smc/src/`.

## 1. Files and roles

| File | Lines | Role | Status |
|---|---|---|---|
| `user/preferences.h` | 221 | `cPreferences : public CEGUI::XMLHandler` — 44 public settings fields + 46 `static const m_*_default` + `Load/Save/Reset_*/Update/Apply/Apply_Video/Apply_Audio`; `extern cPreferences *pPreferences` | live: read from 14 files, 500+ `pPreferences->m_` sites (`rg -n 'pPreferences->m_' . \| cut -d: -f1 \| sort \| uniq -c`) |
| `user/preferences.cpp` | 885 | the 46 default definitions, XML load (`handle_item`, a 60-branch `name.compare()` chain) and save (`Write_Property` list), reset groups, apply | live; only file in `user/` the SDL2 port touched (26 lines: 9 `SDLK_*` renames + 17 guard edits, `git show d438165 --stat -- smc/src/user`) |
| `user/savegame.h` | 258 | value-ish data classes `cSave_Overworld_Waypoint`, `cSave_Overworld`, `cSave_Level_Object_Property`, `cSave_Level_Object`, `cSave_Level`, `cSave`; the facade `cSavegame`; the parser `cSavegame_XML_Handler`; `#define SAVEGAME_VERSION 12` / `SAVEGAME_VERSION_UNSUPPORTED 5`; `extern cSavegame *pSavegame` | live: `cSave_Level_Object` is the type 21 sprite/enemy classes serialise through (`rg -l 'Save_To_Savegame' . -g '!user/*'` = 33 files) |
| `user/savegame.cpp` | 1111 | dtors that free the pointer graph; `cSavegame::Load_Game/Save_Game/Load/Save/Get_Description/Is_Valid`; the XML handler with old-format (`.save`, capitalised elements) compatibility | live: called from `gui/menu_data.cpp:3101-3230` (slot menu) and `core/game_core.cpp:246` (`load_savegame` action) |

Nothing in `user/` is dead at file level. Dead/vestigial *pieces* are in §5.

## 2. `cPreferences`

### Lifecycle and consumers

- Created `core/main.cpp:Init_Game` (`pPreferences = new cPreferences()`, line 257) **after** `pVideo`/`pAudio` (251-252) — required, because the ctor calls `Reset_All` → `Reset_Video`/`Reset_Audio`, which write `pVideo->m_geometry_quality`, `pVideo->m_texture_quality`, `pAudio->m_sound_volume`, `pAudio->m_music_volume` (`preferences.cpp:Reset_Video`, `Reset_Audio`). Four "preferences" therefore live in *other* objects, not in `cPreferences` (see the `_default` list: `m_geometry_quality_default`, `m_texture_quality_default`, `m_sound_volume_default`, `m_music_volume_default` have no matching `m_` field in the class).
- `Load()` at `main.cpp:277` (under a fake CEGUI system, `pVideo->Init_CEGUI_Fake()` 275, because the XML parser needs `CEGUI::System`), `Apply()` at 310, `Save()` at `main.cpp:350` (`Exit_Game`) and `gui/menu_data.cpp:1846`; deleted `main.cpp:394`.
- Consumers by group (site counts outside `user/`, `rg -o 'pPreferences->m_[a-z_0-9]+' . -g '!user/*' | sort | uniq -c`):
  - **Keyboard** (`m_key_*`, `m_scroll_speed`): `level/level_player.cpp` (33), `core/editor.cpp` (25), `level/level.cpp` (22), `gui/menu_data.cpp`, `objects/text_box.cpp` (8), `overworld/overworld.cpp` (6), `input/keyboard.cpp`, `gui/generic.cpp`. Top fields: `m_key_left/right/down` 20 each, `m_key_up` 18.
  - **Joystick** (`m_joy_*`): `input/joystick.cpp` (59), plus `menu_data.cpp`; `m_joy_enabled` 18, `m_joy_axis_threshold` 12.
  - **Video** (`m_video_*`): `video/video.cpp` (24), `core/main.cpp:669-671` (`m_video_fps_limit` → `Correct_Frame_Time`), `menu_data.cpp`.
  - **Audio** (`m_audio_*`): `audio/audio.cpp` (9; `m_audio_hz` → `Mix_OpenAudio` at `audio.cpp:199`).
  - **Editor** (`m_editor_*`): `core/editor.cpp`, `menu_data.cpp`.
  - **Game**: `m_language` → `main.cpp:279` `I18N_Set_Language`; `m_menu_level` → `gui/menu.cpp:126`; `m_always_run` (6); `m_camera_*` only via `Update/Apply` and the menu spinners; `m_level_background_images` → `level/level_background.cpp:268` (1 site); `m_image_cache_enabled` → `video/video.cpp:709`; `m_game_version` → `video/video.cpp:715` (`!= smc_version` forces an image-cache rebuild).
  - The options menu edits keys generically through `void *` (`gui/menu_data.cpp:2204` `SDLKey *key = static_cast<SDLKey *>(shortcut_item.m_key)`) and reads every `m_*_default` static through the instance (`pPreferences->m_key_up_default` etc., 1 site each).

### All defaults (`preferences.cpp:36-97`)

| Group | Static | Value |
|---|---|---|
| Game | `m_always_run_default` | `0` |
| | `m_menu_level_default` | `"menu_green_1"` |
| | `m_camera_hor_speed_default` / `m_camera_ver_speed_default` | `0.3f` / `0.2f` |
| Video | `m_video_fullscreen_default` | `0` under `#ifdef _DEBUG`, else `1` |
| | `m_video_screen_w_default` / `_h_default` / `_bpp_default` | `1024` / `768` / `32` |
| | `m_video_vsync_default` | `0` ("possible bad drivers") |
| | `m_video_fps_limit_default` | `240` |
| | `m_geometry_quality_default` / `m_texture_quality_default` | `0.5f` / `0.75f` (stored in `pVideo`) |
| Audio | `m_audio_music_default` / `m_audio_sound_default` | `1` / `1` |
| | `m_audio_hz_default` | `44100` |
| | `m_sound_volume_default` / `m_music_volume_default` | `100` / `80` (stored in `pAudio`) |
| Keyboard | `m_key_up/down/left/right_default` | `SDLK_UP/DOWN/LEFT/RIGHT` |
| | `m_key_jump/shoot/item/action_default` | `SDLK_s` / `SDLK_SPACE` / `SDLK_RETURN` / `SDLK_a` |
| | `m_key_screenshot_default` | `SDLK_PRINTSCREEN` (was `SDLK_PRINT` pre-SDL2) |
| | `m_key_editor_fast_copy_{up,down,left,right}_default` | `SDLK_KP_8/2/4/6` |
| | `m_key_editor_pixel_move_{up,down,left,right}_default` | `SDLK_KP_8/2/4/6` (same keys as fast_copy — the editor distinguishes by modifier, not here) |
| | `m_scroll_speed_default` | `1.0f` |
| Joystick | `m_joy_enabled_default` / `m_joy_analog_jump_default` | `1` / `0` |
| | `m_joy_axis_hor_default` / `_ver_default` / `_threshold_default` | `0` / `1` / `10000` |
| | `m_joy_button_{jump,shoot,item,action,exit}_default` | `0` / `1` / `3` / `2` / `4` |
| Editor | `m_editor_mouse_auto_hide_default` / `m_editor_show_item_images_default` / `m_editor_item_image_size_default` | `0` / `1` / `50` |

Fields with **no** `_default` static (reset inline in `Reset_All`/`Reset_Game`/`Reset_Joystick`): `m_game_version = smc_version`, `m_language = ""`, `m_force_user_data_dir.clear()`, `m_joy_name.clear()`, `m_level_background_images = 1`, `m_image_cache_enabled = 1`, `m_config_filename = "config.xml"`.

### File format and location

- Filename: `Reset_All` sets `m_config_filename = "config.xml"`. `Load` (`preferences.cpp:109-164`) first tries that **relative to the cwd** ("Using local preferences file"), else prefixes `pResource_Manager->user_data_dir`. That dir is `Get_User_Directory()` (`core/filesystem/filesystem.cpp:249-277`): `$HOME/.smc/` on `__unix__`, `$HOME/Library/Application Support/smc/` on `__APPLE__`, `%APPDATA%/smc/` on Win32. So the effective path is **`~/.smc/config.xml`** — confirmed. `getenv("HOME")` is C-cast to `std::string` with no null check (`filesystem.cpp:271`).
- Schema: `data/schema/Config.xsd` — root `<config>` (or legacy `<Preferences>`) containing `<property name= value=/>` (V2.0) or legacy `<Item Name= Value=/>`. `handle_item` (`preferences.cpp:454-467`) maps `Name/Value` → `name/value` for "V.1.9 and lower".
- Load path: `CEGUI::System::getSingleton().getXMLParser()->parseXMLFile(*this, …, DATA_DIR "/" GAME_SCHEMA_DIR "/Config.xsd", "")` (`preferences.cpp:145/147`; `DATA_DIR`/`GAME_SCHEMA_DIR` = `core/global_game.h:173-189`). `elementStart` dispatches `property`/`Item` to `handle_item`; `elementEnd` is an empty body (`preferences.cpp:449-452`). `handle_item` takes `CEGUI::XMLAttributes` **by value** (needed: it mutates the copy at line 466).
- A missing file is not an error: `Load` returns `0` only when the user-dir file is absent, and the caller ignores the return (`main.cpp:277`). A CEGUI parse exception is caught, printed, and shown on `pHud_Debug` — then `Load` still returns `1` with whatever parsed before the error.
- After parsing, a non-empty `m_force_user_data_dir` overrides the user dir (`pResource_Manager->Set_User_Directory`, `preferences.cpp:160`); the setter normalises separators and appends `/` (`handle_item`, 491-500). `main.cpp:286` `Init_User_Directory()` then creates the subdirs.
- Save: `Update()` pulls live values from `pLevel_Manager->m_camera`, `pAudio`, `pJoystick` (`preferences.cpp:354-372`), then a `CEGUI::XMLSerializer` writes one `<config>` with 53 `Write_Property` calls (`preferences.cpp:186-249`). `Write_Property` is `core/game_core.h:106-125`: a `CEGUI::String` base plus `inline` overloads for `int`, `unsigned int`, `Uint64`, `long`, `float` — `bool`, `Uint16`, `Sint16` and `SDLKey` all resolve to the `int` overload; `Uint8` needs the explicit `static_cast<int>` you see on `video_screen_bpp` and the `joy_button_*` lines (else it would become a `CEGUI::String` of one char). `game_version` is written as the text `"2.0.0"` and read back through `string_to_version_number` (`core/property_helper.cpp:300`) into `SMC_VERSION_NUM` form (`X*10000+Y*100+Z`, `core/global_game.h:31`). Windows uses `utf8_to_ucs2` for the `ofstream` path (`preferences.cpp:171-175`).
- Load accepts legacy names too: `always_run`, `user_data_dir`, `camera_hor_speed`, `camera_ver_speed`, `video_geometry_detail`, `video_texture_detail` (`handle_item`).

### `Reset_*`, `Apply`, `Apply_Video`, `Apply_Audio`

- `Reset_All` = version + user dir + the six groups + the two "Special" flags + filename. `Reset_Game/Video/Audio/Keyboard/Joystick/Editor` are each called from the options menu's per-tab "defaults" button (`gui/menu_data.cpp:2402, 2708, 2746, 2895, 2954`); `Reset_Video` is never called from the menu (the menu resets video fields individually, e.g. `menu_data.cpp:2565`).
- `Apply()` (`preferences.cpp:374-384`): copies camera speeds into `pLevel_Manager->m_camera` and clears `m_joy_enabled` if `pVideo->m_joy_init_failed`. Called once at `main.cpp:310`.
- `Apply_Video(w,h,bpp,fullscreen,vsync,geom,tex)` (`386-422`): if resolution/bpp/vsync/texture-quality changed → store and `pVideo->Init_Video(1)` (full re-init + texture reload); else only `Init_Geometry()` on geometry change and `Toggle_Fullscreen()` on fullscreen change. Note `m_video_fullscreen` is written in the first branch but in the else-branch `Toggle_Fullscreen` "switches video_fullscreen itself" — verified by the comment, not by reading `video.cpp`. Called from `menu_data.cpp:2586`.
- `Apply_Audio(sound, music)` (`424-439`): forces both off if `pVideo->m_audio_init_failed`, else stores and `pAudio->Init()`. **No caller found**: `rg -n 'Apply_Audio' .` matches only its declaration and definition → dead public method (the menu toggles `pAudio->m_music_enabled` directly and `Update()` copies back).

### The 17 `if( val >= 0 )` guards

`rg -c 'if\( val >= 0 \)' user/preferences.cpp` = 17, one per `keyboard_key_*` item (e.g. `preferences.cpp:613-621`):

```cpp
else if( name.compare( "keyboard_key_up" ) == 0 )
{
	int val = attributes.getValueAsInteger( "value" );

	if( val >= 0 )
	{
		m_key_up = static_cast<SDLKey>(val);
	}
}
```

Pre-port each read `if( val >= 0 && val <= SDLK_LAST )` (`git show d438165 -- smc/src/user/preferences.cpp`); SDL2 has no `SDLK_LAST` (keycodes are `Sint32` with `SDLK_SCANCODE_MASK = 1<<30`, so no dense upper bound), and the port dropped only the upper half. `SDLKey` is now `typedef SDL_Keycode SDLKey` in `core/global_basic.h:52-62` — **pasted four times in a row** (four identical `#include "SDL.h"` + comment + typedef blocks), harmless only because a repeated identical typedef is legal C++. The remaining `val >= 0` is **not dead** but nearly so: `getValueAsInteger` returns `0` for garbage text (accepted → `SDLK_UNKNOWN`), so the guard only rejects an explicit negative literal in a hand-edited file; any positive int, including non-keycodes, is accepted. Every one of the 17 blocks is byte-identical apart from the field — a table-driven loader would delete ~150 lines. The 13 other clamped items (`video_screen_h/w` 200..2560, `bpp` 8..32, volumes `0..MIX_MAX_VOLUME`, `audio_hz` 0..96000, `joy_axis_*`/`joy_button_*` 0..256, threshold 0..32767) keep both bounds.

## 3. `cSavegame`

### Versioning and location

- `SAVEGAME_VERSION 12`, `SAVEGAME_VERSION_UNSUPPORTED 5` (`savegame.h:29-30`, `#define`s). `Load_Game` only **warns** for `m_version <= 5` and proceeds (`savegame.cpp:238-241`). Format-history branches still live in the loader: `< 8` player state was the type (`savegame.cpp:314`), `< 9` y-coordinate shift by 1200 (`348`), `Overworld_Data` element for `<= 10` old format (`Handle_Overworld_Data`), `level_time` "new in V.11" (`Handle_Player`), waypoint `world_name`/`level_name` "pre 0.99.6" (`Handle_Overworld_Waypoint`). `cSave::Init` seeds `m_level_engine_version = 39` ("earliest available") vs the current `level_engine_version = 40` (`core/global_game.h:214`).
- Directory: `m_savegame_dir = pResource_Manager->user_data_dir + USER_SAVEGAME_DIR` (`savegame.cpp:220`; macro `"savegames"`, `global_game.h:198`) → `~/.smc/savegames/`. Computed once in the ctor, which runs at `main.cpp:338`, after `pPreferences->Load()` may have overridden the user dir (277) — ordering is correct. Slot files: `<slot>.smcsav` (new) with fallback `<slot>.save` (old, `cSavegame::Load` 565-571); `Save` deletes the `.save` twin (`Delete_File`, 590). `Is_Valid` = either file exists.

### XML format (`Save`, `savegame.cpp:586-742`; schema `data/schema/Savegame.xsd`)

```
<savegame>
  <information> version, level_engine_version, save_time (Uint64), description </information>
  <player> lives, points (long), goldpieces, type, state, itembox_item, [level_time if any level], overworld_active, overworld_current_waypoint </player>
  <level>* level_name, [player_posx, player_posy — only if BOTH != 0]
     <spawned_objects> one element per sprite via cSprite::Save_To_XML </spawned_objects>
     <objects_data> <object>* type + the object's own name/value properties </object> </objects_data>
  </level>
  <overworld>* name, <waypoint>* destination, access </waypoint> </overworld>
</savegame>
```

Every scalar is a `<property name= value=/>` child (`Write_Property`). The old format (`Savegame_old.xsd`) uses `Savegame/Information/Player/Level/Level_Object/Overworld_Data/Overworld/Overworld_Level` and `Property Name= Value=`; the handler accepts both spellings side by side (`elementStart` 874-884, `elementEnd` 886-955).

### Parsing (`cSavegame_XML_Handler`)

The handler is a **flat attribute accumulator**: `elementStart` adds every `property` to `m_xml_attributes`; `elementEnd` dispatches on the closing element and then clears the map, **except** for `objects_data`, `object`, `spawned_objects`, any level-object element, and `waypoint`, which `return` before the clear (to keep the enclosing level's `level_name`/`player_pos*` and the overworld `name` alive). Consequences: `Handle_Level_Object` must skip `level_name`/`player_posx`/`player_posy` by name (`996`) and remove each consumed property (`1004-1010`); `Handle_Overworld_Waypoint` removes four keys (`1099-1102`). Spawned objects are rebuilt with `Create_Level_Object_From_XML(element, attrs, m_level_engine_version, pActive_Level->m_sprite_manager)` (`level/level.cpp:1218`) — i.e. attached to the *currently active* level's manager at parse time, then re-homed by `Load_Game` (`sprite->Set_Sprite_Manager(level->m_sprite_manager)`, 369). All seven `Handle_*` methods take an `attributes` parameter and ignore it, reading `m_xml_attributes` instead. The ctor parses in its body and swallows CEGUI exceptions the same way as preferences (838-850), so `Acquire_Savegame` can hand back a half-filled `cSave`.

### Save/load flow

- `Save_Game(slot, description)` (`413-561`): refuses if `pLevel_Player->m_maryo_type == MARYO_DEAD || m_lives < 0` (`m_lives` is `int`, `level/level_player.h:275`). Builds a `cSave`: for every loaded level in `pLevel_Manager->objects` a `cSave_Level` with `Trim_Filename` name, player pos only for `pActive_Level` (`m_pos_y - 5.0f`, 453), **copies** of spawned non-auto-destroy sprites (`obj->Copy()`, 468), and one `cSave_Level_Object` per sprite whose `Save_To_Savegame()` returns non-null (`477`; base `objects/sprite.h:134` returns `NULL`; 21 overriders). Then all overworlds' `TYPE_OW_WAYPOINT` sprites → `cSave_Overworld_Waypoint{Get_Destination(), m_access}`. Active overworld only if `pActive_Overworld && Game_Mode_Type != MODE_TYPE_LEVEL_CUSTOM`. Calls `Save`, sets the HUD text (guarded `if( pHud_Debug )` here, unguarded in `Save` at 603), deletes the `cSave`.
- `Load_Game(slot)` (`228-411`): returns 0 fail / 1 level save / 2 overworld save. Order: overworld waypoint access → active overworld + `pOverworld_Player->Set_Waypoint` (or `MODE_TYPE_LEVEL_CUSTOM`) → player type/state → per level `pLevel_Manager->Load`, active-level detection, spawned sprites re-added, `objects_data` matched **by position and type** via `level->m_sprite_manager->Get_From_Position(posx, posy, type, 1)` then `Load_From_Savegame` → HUD points/gold/lives/itembox. Then `delete savegame`.
- `Set_Access` linkage: `waypoint->Set_Access( save_waypoint->m_access )` (`284`) → `overworld/world_waypoint.cpp:369` sets `m_access` (and `m_access_default` only when the second arg is true — it is not here). Waypoint lookup is by destination string: `overworld->Get_Waypoint( overworld->Get_Waypoint_Num( m_destination ) )` (`274`).
- `Get_Description(slot, only_description)` (`744-803`): `"<n>. Free Save"` if invalid; otherwise a **full parse** of the file (`Load`) just to format `"<n>. <desc> - Level <name> - Date …"` or `" - <overworld>"`; `" -  Unknown"` if no level has a non-zero position. Called twice per slot from the menu (`menu_data.cpp:3211, 3230`), so a 10-slot menu parses every save 2×.
- `Is_Valid(slot) const` — the only `const` method in the file.
- The menu's own load path (`gui/menu_data.cpp:3110-3112`) calls `pSavegame->Load` only for `Get_Active_Level()` and deletes the result; the real load then happens via the `load_savegame` game action in `core/game_core.cpp:246`. So each menu load parses the file twice too.

### Allocation / ownership (`rg -n '\bnew\b|\bdelete\b' user/ | grep -v //`)

10 `new` sites, 10 `delete` sites, all in `savegame.cpp`, all manual:

| Owner | Allocates | Frees |
|---|---|---|
| `cSave::~cSave` | — | every `cSave_Level *` in `m_levels`, every `cSave_Overworld *` in `m_overworlds` (152-164) |
| `cSave_Level::~cSave_Level` | — | every `cSave_Level_Object *` and every spawned `cSprite *` (128-140) — the sprites are freed here **unless** `Load_Game` moved them into the level and `clear()`ed the list (374) |
| `cSave_Overworld::~cSave_Overworld` | — | every `cSave_Overworld_Waypoint *` (index loop, 55-58) |
| `cSave_Level_Object` | `m_properties` holds `cSave_Level_Object_Property` **by value** (the one value-type list, `savegame.h:74`) | — |
| `cSavegame::Save_Game` | `new cSave` + `new cSave_Level` / `cSave_Overworld` / `cSave_Overworld_Waypoint` | `delete savegame` (558) |
| `cSavegame::Load` | `new cSavegame_XML_Handler` | `delete loader` (581); the `cSave` is released to the caller via `Acquire_Savegame` (nulls `m_savegame`, 867-872); `~cSavegame_XML_Handler` deletes it only if not acquired |
| `cSavegame_XML_Handler::Handle_*` | `new cSave_Level`, `cSave_Level_Object`, `cSave_Overworld`, `cSave_Overworld_Waypoint` | transferred by `vector::swap` into the `cSave` |
| `Load_Game` / `Get_Description` / `menu_data.cpp:3112` | — | `delete savegame` (409, 801) |
| The 21 `Save_To_Savegame` overriders (`enemies/*.cpp`, `objects/*.cpp`) | `new cSave_Level_Object` | `cSave_Level` dtor |

Leak on the parse-error path: none found (the handler's `cSave` is still deleted by its dtor). Leak on `Handle_Level_Object` early return (`type <= 0`, 976): nothing allocated yet — clean. `cSave_Level_Object_Property` has no dtor; `cSave_Overworld_Waypoint` and `cSave_Level_Object` have empty/`clear()`-only dtors.

## 4. Old-pattern counts (`cd smc/src/user`)

| Pattern | Command | Count |
|---|---|---|
| raw `new` | `rg -n '\bnew\b' . \| grep -v //` | 10 (all `savegame.cpp`) |
| raw `delete` | `rg -n '\bdelete\b' . \| grep -v //` | 10 (all `savegame.cpp`) |
| `NULL` | `rg -c '\bNULL\b' .` | 5 (4 savegame.cpp, 1 preferences.cpp) |
| `typedef` | `rg -c '\btypedef\b' .` | 5 (all `savegame.h`: the four `*List` vectors of pointers + the by-value property list) |
| explicit iterator loops | `rg -c '::iterator' .` | 25 (all `savegame.cpp`) + 2 index loops (`for( unsigned int i`) |
| `virtual` / `override` | `rg -c '\bvirtual\b' .` / `'\boverride\b'` | 7 / **0** (`elementStart`/`elementEnd` ×2, dtors ×3) |
| unscoped `enum` | `rg -c '\benum\b' .` | 0 defined here (consumes `SpriteType`, `Maryo_type`, `Moving_state`, `SDLKey` via `static_cast`) |
| `#define` | `rg -c '#define' .` | 4 (2 include guards, `SAVEGAME_VERSION`, `SAVEGAME_VERSION_UNSUPPORTED`) |
| `bool = 0/1` | `rg -c '= [01];' .` / `'return [01];'` | 37 / 9 (e.g. every `m_*_default = 0/1`, `m_access = 0`, `return 1;`) |
| `printf` / `sprintf` / `char[]` | `rg -c printf .` / `sprintf` / `'char '` | 18 / 0 / 0 (all diagnostics are `printf` to stdout, `%d` with `unsigned int` args at 240, 264, 279, 295, 301, 391) |
| C-style casts | `rg -n '\((const std::string\|const CEGUI::utf8 ?\*)\)' .` | 4 (`(const std::string)ex.getMessage().c_str()` ×2, `(const CEGUI::utf8*)` ×2 under `_WIN32`) |
| `static_cast` | `rg -c static_cast .` | 33 (25 preferences.cpp: 17 `SDLKey` + 8 `int` for `Write_Property`; 8 savegame.cpp) |
| `int_to_string` / `ostringstream` | `rg -c int_to_string .` / `ostringstream` | 10 / 0 |
| `std::string::compare(...) == 0` instead of `==` | `rg -c '\.compare\(' .` | 57 (54 of them in `handle_item`) |
| `(void)` parameter lists | `rg -c '( void )' .` | 33 |
| `fixme` / `todo` | `rg -n -i 'todo\|fixme' .` | 4 fixme / 0 todo (§5) |

## 5. Dead or suspicious code

1. **`cPreferences::Apply_Audio` is never called** (`rg -n Apply_Audio smc/src` → declaration + definition only). Dead public API.
2. **`cPreferences::elementEnd` is an empty override** (`preferences.cpp:449-452`); required by the CEGUI interface, but a one-line `{}` in the header would do.
3. **Broken `else if` chain in `handle_item`** — `preferences.cpp:585`: `if( name.compare( "audio_music_volume" ) == 0 )` (a bare `if`, every other branch is `else if`). Functionally harmless (names are disjoint) but every item matched by the first chain is compared again against the ~45 names of the second chain, and a reader sees two chains where the author meant one. Quote: `	}\n	if( name.compare( "audio_music_volume" ) == 0 )`.
4. **Unreferenced-outside-`user/` fields** (`rg 'pPreferences->m_force_user_data_dir\|m_config_filename' . -g '!user/*'` → 0): `m_force_user_data_dir` and `m_config_filename` are public but only used internally — should be private. `m_game_version` has one reader (`video/video.cpp:715`), `m_level_background_images` one (`level_background.cpp:268`), `m_image_cache_enabled` one (`video.cpp:709`), `m_camera_hor/ver_speed` only the menu spinners + `Update/Apply` (the live value lives in `pLevel_Manager->m_camera`). None of the 44 fields is written-but-never-read; the four "in another object" defaults (§2) are the only oddity.
5. **"Active level" sentinel is `posx != 0 && posy != 0`** (`cSave::Get_Active_Level` 207, `Load_Game` 342, `Save` 650, `Get_Description` 779, all via `Is_Float_Equal`). A player standing exactly on x==0 or y==0 saves as a *non*-active level, and `Save` then omits `player_posx/posy` entirely. The `-5.0f` at `Save_Game:453` is a nudge, not a fix. `std::optional<Point>` is the honest type.
6. **Object identity by position** — `Load_Game:386` `Get_from_Position( posx, posy, save_object->m_type, 1 )` with `posx/posy` round-tripped through `string_to_int`. Any level edit that moves an object silently drops its saved state ("not available" warning, 391).
7. **Version check that doesn't refuse** — `Load_Game:238-241` prints "Versions %d and below are unsupported" and continues loading anyway.
8. **`Get_Description`'s `Load` result is only null-checked here, not in `menu_data.cpp:3110-3111`** (`savegame->Get_Active_Level()` on a possibly null pointer — reachable when `Is_Valid` is true but the parse throws, since the handler swallows exceptions and still returns a `cSave`; so in practice non-null, but only by accident of the swallow).
9. **Unused parameters**: all seven `cSavegame_XML_Handler::Handle_*(const CEGUI::XMLAttributes &attributes)` ignore `attributes` and read the member `m_xml_attributes` — misleading signatures.
10. **Per-iteration copies**: `cSave_Level_Object::exists` and `Get_Value` (`85-111`), `Save:684`, `Handle_Level_Object:1007` copy `cSave_Level_Object_Property obj = (*itr)` (two `std::string`s) on every step.
11. **`(std::string)getenv("HOME")`** in `Get_User_Directory` (`filesystem.cpp:271`, outside `user/` but the sole source of every path here) — UB on a missing `HOME`.
12. **Four pasted `typedef SDL_Keycode SDLKey` blocks** in `core/global_basic.h:51-62` (SDL2-port artefact; keep one).
13. **`fixme`s** (4, all Windows UTF-8 workarounds): `preferences.cpp:143, 170`, `savegame.cpp:592, 838` — "Check again if CEGUI 0.8 works with std::string utf8" / "more portable way f.e. with imbue()". `std::filesystem::path` makes both moot.
14. Commented-out code in `user/`: none (`rg -n '^\s*//.*;\s*$'` = 0). The `printf( "Warning : Savegame object type %d …", save_object->m_type …)` passes an enum to `%d` — fine, but the `unsigned int save_slot` to `%d` at 240/264/279/295/301 is a mismatch.
15. `Update()` (`preferences.cpp:363`) treats joystick index `> 0` as "not default" — index 0 with a real name is saved as empty; `Reset_Joystick` clears `m_joy_name` unconditionally, so a re-selection is lost on "defaults".

## 6. Modern-C++ hot spots (top five, this subsystem)

| # | Pattern | Replacement | Sites |
|---|---|---|---|
| 1 | Pointer object graph: `vector<cSave_Level *>`, `vector<cSave_Overworld *>`, `vector<cSave_Overworld_Waypoint *>`, `vector<cSave_Level_Object *>` + 4 hand-written freeing dtors + `swap`-based ownership transfer + `cSave *` returned from `Load`/`Acquire_Savegame` "should be deleted if not used anymore" (`savegame.h:189-192, 214-217`) | value types (`struct` with `std::string`/`std::vector<T>`), `std::optional<cSave>` from `Load`, `std::unique_ptr<cSprite>` for the one truly polymorphic list (`m_spawned_objects`), and `std::optional<std::pair<float,float>>` for the player position (kills the `!= 0 && != 0` sentinel, §5.5) | 10 `new` / 10 `delete` / 5 `typedef` / 4 dtors / 5 sentinel checks |
| 2 | 60-branch `name.compare("…") == 0` chain in `handle_item` + 53 hand-listed `Write_Property` lines in `Save`, 17 identical `if( val >= 0 )` blocks | one `static constexpr` table of `{name, member-pointer/variant, min, max, legacy-alias}` driving both load and save (or a `std::unordered_map<std::string_view, Setter>`); `std::clamp` for the 13 ranged items | ~430 lines of `preferences.cpp` (454-877 + 186-249) |
| 3 | 25 explicit `::iterator` loops + 2 index loops, with by-value element copies | range-`for` with `const auto &`; `std::find_if`/`std::any_of` for `exists`/`Get_Value` | 27 loops in `savegame.cpp` |
| 4 | String-concatenated paths + `File_Exists` + `#ifdef _WIN32 utf8_to_ucs2` ofstream + `insert(0, dir)`; `getenv("HOME")` C-cast | `std::filesystem::path` (`user_dir / "savegames" / (std::to_string(slot) + ".smcsav")`, `exists()`, `remove()`), `std::ofstream(path)` is UTF-8-correct on all platforms since C++17 | 8 path builds (`savegame.cpp:565, 570, 588, 590, 807`; `preferences.cpp:127, 145, 172`), 2 `#ifdef _WIN32` pairs |
| 5 | `printf` diagnostics with `%d`/`unsigned` mismatches, `int_to_string` + `+` chains, `Time_to_String` | `std::format` / `std::println` (C++20/23), `std::format("{:%Y-%m-%d %H:%M:%S}", …)` via `<chrono>` for `save_time` | 18 `printf`, 10 `int_to_string`, 1 `Time_to_String` |

Also worth doing in the same pass: `override` on the 4 `elementStart/elementEnd` (0 today); `bool`s `true/false` (37 `= 0/1`); `nullptr` (5); `enum class` for the `1/2` return of `Load_Game` and `SAVEGAME_VERSION` as `constexpr int`; `Is_Valid` and `Get_Description` `const`/`[[nodiscard]]`; drop the `( void )` lists (33).

## 7. Cross-references

**Depends on** (`#include`s + globals actually used): `core/global_basic.h` (`SDLKey` typedef, `vector`), `core/global_game.h` (`smc_version`, `level_engine_version`, `DATA_DIR`, `GAME_SCHEMA_DIR`, `USER_SAVEGAME_DIR`), `core/game_core.h` (`Write_Property`, `Game_Mode_Type`, `Is_Float_Equal`), `core/property_helper.h` (`string_to_version_number`, `string_to_int64`, `string_to_long`, `int_to_string`, `utf8_to_ucs2`, `Time_to_String`), `core/filesystem/{filesystem,resource_manager}.h` (`File_Exists`, `Delete_File`, `Trim_Filename`, `Convert_Path_Separators`, `pResource_Manager->user_data_dir`, `Set_User_Directory`), `core/i18n.h` (`_()`), `video/video.h` (`pVideo->m_geometry_quality/m_texture_quality/m_joy_init_failed/m_audio_init_failed`, `Init_Video/Init_Geometry/Toggle_Fullscreen`), `audio/audio.h` (`pAudio` volumes/enabled flags, `Init`), `input/joystick.h` (`pJoystick->m_current_joystick`, `Get_Name`), `gui/hud.h` (`pHud_Debug/Time/Points/Goldpieces/Lives/Itembox/Manager`), `level/level_manager.h` + `level/level.h` (`pLevel_Manager->Load/Get_Path/Set_Active/m_camera/objects`, `cLevel::Is_Level_Object_Element`, `Create_Level_Object_From_XML`), `level/level_player.h` (`pLevel_Player` type/state/pos/lives/points/gold, `Set_Type/Set_Pos`), `overworld/{world_manager,overworld}.h` (`pOverworld_Manager->Get_from_Name/Set_Active/objects`, `cOverworld::Get_Waypoint/Get_Waypoint_Num`, `cWaypoint::Set_Access/Get_Destination/m_access`, `pOverworld_Player->Set_Waypoint/m_current_waypoint`), `objects/sprite.h` (`cSprite::Copy/Save_To_XML/Save_To_Savegame/Load_From_Savegame/Set_Sprite_Manager/Set_Spawned`), CEGUI `XMLHandler/XMLParser/XMLAttributes/XMLSerializer/PropertyHelper/Exception`, SDL2 `SDL_Keycode`, SDL_mixer `MIX_MAX_VOLUME`, schema files `data/schema/{Config,Savegame,Savegame_old}.xsd`.

**Depended on by**: `core/main.cpp` (creates/loads/applies/saves/deletes both singletons), `gui/menu_data.cpp` (81 preference sites: the whole options menu; the savegame slot menu), `input/joystick.cpp` (59), `level/level_player.cpp` (33), `core/editor.cpp` (25), `video/video.cpp` (24), `level/level.cpp` (22), `audio/audio.cpp` (9), `objects/text_box.cpp` (8), `overworld/overworld.cpp` (6), `gui/menu.cpp` (5), `level/level_background.cpp`, `input/keyboard.cpp`, `gui/generic.cpp` (1 each); `core/game_core.cpp:246` (`load_savegame` action → `Load_Game`); the 21 `Save_To_Savegame`/`Load_From_Savegame` overriders in `enemies/{enemy,flyon,furball,gee,krush,rokko,spikeball,static,thromp,turtle}` and `objects/{ball,box,goldpiece,moving_platform,path,powerup}` (each allocates a `cSave_Level_Object` and reads it back via `exists`/`Get_Value`); `core/global_game.h` forward-declares `cSave_Level_Object`.
