# Modernize step 7 — enum class + virtual dispatch

**Status:** DEFERRED / parked — its own task, skipped for now (William Emerison Six
<billsix@gmail.com>, 2026-09-24: "skip the enums stuff for now but make it a separate task"). Recon
is done (below); resume when the maintainer wants to spend the ~860-edit enum-class churn and/or the
risky dispatch rewrite. Not part of the current `modernize-cpp-3` run.
**Priority:** 6 · **Difficulty:** 8
**Part of:** `smc-modernize-cpp.md` (umbrella step 7)

## BLUF

Two separable jobs, both large: **(7a) `enum` → `enum class`** and **(7b) virtual dispatch
replacing the `m_type` type-tag switches/downcasts**. Recon (below) shows 7a is **~860
enumerator-qualification edits for modest safety value** (the enums work today; `enum class` mainly
prevents accidental cross-enum compares, not a known bug here), and **several enums are bound to the
level/save FILE FORMAT** (their int values are serialized), so those need a serialization boundary.
7b is a **high-risk gameplay rewrite**. Neither is the clean compiler-verified win the ownership
steps were — hence this doc surfaces the cost so the scope can be chosen deliberately.

## Recon: the 36 unscoped enums, classified (2026-09-24)

Verdicts (from a full usage scan):
- **SAFE** (only `==`/`switch`/assign — convert cleanly, but see churn): `GameMode`, `GameModeType`,
  `GameAction`, `LevelDrawType`, `MenuID`, `GroundType` (file uses string names), `input_identifier`,
  `RenderType`, `BlendingMode`, `ParticleClipMode`, `ColCheckType`, `TurtleBoss_state`,
  `Col_Valid_Type`, `ArrayType`, `MassiveType` (file uses string names).
- **INDEX / arithmetic** (convertible with `static_cast` at the index sites): `DefaultColor`,
  `LevelLandType`, `performance_timer_type` (`m_perf_timer[...]`), `AudioChannel` (SDL_mixer channel
  ints), `Maryo_imgpos` (`m_images[...]` + `+ m_direction`).
- **SERIALIZED** (int value crosses the level/save file boundary — needs a serialization boundary,
  convert last): `SpriteType` (the `m_type` one — also a z-order array index
  `m_z_pos_data[m_type]`), `BackgroundType`, `Maryo_type`, `Moving_state`, `Turtle_state`,
  `ObjectDirection` (int for balls + arithmetic in player image offsets), `ball_effect`,
  `Effect_Fadeout`/`Effect_Fadein` (screen_fadeout/in int + `rand()%_AMOUNT`; NO explicit values —
  ordering is load-bearing), `Level_Entry_type`, `Level_Exit_type`, `Waypoint_type`,
  `Box_Invisible_Type`, `Moving_Platform_Type`, `Moving_Platform_State`, `Camera_movement`.

**Churn for 7a (SAFE enums only):** ~860 enumerator uses to qualify (`ARRAY_` 122, `MASS_` 155,
`COL_VTYPE_` 212, `MODE_[A-Z]` 178, `GA_` 70, `MENU_` 49, `GROUND_` 30, `COLLIDE_` 28, `REND_` 15,
…). Doable via a per-enumerator codemod (word-boundary, excluding the definition block) + compiler
cleanup, but it is a big, low-value-per-edit change.

## Recommendation (for the maintainer to confirm)

1. **7a enum class:** LOW value / HIGH churn here. Options: (a) skip it; (b) do only the SAFE
   enums, accept the ~860-edit codemod; (c) also do the SERIALIZED ones behind a serialization
   boundary (int cast only at the file-I/O sites, `enum class` everywhere else) — most work, most
   correctness risk. My lean: defer 7a (or do a small codemod pass) and spend the effort on the
   higher-value showcases (step 9 dedup) instead.
2. **7b virtual dispatch:** HIGH value (this is the "excellent modern C++" centerpiece — replaces
   the `m_type` switches with virtual methods) but HIGH risk (gameplay behavior) and smoke-invisible;
   best done in small, individually play-testable commits guided by the author's own todos
   (`objects/box.cpp:453`, `enemies/spika.cpp:404`).

## Open questions

1. Step 7 scope: (a) skip enum class, (b) enum class for the SAFE enums only (~860-edit codemod),
   or (c) enum class incl. serialized ones with a boundary? And do you want the 7b dispatch rewrite
   now (risky) or later? (My lean: defer 7a, and do 7b in small reviewed commits — or move to step 9
   dedup / step 8 std::format first, which are higher value-per-risk.)
