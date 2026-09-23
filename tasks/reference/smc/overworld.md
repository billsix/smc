# overworld/ — source analysis (authored directly by the Opus session after the Fable reader hit the rate limit)

Scope: `smc/src/overworld/` (4762 lines, 7 file pairs). Data: `smc/data/world/<name>/{world,layer,description}.xml`.

## 1. Files and roles

| File | Role | Live? |
|---|---|---|
| `overworld.{h,cpp}` | `cOverworld` (a `CEGUI::XMLHandler`) — one world map; owns its sprite/animation managers, the layer graph, waypoints, HUD name sprites. `cOverworld_description`, `cOverworld_Manager` (below). `Create_World_Object_From_XML` factory. | live |
| `world_manager.{h,cpp}` | `cOverworld_Manager : cObject_Manager<cOverworld>` (global `pOverworld_Manager`) — all loaded worlds; `Get`/`Get_from_Path`/`Get_from_Name`, `Set_Active`. | live |
| `world_player.{h,cpp}` | `cOverworld_Player : cAnimated_Sprite` (global `pOverworld_Player`) — Maryo on the map; `Start_Walk`, `Action_Interact`, waypoint traversal. | live |
| `world_waypoint.{h,cpp}` | `cWaypoint : cSprite` — a level node: type, `m_access`/`m_access_default`, `m_destination`, forward/backward directions. | live |
| `world_layer.{h,cpp}` | `cLayer` + `cLayer_Line_Point`/`_Start` — the walkable graph (line segments between waypoints). | live |
| `world_sprite_manager.{h,cpp}` | `cWorld_Sprite_Manager : cSprite_Manager` — overrides only `Add`. | live |
| `world_editor.{h,cpp}` | `cEditor_World : cEditor` — the world editor. | live |

## 2. `cOverworld` / `cOverworld_Manager` — ownership

Construction (`overworld.cpp:176-188`) `new`s, in order: `cWorld_Sprite_Manager`, `cAnimation_Manager`,
`cOverworld_description`, `cLayer`, and two `cHudSprite` (world/level name). The destructor
(`:202-207`) deletes in the **same order** (not reversed) — harmless here because none depends on
another at teardown, but it is the same hand-ordering hazard as `main.cpp`. `cOverworld_Manager`
inherits `cObject_Manager<cOverworld>` so its worlds are an owning `vector<cOverworld*>`.

`Create_World_Object_From_XML` (`:1176-1251`) is the per-element factory: `sprite`, `waypoint`,
`sound` (`cRandom_Sound`), `line` (`cLayer_Line_Point_Start`), particle emitters — each a raw
`new`. Same shape as the level factory; a candidate for a shared registry.

## 3. Waypoints and the walk graph

`cWaypoint` (world_waypoint.h): `Waypoint_type` enum, `m_access`/`m_access_default` (locked until a
level is beaten), `m_destination` (level or sub-world name), `m_direction_forward`/`_backward`.
`cLayer_Line_Point_Start` holds `m_origin` and links to an end point; `Get_End_Waypoint` walks it.
`cOverworld_Player::Start_Walk(direction)` (world_player.cpp) finds the front line via
`Get_Front_Line`, checks the next waypoint's `m_access`, and animates along the segment; on arrival
`Activate_Waypoint`. Access is granted by `Set_Progress`/`Set_Access` and persisted through the
savegame (`user/savegame.cpp` writes `access` per waypoint) — this is why "can't move at the start"
is correct behaviour, not a bug (documented in the build reference doc).

**Input:** arrows are hard-wired to the cursor keys in `cOverworld::Key_Down` (not the
`m_key_*` preferences — verified: `SDLK_LEFT/RIGHT/UP/DOWN` literally). The joystick reaches the
overworld through the synthetic key events `cJoystick::Handle_Hat/Motion` post. Return/Space/KP-Enter
= `INP_ACTION` (enter level); Esc/Backspace = `INP_EXIT` (menu). There is a `// todo : move to a
Process_Input function` at `overworld.cpp:633` over the camera-mode polling block.

## 4. `cWorld_Sprite_Manager` and the player twins

`cWorld_Sprite_Manager` subclasses `cSprite_Manager` and overrides **only `Add`** (world sprites
have simpler z-handling). `cOverworld_Player` and `cLevel_Player` are **separate ~unrelated
classes** that share no base beyond `cAnimated_Sprite`; `Start_Walk` is 124 lines of movement logic
with no counterpart reuse. Not as duplicated as feared (the overworld player is much simpler than
the level player), but the two are a natural "player" abstraction if step 5 of the modernization
wants one.

## 5. The world editor

`cEditor_World` (world_editor.cpp). CEGUI usage across `overworld/` is small: **6 `subscribeEvent`**
(5 in `world_waypoint.cpp` for the waypoint settings dialog, 1 in `world_layer.cpp`) — versus 58 in
`gui/menu_data.cpp`. Fire-and-forget subscriptions (connections not stored), same pattern as the rest.

## 6. Type-tag dispatch

Light. `static_cast<cX*>` downcasts: world_waypoint 5, world_layer 2, world_sprite_manager 2,
world_manager 1 — mostly casting sprite-manager entries back to `cWaypoint`/`cLayer_Line_Point`.
One `enum` (`Waypoint_type`), unscoped. No large `switch( m_type )`.

## 7. Old-pattern counts (`rg` over `overworld/`)

new 51 · delete 10 · NULL 31 · typedef 2 · iterator-for 16 · virtual 48 · **override 0** · enum 1 ·
`#define` 7 · `bool x = 0/1` 10 · sprintf 0 · todo 3 (one real, at `:633`; the others in comments).

The new/delete asymmetry (51/10) is the factory `new`s handing ownership to the sprite managers,
which delete via `Delete_All` — normal for this codebase, verified.

## 8. Dead / suspicious

- No dead classes found; every world object type appears in `Create_World_Object_From_XML` and in world XML.
- The construction/teardown same-order in `cOverworld` dtor is a latent hazard (note for step 5).
- `Start_Walk` returning `bool` but callers ignoring it in several spots — cosmetic.

## 9. Modern-C++ hot spots (top 5)

1. `cOverworld`'s six owned pointers → `unique_ptr` members (fixes teardown order by construction).
2. `cObject_Manager<cOverworld>` base → `std::vector<std::unique_ptr<cOverworld>>` (shared with every other manager — the central refactor).
3. The `Create_World_Object_From_XML` factory → shared with the level factory via a type registry.
4. `enum Waypoint_type` → `enum class`; the direction handling → the shared `ObjectDirection` enum class.
5. `NULL`→`nullptr` (31), `override` on the 48 virtuals, `bool=0/1`→`true/false` (10).

## 10. Cross-references

Depends on: `core/{sprite_manager,camera,xml,global_game}`, `objects/{sprite,animated_sprite}`,
`video` (HUD sprites, particles), `user/savegame` (access persistence), `gui/hud`, `input`. Depended
on by: `core/main` (mode), `gui/menu` (start), `user/savegame`, `core/editor`.
