# Bug: CAPE → SMALL downgrade never shrinks Maryo's collision size

**Status:** DONE 2026-09-23 (branch `pre-modernization-fixes`). **Done:** in
`cLevel_Player::Draw_Animation`, the CAPE→SMALL branch's fourth term was `new_mtype == MARYO_CAPE`
(impossible with `new_mtype == MARYO_SMALL`); changed to `maryo_type_old == MARYO_CAPE`, so a
caped Maryo downgraded to small now runs `Change_Size(5.0f, 12.0f)`. Compiles green.
**Priority:** 5 · **Difficulty:** 2

## BLUF

`cLevel_Player::Draw_Animation` has a boolean condition whose fourth term can never be true, so a
CAPE→SMALL downgrade skips the `Change_Size` call that shrinks Maryo. Fix the term; verify the
collision box shrinks when a caped Maryo is hit down to small.

## Context

- Verified in `tasks/reference/smc/level.md` (§10, "Bug smells actually seen" #1).
- `smc/src/level/level_player.cpp`, `cLevel_Player::Draw_Animation`:
  ```cpp
  else if( ( maryo_type_old == MARYO_BIG || maryo_type_old == MARYO_FIRE ||
             maryo_type_old == MARYO_ICE || new_mtype == MARYO_CAPE ||
             maryo_type_old == MARYO_GHOST ) && new_mtype == MARYO_SMALL )
  ```
  The fourth term reads `new_mtype == MARYO_CAPE`, but it is `&&`-ed with `new_mtype ==
  MARYO_SMALL`, so it can never contribute. It should be `maryo_type_old == MARYO_CAPE`. As written,
  a CAPE→SMALL transition skips `Change_Size( 5.0f, 12.0f )`.

## Fix

Change the fourth term to `maryo_type_old == MARYO_CAPE`. (Note CAPE is reachable only via the F2
debug key or a savegame — see level.md — so the visible impact is limited, but the code is wrong.)

## Verification

Container build green; enter CAPE (F2 in a debug build), take a downgrade hit to SMALL, confirm the
collision rect shrinks (the `Change_Size` runs). No regression to BIG/FIRE/ICE/GHOST→SMALL.

## Open questions

None.
