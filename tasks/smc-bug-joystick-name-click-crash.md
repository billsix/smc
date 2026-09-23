# Bug: clicking the "Joystick" options label crashes the game (uncaught CEGUI exception)

**Status:** DONE 2026-09-23 (branch `pre-modernization-fixes`, unsigned per the maintainer's
say-so). **Done:** `cMenu_Options::Joystick_Name_Click` in `gui/menu_data.cpp` now calls
`getWindow("joystick_combo")` (was `"combo_joy"`, which threw `UnknownObjectException` and
terminated). One-line name fix + a comment; compiles green (`build.sh game`). On-screen verify of
the Options → Joystick label click is still the maintainer's to do.
**Priority:** 3 · **Difficulty:** 2 (a crash, but a one-word fix)

## BLUF

`cMenu_Options::Joystick_Name_Click` looks up a CEGUI window by the wrong name (`combo_joy`); the
layout and the rest of the code use `joystick_combo`. `getWindow` throws `UnknownObjectException`,
and the only `try/catch` wraps `Init_Game`, not the main loop, so the game calls `std::terminate`.
Fix the name.

## Context

- Verified in `tasks/reference/smc/gui.md` (§7 #9).
- `smc/src/gui/menu_data.cpp`, `cMenu_Options::Joystick_Name_Click` (~:2762):
  `getWindow( "combo_joy" )` — but `tab_joystick.layout` names it `joystick_combo` and
  `Init_GUI_Joystick` fetches `"joystick_combo"` (~:1681). The handler is live (subscribed to the
  "Joystick" label's `EventMouseClick`, ~:1679), so clicking that label throws.
- `core/main.cpp:181-190` `try/catch` wraps `Init_Game` only; an exception from the frame loop is
  uncaught → terminate.

## Fix

Change `"combo_joy"` to `"joystick_combo"` in `Joystick_Name_Click`. (Optional hardening, its own
decision: widen the frame-loop `try/catch` — but that is a broader change; this task is just the
name.)

## Verification

Container build green; on the host, open Options → Joystick and click the "Joystick" label; confirm
it cycles the joystick name instead of crashing.

## Open questions

None.
