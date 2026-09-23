# Data formats — level, world, savegame, preferences, .settings, and the CEGUI XML seam (L2)

Cross-cutting synthesis over `level/level.cpp`, `overworld/`, `user/`, `video/img_settings`,
`core/property_helper`. Provenance: commit `d438165`. Baseline: XML files parsed and written not by
an XML library the game owns, but by CEGUI's XML API.

## The seam: SMC owns no XML API

Every loader is a `CEGUI::XMLHandler` subclass (`cLevel`, `cOverworld`, `cOverworld_description`,
`cPreferences`, `cSavegame_XML_Handler`, `cCampaign_XML_Handler`). Each collects
`<property name= value=>` pairs into a `CEGUI::XMLAttributes` bag on `elementStart` and reads them
back with `getValueAsString/Integer/Float/Bool` on `elementEnd`. Writers use `CEGUI::XMLSerializer`
via `Write_Property` (`core/game_core.h`; overloads for `int/unsigned/Uint64/long/float` — `bool`,
`Uint8`, `Sint16`, `SDLKey` all fold into the `int` overload, so `Uint8` fields need an explicit
`static_cast<int>` or they serialize as a one-char string). Newlines round-trip as literal `<br/>`.
**Replacing CEGUI's XML means touching ~37 handler classes, not one file** — a reason the CEGUI 0.7
pin is load-bearing beyond the GUI. Schemas live in `data/schema/*.xsd`.

## Level files (`.smclvl`, root `<level>`)

`cLevel::Load` → `getXMLParser()->parseXMLFile(*this, file, "Level.xsd", "")` (`cLevel` *is* the
handler). Elements: `information` (game/engine version, save_time), `settings` (author, version,
music, description, difficulty, land_type, camera limits, fixed hor vel), `background` 0..n, `player`
(posx/posy/direction), then object elements gated by `cLevel::Is_Level_Object_Element` (a 20-string
`||` chain kept in sync by hand with the factory).

**The factory** `Create_Level_Object_From_XML` (~750 lines) maps element (+ a `type` attribute) to a
`new cX`: `sprite`→`cSprite`, `box`→`cBonusBox`/`cSpinBox`/`cText_Box` by type, `item`/`powerup`→the
five pickups, `enemy`→the 12 enemies + `cTurtleBoss` by name, `moving_platform`/`falling_platform`,
`enemystopper`, `levelexit`, `level_entry`, `sound`→`cRandom_Sound`, `particle_emitter`/`global_effect`→
`cParticle_Emitter`, `path`, `ball`. Unknown types warn and return NULL; **`music` is in the element
whitelist but has no factory branch → silently dropped**. Callers: `cLevel::elementEnd`,
`cEditor_Level::Get_Object`, and `cSavegame`.

**Version migrations.** `level_engine_version = 40`. `elementEnd("information")` multiplies float
versions `< 3` by 10 (pre-1.7 files). Then a ladder of `if( engine_version < N )` blocks rewrites
positions/image paths/enemy names (gumba→furball, rex→krush), the biggest being `< 35` subtracting
600 from every `posy` — the **identical 7-line block is pasted 12 times** in the factory. The
modernization target: an `unordered_map<string_view, Creator>` (closing the `music` gap) plus a list
of `{min_version, filter, migration}` applied before construction.

## World files (`data/world/<name>/`)

Three files: `world.xml` (waypoints, sprites, lines, sounds via `Create_World_Object_From_XML`),
`layer.xml` (the walk graph — `cLayer_Line_Point_Start` segments), `description.xml`. Waypoints carry
`type`, `destination` (level/sub-world), `direction_forward`/`_backward`, and `access`/`access_default`
(a level node is locked until beaten; persisted through the savegame). See [[overworld]].

## Savegames (`.smcsav`, fallback `.save`, root `<savegame>`)

`SAVEGAME_VERSION 12`. Structure: `information` (version, engine version, save_time Uint64,
description); `player` (lives, points, gold, type, state, itembox, level_time, overworld position);
`level`* (name, optional player pos, `spawned_objects` via each sprite's `Save_To_XML`, `objects_data`
via each sprite's `Save_To_Savegame`→`cSave_Level_Object`); `overworld`* (name, `waypoint`* with
access). Old format (`Savegame_old.xsd`) uses capitalised elements and `Property Name=/Value=`; the
handler accepts both spellings. **Object state is matched back by position + type**
(`Get_from_Position(posx, posy, type)`), so editing a level moves an object out from under its saved
state (warns "not available"). The "active level" sentinel is `posx != 0 && posy != 0` — a player on
x==0 or y==0 saves as non-active (`std::optional<Point>` is the honest type). See [[user]].

## Preferences (`~/.smc/config.xml`, root `<config>`)

`cPreferences` = 44 settings + 46 `m_*_default` statics. `Load` reads a 60-branch `name.compare()`
chain (accepts legacy names too); `Save` writes 53 `Write_Property` lines. Location resolves through
`Get_User_Directory` (`$HOME/.smc/`, `%APPDATA%/smc/`, `~/Library/Application Support/smc/`). The 17
`keyboard_key_*` items each guard with `if( val >= 0 )` (the SDL 2 port dropped the old `&& val <=
SDLK_LAST` upper bound — SDL 2 keycodes are sparse). The whole load/save is table-driven-able (~430
lines → a `{name, member, min, max, alias}` table + `std::clamp`). See [[user]].

## `.settings` image sidecars (`video/img_settings`)

A whitespace-tokenised, one-command-per-line format (`cFile_parser`, not CEGUI XML): `base <file>
[use_settings]`, `int_x/int_y` (draw offset), `col_rect x y w h`, `width`/`height` (logical size,
also gates caching), `rotation`, `mipmap`, `editor_tags`, `name`, `type` (a `SpriteType`),
`ground_type`, `author`, `obsolete`. `base` inheritance is recursive; a `// todo` notes the override
order is wrong (base overwrites derived). Each parse allocates a `cImage_Settings_Data` the caller
frees. See [[video]].

## Number/string formatting (`core/property_helper`)

`int_to_string` (195 sites) / `string_to_int` (62) / `string_to_float` (78) via `ostringstream`/
`istringstream`; `float_to_string` is a 150-line vendored `modp_dtoa` into a `char[64]`. Six
`switch`-based `*_Name` and seven `strcmp`-chain `*_Id` functions convert enums ↔ names (~330 lines,
inverse tables). Modernization: `std::to_chars`/`from_chars`/`std::format` and one `constexpr
std::array<pair<Enum, string_view>>` per enum. See [[core]].

Related: [[game-loop-and-modes]], [[object-model]], [[level]], [[overworld]], [[user]], [[video]], [[core]].
