# Input flow — SDL event to player action (L1)

Cross-cutting synthesis over `input/`, `core/main.cpp`, `level/level_player`, `overworld`, `gui/menu`.
Provenance: commit `d438165` (the SDL 1.2 → 2 port lives largely in `input/`). Baseline: a single
SDL event pump feeding CEGUI first, then a mode handler, then per-frame polling.

## The path, end to end

```
SDL_PollEvent (core/main.cpp: Update_Game)
      │
      ▼
Handle_Input_Global (core/main.cpp)   ── switch on ev->type
      ├─ SDL_TEXTINPUT  → cKeyboard::Text_Input  → CEGUI injectChar (UTF-8 decoded)
      ├─ SDL_KEYDOWN/UP → cKeyboard::Key_Down/Up
      │        ├─ 1. CEGUI_Handle_Key_Down (inject scancode; if consumed, stop)
      │        ├─ 2. m_keys[key] = 1   (record for polling)
      │        ├─ 3. global hotkeys (Alt+F4, Alt+Return, Ctrl+C/X/V, F-keys, screenshot)
      │        └─ 4. dispatch by Game_Mode → pActive_Level / pActive_Overworld /
      │                pMenuCore / level-settings  ::Key_Down
      ├─ SDL_JOYBUTTON* → cJoystick::Handle_Button_*
      ├─ SDL_JOYHAT/AXIS → cJoystick::Handle_Hat/Motion  ── synthesize keyboard events!
      └─ default        → cMouseCursor::Handle_Event → editors'/menu's Handle_Event
```

CEGUI always gets first refusal (so a focused editbox/listbox wins), then the game's own handling.

## Two input models coexist

- **Event-driven** (overworld, menu): a keydown is one action. `cOverworld::Key_Down` reacts once
  per press to walk a step; `cMenuCore::Key_Down` moves the selection. Arrows here are **hard-wired
  to the cursor keys**, not the `m_key_*` preferences.
- **Polled** (the level player): `cLevel_Player` reads `pKeyboard->m_keys[pPreferences->m_key_left]`
  (etc.) every frame inside the physics — 26 poll sites in `level_player.cpp`. Who polls `m_keys[`:
  level_player 26, overworld 5 (camera mode), editor 4, text_box 2, level 2.

## The joystick bridge (why a pad "just works")

`cJoystick::Handle_Hat`/`Handle_Motion` do not set a separate joystick state for movement — they
**call `pKeyboard->Key_Down(pPreferences->m_key_left)`** (etc.), synthesizing keyboard presses. So
every keyboard poll site picks up the stick with no joystick-specific code. This is why the overworld
and the level player both respond to a controller. The asymmetry: buttons are read via
`pJoystick->Button(...)`, axes via the synthetic keys, and `cJoystick::Left()/Right()/Up()/Down()`
read the module-global `input_event` (true only during an axis-motion frame) while `m_left`/`m_right`
persist — two ways to ask "is left held" that answer differently. No SDL 2 hotplug handling, so a
controller must be present at `Init` (the container docs say "plug in before launch").

## The key-state adapter (SDL 2 port artifact)

SDL 1.2 indexed key state by keysym; SDL 2 keycodes are sparse. `cKey_State` (in `keyboard.h`) stores
state by scancode and translates on every `operator[]` via `SDL_GetScancodeFromKey`, so every
`m_keys[SDLK_x]` in the game compiles unchanged. `typedef SDL_Keycode SDLKey` (in `global_basic.h`)
keeps the `SDLK_*` spellings. `SDL_GetKeyboardState` (SDL's own array) is used directly in only two
places (`level_player`, `text_box`). `SDLKey_to_CEGUIKey` is a large `switch` mapping keysyms to
`CEGUI::Key`.

## The mouse is also the editor

`cMouseCursor` (a `cMovingSprite`) is both the in-game cursor and the level/world editor's selection
model: `m_selected_objects`, `m_copy_objects`, `m_hovering_object`, `m_active_object` (all raw-pointer
vectors). `Mover_Update` runs its **own inner `SDL_PollEvent` loop** while the middle button pans —
a second event loop outside `Handle_Input_Global`. See [[editors]].

## Modernization pointers

Sample input once per frame into a `struct PlayerInput { bool left,right,up,down,jump,action; }`
filled in `cLevel_Manager::Update` and passed to `cLevel_Player::Update` — this removes the 26 poll
sites, collapses the `Key_Down`/`Joy_Button_Down` twin ladders into one `INP_*` table, and forces
`Validate_Collision` (which currently reads keys and even calls `Action_Interact` as a side effect)
to stop reading input. Precompute the keycode→scancode map instead of per-access. Unify the joystick
direction queries on stored state. See [[modernization-plan]].

Related: [[game-loop-and-modes]], [[editors]], [[input]], [[level]], [[overworld]].
