# Bug: fire/ice/flying/ghost Maryo shows the fall sprite while jumping

**Status:** DONE 2026-09-23 (branch `pre-modernization-fixes`). **Done:** the `// jumping`
jump-right slot in `cLevel_Player::Load_Images` for fire/ice/flying/ghost now loads
`maryo/<costume>/jump_right` (was `fall_right`); jump_left was already correct. Verified all four
`jump_right.png` assets exist; small/big were already right. Compiles green.
**Priority:** 5 · **Difficulty:** 2

## BLUF

`cLevel_Player::Load_Images` loads the *fall* sprite into the *jump* animation slot for four
costumes (fire, ice, flying, ghost), even though the correct `jump_right.png` assets exist on
disk. Point the jump slot at the jump image. Small and Big Maryo are already correct.

## Context

- Verified in `tasks/reference/smc/level.md` (§10 #2).
- `smc/src/level/level_player.cpp`, `cLevel_Player::Load_Images`, under the `// jumping` comment:
  ```cpp
  Add_Image( pVideo->Get_Surface( "maryo/fire/fall_right" + special_state + ".png" ) );  // ~:2907
  ```
  Same at ~:2947 (ice), ~:2987 (flying), ~:3039 (ghost). Small/Big correctly use `jump_right`
  (~:2832, ~:2867). The files `data/pixmaps/maryo/{fire,ice,flying,ghost}/jump_right.png` (and
  `jump_right_holding.png`) exist (`ls data/pixmaps/maryo/*/`).

## Fix

Replace `fall_right` with `jump_right` in those four `// jumping` `Add_Image` calls (and the
`_holding` variant if the surrounding block loads holding frames). Match the Small/Big pattern.

## Verification

Container build green; jump as fire/ice/flying/ghost Maryo and confirm the jump pose (not the fall
pose) shows on the way up. Line up against the reference frame in
`tasks/adhoc/container-build/data/main-menu-sdl2-xvfb-sandbox.png` workflow (headless smoke +
screenshot). No change to Small/Big.

## Open questions

None.
