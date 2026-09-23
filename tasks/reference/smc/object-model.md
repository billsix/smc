# The object model — sprites, type tags, cloning, collision (L1 + L2)

Cross-cutting synthesis over `objects/`, `enemies/`, `core/sprite_manager`, `core/collision`.
Provenance: commit `d438165`. Cite by symbol; line numbers rot. Baseline for the reader: a
2010-era single-inheritance game-object tree with runtime type tags instead of virtual dispatch.

## L1 — the mental model

Everything drawable or collidable is a `cSprite`. The hierarchy is a single spine:

```
cCollidingSprite        collision list + empty Handle_Collision_* virtuals
└─ cSprite              position, image, TWO type tags, XML I/O, editor hooks, Copy()
   └─ cMovingSprite     velocity, direction, gravity
      └─ cAnimated_Sprite   frame animation
         ├─ cBaseBox → cBonusBox / cSpinBox / cText_Box
         ├─ cPowerUp → cMushroom / cFirePlant / cMoon / cjStar
         ├─ cGoldpiece → cJGoldpiece / cFGoldpiece
         ├─ cEnemy → the 13 enemies (flat) incl. bosses/cTurtleBoss
         ├─ cMoving_Platform, cEnemyStopper, cLevel_Entry, cLevel_Exit, cBall
         ├─ cLevel_Player, cOverworld_Player   (the two players)
         └─ cAnimation → cParticle_Emitter, cAnimation_Goldpiece/Fireball
   └─ cPath, cWaypoint, cLayer_Line_Point, cRandom_Sound, cHudSprite  (off cSprite directly)
   └─ cMouseCursor  (off cMovingSprite — the editor cursor is a sprite)
```

Depth 5–6. **The design choice that shapes the whole codebase: identity is a runtime tag, not
the C++ type.** Every sprite carries two unscoped enums from `core/global_game.h`:
- `SpriteType m_type` — **58 values** (`TYPE_FURBALL`, `TYPE_BONUS_BOX`, `TYPE_LEVEL_EXIT`, …), the fine-grained "what is this exactly".
- `ArrayType m_sprite_array` — 8 values (`ARRAY_MASSIVE/PASSIVE/ENEMY/ACTIVE/HUD/ANIM/PLAYER/UNDEFINED`), the render/collision bucket.

Code asks "what are you" by comparing `m_type`, then recovers the concrete class with a
`static_cast` guarded by that comparison. Virtual functions exist (688 in `objects/`, 192 in
`enemies/`) for the *uniform* operations (`Update`, `Draw`, `Load_From_XML`, `Copy`,
`Handle_Collision_*`), but the *branching gameplay* is tag tests. There are **zero `override`
keywords** in the entire game.

## L2 — the mechanisms

### Type-tag dispatch, quantified

The heaviest concentration is in the level player. `cLevel_Player::Validate_Collision`
(`level/level_player.cpp`) is a `switch(obj->m_type)` inside an `if/else` on `m_massive_type`,
with `static_cast<cTurtle*>`/`<cBall*>`/`<cBaseBox*>`/`<cLevel_Exit*>` inside the matching cases;
`Handle_Collision_Enemy`/`Massive` add more. Nineteen distinct `TYPE_*` enemy names are
special-cased in `level_player.cpp` alone. `objects/` has ~50 `m_type ==` sites (sprite.cpp 15,
powerup.cpp 11, box.cpp 6). Most are **closed sets** over the fixed enum, so they convert cleanly
to virtual predicates; a minority are open "is it one of these gameplay types" tests that stay as
data. The author's own `// todo` at `objects/box.cpp:453` ("handle this on enemy class
`Handle_Collision_Box()`") and `enemies/spika.cpp:404` ("remove hackiness and implement a better
generic downgrade handler") ask for exactly this conversion.

One unguarded downcast to note: `cLevel_Player::Handle_Collision_Enemy` does
`static_cast<cEnemy*>(...)` relying only on the caller having routed by `ARRAY_ENEMY` — correct
today, fragile.

### Copy / clone

`virtual cX *Copy( void ) const` is declared by 14 `objects/` classes and all 13 enemies (and
`cHudSprite`, `cLayer_Line_Point_Start`). Each `new`s its own type and copies the start-state
fields; the covariant raw-pointer return is the classic `std::unique_ptr<cSprite> Clone() const
override` case. **`Copy()` is the editor's copy/paste and the savegame's spawned-object snapshot**
(`cMouseCursor::Copy` → `object->Copy()`; `cSavegame::Save_Game` → `obj->Copy()`), so it is live
and load-bearing, not dead scaffolding.

### Collision

`cCollidingSprite` (in `objects/sprite.h`) owns `m_collisions` (a `cObjectCollision*` list built by
`Create_Collision_Object`/`Add_Collision`) and the empty-by-default typed handlers
`Handle_Collision_Player/Enemy/Massive/Passive/Box`. `Handle_Collisions` walks the list and calls
the typed handler on the partner. `cObjectCollisionType` (in `core/collision`) is itself a
`cObject_Manager<cObjectCollision>`. `m_massive_type` (`MassiveType`) decides whether an object
blocks. Collision is thus a validate → collect → handle pipeline, dispatched by tag at the "handle"
step.

### Ownership of a sprite

A sprite is owned by the `cSprite_Manager` of its level/world/menu (an owning `vector<cSprite*>`
via `cObject_Manager<cSprite>`; see [[ownership-and-lifetime]]). The exception is the two players:
`pLevel_Player` is a process global with `m_disallow_managed_delete = 1` so the manager never frees
it. Sprites are created by the XML factory ([[data-formats]]) or `cMouseCursor::Copy`, handed to
the manager with `Add`, and freed by `Delete`/`Delete_All`, or deferred via `Destroy()`/
`m_auto_destroy` (the manager reuses the first auto-destroy slot on the next `Add`).

### Duplication the tree invites

The 13 enemies share an almost identical method *shape* (`Copy`, `DownGrade`, `Init`,
`Load/Save_To_XML`, `Update`, `Update_Dying`, `Set_Direction`, `Validate_Collision`,
`Handle_Collision_*`). `cTurtleBoss` shares **22 identically-named methods** with `cTurtle` — the
boss is largely a copy of the turtle. A `cWalkingEnemy` intermediate base (wall/edge turn, standard
downgrade, standard dying animation) is the single largest dedup opportunity in the game. See
[[enemies]] and [[modernization-plan]] (the modernize task).

Related: [[ownership-and-lifetime]], [[data-formats]], [[objects]], [[enemies]], [[level]].
