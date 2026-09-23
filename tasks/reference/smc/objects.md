# objects/ — source analysis (authored directly by the Opus session after the Fable reader hit the rate limit)

Scope: `smc/src/objects/` (13,984 lines, 16 file pairs) — the sprite base hierarchy plus every
non-enemy game object. SDL 2 / CEGUI / Boost are seams.

## 1. Files and roles

| File | Class(es) | Base | Live? |
|---|---|---|---|
| `sprite.{h,cpp}` | `cCollidingSprite`, `cSprite` | — / `cCollidingSprite` | live (the root) |
| `movingsprite.{h,cpp}` | `cMovingSprite` | `cSprite` | live |
| `animated_sprite.{h,cpp}` | `cAnimation_Surface`, `cAnimated_Sprite` | `cMovingSprite` | live |
| `box.{h,cpp}` | `cBaseBox` | `cAnimated_Sprite` | live |
| `bonusbox.{h,cpp}` | `cBonusBox` | `cBaseBox` | live |
| `spinbox.{h,cpp}` | `cSpinBox` | `cBaseBox` | live |
| `text_box.{h,cpp}` | `cText_Box` | `cBaseBox` | live |
| `powerup.{h,cpp}` | `cPowerUp`, `cMushroom`, `cFirePlant`, `cMoon` | `cAnimated_Sprite` / `cPowerUp` | live |
| `star.{h,cpp}` | `cjStar` | `cPowerUp` | live |
| `goldpiece.{h,cpp}` | `cGoldpiece`, `cJGoldpiece`, `cFGoldpiece` | `cAnimated_Sprite` / `cGoldpiece` | live |
| `moving_platform.{h,cpp}` | `cMoving_Platform` | `cAnimated_Sprite` | live |
| `path.{h,cpp}` | `cPath`, path state helpers | `cSprite` | live |
| `enemystopper.{h,cpp}` | `cEnemyStopper` | `cAnimated_Sprite` | live |
| `level_entry.{h,cpp}` | `cLevel_Entry` | `cAnimated_Sprite` | live |
| `level_exit.{h,cpp}` | `cLevel_Exit` | `cAnimated_Sprite` | live |
| `ball.{h,cpp}` | `cBall` | `cAnimated_Sprite` | live (Maryo's fireball/iceball) |

## 2. The class hierarchy

```
cCollidingSprite                 (collision list + Handle_Collision_* virtuals)
└─ cSprite                       (position, image, type tags, XML, editor hooks, Copy)
   └─ cMovingSprite              (velocity, direction, gravity, Collide_Move)
      └─ cAnimated_Sprite        (frame animation)
         ├─ cBaseBox ─ cBonusBox / cSpinBox / cText_Box
         ├─ cPowerUp ─ cMushroom / cFirePlant / cMoon / cjStar
         ├─ cGoldpiece ─ cJGoldpiece / cFGoldpiece
         ├─ cMoving_Platform, cEnemyStopper, cLevel_Entry, cLevel_Exit, cBall
         └─ (enemies/ hang off cAnimated_Sprite too — see enemies.md)
      └─ cMouseCursor            (in input/, also a cMovingSprite)
   └─ cPath, cWaypoint (overworld) hang off cSprite directly
```

Depth 5–6. `cCollidingSprite` defines the collision interface: `Handle_Collision` dispatches to
the empty-by-default `Handle_Collision_Player/Enemy/Massive/Passive/Box` (sprite.h:87-97), which
each leaf overrides. Collision data: `m_collisions` (a `cObjectCollision*` list), built by
`Create_Collision_Object`/`Add_Collision`, consumed by `Handle_Collisions`.

Virtuals of `cSprite` a leaf typically overrides: `Init`, `Copy`, `Load_From_XML`,
`Save_To_XML`, `Load_From_Savegame`/`Save_To_Savegame`, `Update`, `Draw`, `Handle_Collision_*`,
`Editor_Activate`/`Editor_Deactivate`/`Editor_State_Update`. `virtual` count across `objects/` is
**217**, `override` **0**.

## 3. Type tags and dispatch — the central design

Every sprite carries two unscoped enums (both in `core/global_game.h`):
- `SpriteType m_type` — **58 enumerators** (`TYPE_PLAYER`, `TYPE_FURBALL`, `TYPE_MUSHROOM_LIVE1`, …), the fine-grained "what is this".
- `ArrayType m_sprite_array` — 8 values (`ARRAY_MASSIVE/PASSIVE/ENEMY/ACTIVE/HUD/ANIM/PLAYER/UNDEFINED`), the render/collision bucket.

Dispatch is by comparing these, not by virtual calls. `m_type ==` / `m_sprite_array ==` sites in
`objects/` (`rg -c`): sprite.cpp 15, powerup.cpp 11, box.cpp 6, movingsprite.cpp 5, spinbox 3,
goldpiece 3, path 2, moving_platform 2, level_entry 1. `sprite.h:396,406` (`Is_Update_Valid`/
`Is_Draw_Valid`) branch on membership sets of `m_type`/`m_sprite_array`. Many of these are
**closed sets** (the enum is fixed), so they convert cleanly to virtual predicates during
modernization; a handful are open "is it one of these gameplay types" tests that stay as data.

Object creation from level XML is a factory (`core/level.cpp:Create_Level_Object_From_XML`, see
`level.md`) that `switch`es on the element/`type` string and `new`s the class — the inverse of the
`m_type` tag. Total raw `new` in `objects/`: **135**.

## 4. Copy / clone

14 classes declare `virtual cX *Copy( void ) const` (bonusbox, text_box, star, sprite, spinbox,
ball, powerup ×3, path, movingsprite, moving_platform, level_exit, level_entry, enemystopper,
goldpiece). Each `new`s its own type and copies the start-state fields (example — `cGoldpiece::Copy`
returns a `new cGoldpiece` with pos + color). **Used by the editor copy/paste** (`cMouseCursor::Copy`
→ `copy_object->Copy(...)`). Covariant raw-pointer return; the textbook `std::unique_ptr<cSprite>
Clone() const override` case. Any class without `Copy` cannot be duplicated in the editor.

## 5. Ownership

A sprite is owned by the `cSprite_Manager` of its level/world (an owning `vector<cSprite*>` via
`cObject_Manager<cSprite>`). Sprites are created by the XML factory or `cMouseCursor::Copy` and
handed to the manager with `Add`; destroyed by the manager's `Delete`/`Delete_All`, or deferred via
a `m_auto_destroy`/`Destroy()` path. The new/delete asymmetry in `objects/` (135/27) reflects that
most `new`s transfer ownership to the manager, which frees them elsewhere — verified, not a leak,
but exactly why `unique_ptr` in the manager would make ownership legible.

## 6. Collision

`cObjectCollision` / `cObjectCollisionType` (in `core/collision.*`, used here): a validate → collect
→ handle pipeline. `cCollidingSprite::Handle_Collisions` walks `m_collisions` and calls the typed
`Handle_Collision_*` on the partner; the partner is recovered with `static_cast<cX*>` guarded by its
`m_type`/`m_sprite_array`. `m_massive_type` (`MassiveType`) decides whether an object blocks. A
`// todo : handle this on enemy class Handle_Collision_Box()` at box.cpp:453 marks a known
dispatch-by-tag wart.

## 7. Serialization

`Load_From_XML`/`Save_To_XML` per class via `CEGUI::XMLAttributes`/`XMLSerializer`; savegame state
via `Load_From_Savegame`/`Save_To_Savegame` returning `cSave_Level_Object*` (see `user.md`). Property
names are string literals in each class.

## 8. CEGUI seams

`subscribeEvent` in `objects/` = 36: moving_platform 12, path 10, level_exit 5, bonusbox 4, box 2,
level_entry 2, sprite 1, text_box 1 — all editor settings dialogs (`Editor_Activate`), fire-and-forget
connections. This is the per-object editor UI surface.

## 9. Old-pattern counts (`rg` over `objects/`)

new 135 · delete 27 · NULL 82 · typedef 6 · iterator-for 27 · virtual 217 · **override 0** · enum 7 ·
`#define` 17 · `bool x = 0/1` 59 · sprintf 0 · todo 8 (all real design notes — box.cpp:453,
sprite.cpp:929, text_box.cpp:68, movingsprite.cpp:570/1224, moving_platform.cpp:550, …).

## 10. Dead / suspicious

- No dead classes (each appears in the level factory and level XML).
- The 8 `todo`s are the author's own design debt, several about the tag-dispatch (box.cpp:453 wants
  a virtual instead of a box-side type check) — good corroboration for the modernization direction.
- `m_type_name` (a `const std::string` per sprite) duplicates what a virtual `Type_Name()` could return.

## 11. Modern-C++ hot spots (top 5)

1. `virtual cX *Copy() const` (14×) → `std::unique_ptr<cSprite> Clone() const override`.
2. `SpriteType` (58) / `ArrayType` (8) → `enum class`; then the ~50 `m_type ==` closed-set tests → virtual predicates (`Is_Massive()`, `Is_Enemy()`), guided by the author's own box.cpp:453 todo.
3. The XML factory + `m_type` tag pair → a type registry keyed on the element name (shared with overworld).
4. `NULL`→`nullptr` (82), `bool=0/1`→`true/false` (59), `override` on 217 virtuals (will surface real signature mismatches in this deep tree — high value).
5. Owning `vector<cSprite*>` in the sprite manager → `vector<unique_ptr<cSprite>>` (removes the manual delete and the two `cObject_Manager` bugs); the `Copy`/`m_auto_destroy` ownership flags then go away.

## 12. Cross-references

Depends on: `core/{sprite_manager,collision,camera,xml,global_game,framerate}`, `video`
(image/animation/render), `user/savegame`, `audio`. Depended on by: `enemies/` (bases),
`level/`, `overworld/`, `input/mouse` (editor), `gui/hud` (`cHudSprite` is a sprite).
