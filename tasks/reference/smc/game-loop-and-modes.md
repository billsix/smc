# The game loop, modes, and the deferred-action bus (L1 + L2)

Cross-cutting synthesis over `core/main.cpp` and `core/game_core.cpp`. Provenance: commit `d438165`.
Baseline: a single-threaded fixed-function loop with a global "current mode" and a stringly-typed
command queue.

## L1 — the mental model

`main()` runs `while( !game_exit ) { Update_Game(); Draw_Game(); pVideo->Render(); pFramerate->Update(); }`.
A global `GameMode Game_Mode` says which of five modes is active (level, overworld, menu, level
settings, nothing); `Update_Game`/`Draw_Game` are `if/else if` chains that dispatch to the matching
mode object. Nothing switches mode directly. Instead any code sets a global `GameAction Game_Action`
and stuffs string key/value pairs into three `CEGUI::XMLAttributes` "bags"; once per frame
`Handle_Game_Events` drains that queue and performs the transition. It is a **deferred, stringly-typed
command bus** — powerful, and the single biggest correctness/clarity target after ownership.

## L2 — the mechanisms

### The frame

`Update_Game` (`core/main.cpp`): fps cap (`Correct_Frame_Time`) → `Handle_Game_Events` (drain the
action queue) → `while(SDL_PollEvent(&input_event)) Handle_Input_Global(&input_event)` →
`pMouseCursor->Update` → `pAudio->Resume_Music/Update` → mode dispatch (`pLevel_Manager->Update` /
`pActive_Overworld->Update` / `pMenuCore->Update` / level-settings) → `Gui_Handle_Time`. `Draw_Game`:
the same mode dispatch to `Draw`, then the mouse. Both are `if/else if` chains **with no final
`else`** (`MODE_NOTHING` falls through silently), both early-return on `game_exit`.

Per-frame draw order inside a level ([[level]], [[video]]): `Clear_Screen` → backgrounds + sprites +
animations (z-sorted render queue) → player → ghost overlay → HUD → editor → `pGuiSystem->renderGUI()`
→ `SDL_GL_SwapWindow`.

### `Handle_Input_Global` (the SDL event switch)

`SDL_QUIT` → `game_exit`; `SDL_WINDOWEVENT_SIZE_CHANGED` → `notifyDisplaySizeChanged`;
`SDL_WINDOWEVENT_FOCUS_LOST` → pause music and **block in `SDL_WaitEvent(NULL)`** until any event;
text/key → `pKeyboard`; joystick → `pJoystick`; default → `pMouseCursor->Handle_Event` then the
editors'/menu's `Handle_Event`. See [[input-flow]].

### The modes

`GameMode` = `MODE_NOTHING, MODE_LEVEL, MODE_OVERWORLD, MODE_MENU, MODE_LEVEL_SETTINGS`.
`GameModeType` = `MODE_TYPE_DEFAULT | MODE_TYPE_LEVEL_CUSTOM` (a level loaded outside a world — decides
return targets). `Enter_Game_Mode`/`Leave_Game_Mode` dispatch to the mode object's `Enter`/`Leave`;
`Enter_Game_Mode` is the **only writer of `Game_Mode`**. Both are `if/else if` chains with no default.

### The deferred-action bus (the crux)

`GameAction Game_Action` = `GA_NONE, GA_ENTER_MENU, GA_ENTER_WORLD, GA_ENTER_LEVEL,
GA_DOWNGRADE_PLAYER, GA_ACTIVATE_LEVEL_EXIT, GA_ENTER_LEVEL_SETTINGS`. A producer sets it and adds
string data to `Game_Action_Data_{Start,Middle,End}` (three `CEGUI::XMLAttributes` used as string
maps) plus an untyped `void *Game_Action_ptr`. `Handle_Game_Events` loops `while(Game_Action != GA_NONE)`:
copies all five into locals, clears the globals (so a handler may queue a follow-up), then runs the
transition, with `Handle_Generic_Game_Events` interpreting the string keys (`load_level`, `new_level`,
`load_menu`, `enter_world`, `play_music`, `screen_fadeout`, `load_savegame`, `activate_level_entry`, …).

**Enum values travel as decimal strings** (`int_to_string(EFFECT_OUT_BLACK)`), keys are matched by
name, and misspelled keys are silently ignored — a schema-free command bus. `Game_Action_ptr` is a
`void*` that in practice only ever holds a `cLevel_Exit*`. 54 `Game_Action = GA_*` producer sites
across 9 files (menu_data 25, level_player 10). The modernization target is a `std::variant` of
typed action structs, which also converts the four `if/else if` mode chains into `switch(enum class
GameMode)` with a `default`.

### Synchronous modal loops (a recurring anti-pattern)

Several sequences re-enter the render loop from inside game logic, each with its own
`SDL_PollEvent`/`SDL_Delay`: `cLevel_Player::DownGrade_Player` (death animation, with `goto`),
`Draw_Animation` (transform), `cLevel_Manager::Goto_Sub_Level` (pipe fly-through),
`cCamera::Move_to_Position_Gradually`, and the four `gui/generic.cpp` dialog boxes
(`Box_Text_Input`, `Box_Question`, `Draw_Static_Text`). These block the main loop; the
modernization models them as state-machine/coroutine steps advanced by the normal `Update`.

Related: [[input-flow]], [[ownership-and-lifetime]], [[data-formats]], [[core]], [[level]], [[gui]].
