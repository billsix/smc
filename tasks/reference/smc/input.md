# input/ — source analysis (authored directly by the Opus session after the Fable reader hit the rate limit)

Scope: `smc/src/input/{keyboard,mouse,joystick}.{cpp,h}` (3456 lines). SDL 2 / CEGUI are seams.
These files carry the SDL 1.2 → 2 port; see `../../../reference/building-2010-era-smc-today.md`.

## 1. Files and roles

| File | Lines | Role | Live? |
|---|---|---|---|
| `keyboard.h/.cpp` | ~470 | `cKeyboard` (global `pKeyboard`): SDL key events → CEGUI injection → global hotkeys → per-mode dispatch; holds the polled key-state array | live |
| `mouse.h/.cpp` | ~1750 | `cMouseCursor : cMovingSprite` (global `pMouseCursor`): the in-game cursor AND the entire level/world **editor** selection/copy/paste/mover machinery | live |
| `joystick.h/.cpp` | ~660 | `cJoystick` (global `pJoystick`): open one stick, translate axis/hat to synthetic key events, expose button/axis state | live |

`mouse.cpp` is the outlier: ~1750 lines, most of it editor logic (selection, copy buffer, snapping, the mover mode), not "the mouse".

## 2. The input flow, end to end

SDL event pump is in `core/main.cpp:Handle_Input_Global` (the `while( SDL_PollEvent )` loop):

- `SDL_TEXTINPUT` → `cKeyboard::Text_Input(ev->text.text)` — UTF-8 decoded per code point, each fed to `pGuiSystem->injectChar` (`keyboard.cpp`). This is the SDL 2 replacement for `keysym.unicode`.
- `SDL_KEYDOWN` → `cKeyboard::Key_Down(ev->key.keysym.sym)`; `SDL_KEYUP` → `Key_Up`.
- `SDL_JOYBUTTONDOWN/UP` → `cJoystick::Handle_Button_Down_Event/Up_Event`.
- `SDL_JOYHATMOTION` → `cJoystick::Handle_Hat`; `SDL_JOYAXISMOTION` → `cJoystick::Handle_Motion`.
- default → `cMouseCursor::Handle_Event`, then the editors' and menu's `Handle_Event`.

**`cKeyboard::Key_Down` order** (`keyboard.cpp`): (1) `CEGUI_Handle_Key_Down` — inject the scancode into CEGUI; if CEGUI consumes it, return. (2) set `m_keys[key] = 1`. (3) internal hotkeys: Alt+F4 (`game_exit`), Alt+Return (`Toggle_Fullscreen`), Ctrl+C/X/V (GUI clipboard). (4) dispatch by `Game_Mode` to `pActive_Level` / `pActive_Overworld` / `pMenuCore` / `pLevel_Editor->m_settings_screen`'s `Key_Down`. (5) fixed-speed (F6), screenshot, and other global function keys.

**Two input models coexist**, and this matters for the modernization:
- **Event-driven** for the overworld/menu: `cOverworld::Key_Down` reacts to the arrow keydown once (a step).
- **Polled** for the level player: `cLevel_Player` reads `pKeyboard->m_keys[pPreferences->m_key_left]` every frame. Who polls `m_keys[` (`rg -c 'm_keys\['`): `level/level_player.cpp` 26, `overworld/overworld.cpp` 5 (camera mode), `core/editor.cpp` 4, `objects/text_box.cpp` 2, `level/level.cpp` 2, plus `keyboard.h` 4 (the `Is_*_Down` helpers) and `keyboard.cpp` 2.

## 3. The key-state adapter (`cKey_State`, keyboard.h:85 — added by the port)

```cpp
struct cKey_State {
    Uint8 m_state[SDL_NUM_SCANCODES];
    Uint8 &operator[]( SDLKey key ) { return m_state[SDL_GetScancodeFromKey( key )]; }
    const Uint8 &operator[]( SDLKey key ) const { return m_state[SDL_GetScancodeFromKey( key )]; }
};
```

SDL 1.2 indexed key state by keysym; SDL 2 keycodes are sparse (bit 30 set for non-character
keys), so the port stores state by scancode and translates on every `[]`. Cost: one
`SDL_GetScancodeFromKey` (a table lookup) per access — 26 per frame in `level_player` alone,
negligible but avoidable. `SDL_GetKeyboardState` (the SDL-owned array) is used in only two places
(`level_player.cpp:400`, `text_box.cpp:202`), both to read the modifier/typing state directly —
those bypass `m_keys` and are correct. The `typedef SDL_Keycode SDLKey` lives in
`core/global_basic.h` and keeps every `SDLK_*` spelling in the game compiling.

## 4. `SDLKey_to_CEGUIKey` (keyboard.cpp)

A large `switch` mapping each `SDLK_*` to `CEGUI::Key::*` (Scan codes). Arrow keys, letters,
digits, keypad, function keys, modifiers all covered (verified arrows map at `keyboard.cpp:400-403`).
A modern rewrite could drop it entirely by injecting scancodes CEGUI already understands, but that
is a behaviour change, out of scope for the port.

## 5. `cMouseCursor` (mouse.cpp) — cursor + editor

`cMouseCursor` **is a `cMovingSprite`** (the cursor draws as a sprite). Beyond the cursor it owns
the editor's whole selection model: `m_selected_objects` (`SelectedObjectList` = `vector<cSelectedObject*>`),
`m_copy_objects` (`CopyObjectList`), `m_hovering_object`, `m_active_object`. `cSelectedObject` and
`cCopyObject` are tiny owning wrappers around a `cSprite*`. All four collections are raw-pointer
vectors with manual `new`/`delete` — the clearest `unique_ptr` candidates here.

`Mover_Update` (mouse.cpp:~1642) runs its **own inner `SDL_PollEvent` loop** while the middle
mouse button pans the camera — it drains events, warps the mouse with `SDL_WarpMouseInWindow`, and
handles a nested keydown. This is a second event loop outside `Handle_Input_Global` (a structural
smell; harmless but surprising).

## 6. `cJoystick` (joystick.cpp)

- `Init`: `SDL_NumJoysticks`, pick by saved name (`m_joy_name`), `SDL_JoystickEventState(SDL_ENABLE)`, `Stick_Open`.
- `Handle_Hat` / `Handle_Motion`: **synthesize keyboard events** — on axis/hat direction they call `pKeyboard->Key_Down(pPreferences->m_key_left)` etc., so the level player's polling of `m_keys` picks up the stick without any joystick-specific code. Elegant, and why the overworld "just works" with a pad. `m_left/m_right/m_up/m_down` also track the analog state directly (used by the camera and level player's analog jump).
- `Button(num)`: bounds-checked read of `m_buttons` (`vector<bool>`).
- `Left()/Right()/Up()/Down()`: **read the module-global `input_event`** (`core/game_core.cpp`), not stored state — they only report true during the frame an axis-motion event is being processed. A subtle coupling to the event loop; contrast with `m_left` etc. which persist.
- **Hotplug:** none. `SDL_JoystickEventState` is enabled but the code never handles `SDL_JOYDEVICEADDED/REMOVED` (SDL 2), so a controller must be present at `Init`. This matches the container caveat "plug in before launch."

## 7. Old-pattern counts (`rg` over `input/`)

new 19 · delete 12 · NULL 22 · typedef 3 · iterator-for 18 · virtual 5 · **override 0** · enum 0 ·
`#define` 5 (incl. the two wheel-button shims added by the port) · `bool x = 0/1` 7 · sprintf 0 · todo 0.

## 8. Dead / suspicious

- No `todo`s, no dead functions found; every public method is called (verified `pMouseCursor->`, `pJoystick->`, `pKeyboard->` all referenced across the tree).
- `cJoystick::Left()` reading the global `input_event` rather than `m_left` is a real inconsistency: two ways to ask "is left held", answering differently depending on the frame. Worth unifying during modernization, not a bug today.

## 9. Modern-C++ hot spots (top 5)

1. `SelectedObjectList` / `CopyObjectList` raw-pointer vectors → `std::vector<std::unique_ptr<…>>` (mouse.cpp); removes the manual `Clear_*`/`delete` loops.
2. `cJoystick::m_buttons` `vector<bool>` polled by index → fine, but `Left/Right/Up/Down` should read stored state, not the global `input_event`.
3. The `cKey_State` per-access `SDL_GetScancodeFromKey` → precompute the keycode→scancode map once, or store the game's ~15 bound keys as scancodes in preferences.
4. `NULL` → `nullptr` (22), int-as-bool → `true/false` (7), `typedef` → `using` (3).
5. Add `override` to the 5 `virtual`s (`cMouseCursor` overrides `cMovingSprite`).

## 10. Cross-references

Depends on: `objects/movingsprite` (mouse is a sprite), `core/{game_core,global_basic}`,
`user/preferences` (all bindings), `video` (cursor draw), `core/camera`. Depended on by: everything
that reads input — `level/level_player`, `overworld`, `gui/menu*`, `core/editor`, `objects/text_box`.
