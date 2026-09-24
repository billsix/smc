# enemies/ — source analysis (authored directly by the Opus session after the Fable reader hit the rate limit)

Scope: `smc/src/enemies/` + `enemies/bosses/` (9603 lines). Bases live in `objects/`.

## 1. Files and roles

| File | Class | Base | Live? |
|---|---|---|---|
| `enemy.{h,cpp}` | `cEnemy` | `cAnimated_Sprite` | live (base) |
| `furball.{h,cpp}` | `cFurball` | `cEnemy` | live |
| `turtle.{h,cpp}` | `cTurtle` | `cEnemy` | live |
| `krush.{h,cpp}` | `cKrush` | `cEnemy` | live |
| `eato.{h,cpp}` | `cEato` | `cEnemy` | live |
| `flyon.{h,cpp}` | `cFlyon` | `cEnemy` | live |
| `gee.{h,cpp}` | `cGee` | `cEnemy` | live |
| `rokko.{h,cpp}` | `cRokko` | `cEnemy` | live |
| `spika.{h,cpp}` | `cSpika` | `cEnemy` | live |
| `spikeball.{h,cpp}` | `cSpikeball` | `cEnemy` | live |
| `static.{h,cpp}` | `cStaticEnemy` | `cEnemy` | live |
| `thromp.{h,cpp}` | `cThromp` | `cEnemy` | live |
| `bosses/turtle_boss.{h,cpp}` | `cTurtleBoss` | `cEnemy` | live (only boss) |

Every enemy derives directly from `cEnemy` (flat, depth 1 below the base — no enemy-family
sub-bases). All 13 are live: each has a `TYPE_*` and is created by the level XML factory.

## 2. `cEnemy` base (enemy.h)

Data: `m_dead`, `m_kill_points` (score on kill), `m_kill_sound`, `m_fire_resistant`,
`m_can_be_hit_from_shell`, direction/velocity (from `cMovingSprite`), state. Virtuals every enemy
overrides: `Update`, `Update_Late`, `Update_Gravity`, `Update_Dying`, `Set_Dead`, `DownGrade`
(the hit/kill transition), `Handle_Collision*`, `Validate_Collision`, `Handle_out_of_Level`,
`Load_From_XML`/`Save_To_XML`, `Load_From_Savegame`/`Save_To_Savegame`, `Copy`, `Editor_Activate`.
Death/score flow: a hit → `DownGrade` → `Set_Dead` → `Update_Dying` animation → points to the HUD
via `pHud_Points`/`pLevel_Player`. A `// todo: have a parent pointer and use that instead of always
the player` at enemy.cpp:193 flags that kill-credit is hard-wired to the player.

## 3. Per-enemy table

| Enemy | virtuals | own state enum | notable |
|---|---|---|---|
| furball | 15 | — (uses base dir) | the boss-furball variant is a `m_type` flag, not a subclass |
| turtle | 17 | `Turtle_state` (walk/shell/shell-move) | the shell is a turtle state, not a separate object |
| krush | 15 | — | |
| eato | 11 | — | static biter; simplest |
| flyon | 15 | — | piranha; `// todo : create again` at :660 |
| gee | 16 | — | |
| rokko | 15 | — | the big bullet |
| spika | 13 | — | `// todo : remove hackiness … generic downgrade handler` (:404) |
| spikeball | 16 | — | |
| static | 17 | — | rotating spike enemy |
| thromp | 18 | — | the smasher; most virtuals |
| turtle_boss | (5 events) | boss state machine | the only `bosses/` class |

Enemies special-cased by `TYPE_*` in `level_player.cpp`: **19 sites** — the player knows individual
enemy types (e.g. how a shell interacts, what a star does), the clearest cross-cutting tag-dispatch.

## 4. Dispatch

Collision handling is virtual (`Handle_Collision_Player/Enemy/Massive`) but the *inside* of each
recovers the partner with `static_cast<cX*>` guarded by `m_type`. `DownGrade` per enemy is a small
state machine on the enemy's own state. Two unscoped `enum`s in the whole dir (turtle's
`Turtle_state` and one other). No large `switch( m_type )` inside enemies — the tag tests are in
the player and in `objects/`.

## 5. Copy-paste across enemies

The 13 enemies share an almost identical method **shape** — every one implements `Copy`,
`Create_Name`, `DownGrade`, `Init`, `Load_From_XML`/`Save_To_XML`, `Update`, `Update_Dying`,
`Set_Direction`, `Validate_Collision`, `Handle_Collision_*`. `cTurtle` and `cTurtleBoss` share **22
identically-named methods** (verified by `comm` of their method sets: Copy, DownGrade, Hit_Enemy,
Set_Turtle_Moving_State, Stand_Up, Turn_Around, Update_Dying, …) — the boss is largely a copy of
the turtle with more states. This is the biggest single duplication in the game; a shared
`cWalkingEnemy` base (direction flip on wall/edge, standard downgrade, standard dying animation)
would remove a large fraction of the ~9600 lines.

## 6. The boss

`cTurtleBoss` (`bosses/`): a multi-hit state machine (hits, shell throw, timers), 5 CEGUI
subscriptions for its editor settings. Level-specific tuning lives in the class, not data.

## 7. CEGUI seams

`subscribeEvent` in `enemies/` = 34: static 6, turtle_boss 5, gee 5, thromp 4, flyon 4, furball 3,
eato 2, rokko 2, turtle 1, spikeball 1, krush 1 — each the enemy's editor settings dialog,
fire-and-forget.

## 8. Old-pattern counts (`rg` over `enemies/` incl. bosses)

new 83 · delete 5 · NULL 19 · typedef 0 · iterator-for 0 · virtual 192 · **override 0** · enum 2 ·
`#define` 13 · `bool x = 0/1` 18 · sprintf 0 · todo 6. The new/delete gulf (83/5) is enemies
`new`ing sub-objects (images, the shell, particles) handed to managers that free them. Note: **0
iterator-for loops** — enemies don't own collections, they are leaves.

## 9. Dead / suspicious

- No dead enemies. 6 `todo`s, several structural: enemy.cpp:193 (kill-credit hard-wired to player),
  spika.cpp:404 ("remove hackiness … generic downgrade handler" — the author wanting the very
  refactor this feeds), three `// todo : create again` (eato/gee/flyon — a particle re-emit gap).
- Magic numbers: speeds, timers and score values are inline literals throughout (dozens; e.g.
  furball walk speed, thromp smash timing) — not counted individually but pervasive.

## 10. Modern-C++ hot spots (top 5)

1. A shared `cWalkingEnemy` (and folding `cTurtleBoss` onto `cTurtle`) — the 22-method turtle/boss
   overlap and the identical method shape across all 13 are the largest dedup win in the codebase.
2. `virtual cX *Copy() const` (13×) → `std::unique_ptr<cEnemy> Clone() const override`.
3. Add `override` to 192 virtuals (deep override chains — will catch real signature drift).
4. The 19 `TYPE_*` special-cases in the player → virtual behaviour on `cEnemy`
   (`On_Player_Hit`, `Gives_Star()`), per spika.cpp:404's own todo.
5. Inline magic numbers → named `constexpr` per enemy; `enum`→`enum class`; `NULL`→`nullptr` (19).

## 11. Cross-references

Depends on: `objects/{animated_sprite,movingsprite,sprite}`, `core/{collision,sprite_manager,xml,
global_game}`, `video`, `audio`, `user/savegame`, `level/level_player` (kill credit). Depended on
by: `level/level_player` (heavy `TYPE_*` coupling), the level XML factory, `user/savegame`.
