# `gui/` — menu system, HUD, dialog boxes, clipboard, custom spinner

Verified against `smc/src/gui/` at `v1.9-215-g768adeb` (HEAD `768adeb`; the last commit touching
`gui/` is `d438165 "update to SDL2"`, which replaced the X11 cut-buffer clipboard in `generic.cpp`
with `SDL_GetClipboardText`/`SDL_SetClipboardText` and two SDL2 renames in `menu_data.cpp`).
CEGUI pin: `configure.ac:75` `PKG_CHECK_MODULES([CEGUI_OPENGL], [CEGUI-OPENGL >= 0.7.2])`. Every
file was read in full; every claim is anchored `file:Symbol` or `file:line`.

## 1. Files and roles

| File | Lines | Role | Status |
|---|---|---|---|
| `menu.h/.cpp` | 184 / 743 | `cMenu_Item` (an animated `cHudSprite` pair), `cMenuHandler` (item list + the menu's own `cLevel`/`cCamera`/player stand-in), `cMenuCore` (the `MODE_MENU` game-mode object, `pMenuCore`) | live — called from `core/main.cpp:643,709,741`, `core/game_core.cpp:159,164,295,323`, `input/{keyboard,mouse,joystick}.cpp` |
| `menu_data.h/.cpp` | 362 / 3707 | `cMenu_Base` + the five concrete menus and all 58 CEGUI handlers; `cMenu_Options` alone is ~1800 lines (`menu_data.cpp:1174-2965`) | live; contains one dead subsystem (§7: `Change_*_Setting`) |
| `hud.h/.cpp` | 285 / 1407 | `cHudSprite`, `cHud_Manager` (`pHud_Manager`), the six status widgets behind the `pHud_*` globals, `cDebugDisplay` | live — `pHud_Points` in 15 non-gui files, `pHud_Debug` in 13 |
| `generic.h/.cpp` | 146 / 734 | modal `cDialogBox_Text`/`cDialogBox_Question`, `Draw_Static_Text`, `Gui_Handle_Time`, clipboard + GUI copy/paste | live — `Box_Question` from `level_settings/level_editor/world_editor`, `Box_Text_Input` from 4 files, copy/paste from `input/keyboard.cpp:160-176`; the `__APPLE__` Carbon branch is untested by the author's own comment (`generic.cpp:502`) and mis-typed (§7) |
| `spinner.h/.cpp` | 56 / 59 | `CEGUI::SMC_Spinner` — a `Spinner` subclass overriding `getTextFromValue` to use `float_to_string(d_currentValue, 6, 0)` (trims trailing zeros) | live — factory registered `video/video.cpp:165 addFactory<CEGUI::SMC_SpinnerFactory>()`, used by 6 layout files (`menu/tab_{game,video,joystick,editor}.layout`, `level_settings/tab_{main,background}.layout`); its `Mouse_Wheel` handler is commented out (§7) |

Nothing in `gui/` is unreferenced at file level. Dead/vestigial *functions* are itemised in §7.

## 2. The menu system

### Object graph

```
pMenuCore : cMenuCore                        (menu.cpp:739, one global, lives for the process)
 ├─ m_handler : cMenuHandler*                (menu.cpp:275)
 │    ├─ m_level : cLevel*   ← loads pPreferences->m_menu_level ("menu_green_1"/"menu_blue_1")  menu.cpp:126
 │    ├─ m_camera : cCamera*, m_player : cSprite* (MASS_PASSIVE stand-in so pActive_Player is non-null) menu.cpp:120-123
 │    ├─ the SMC logo, added to m_level->m_sprite_manager as TYPE_FRONT_PASSIVE            menu.cpp:129-134
 │    └─ m_items : vector<cMenu_Item*>, m_active : int (-1 = none)                          menu.h:101-104
 ├─ m_animation_manager : cAnimation_Manager*  (two cloud cParticle_Emitters, left/right) menu.cpp:278-310
 ├─ m_menu_id : MenuID
 └─ m_menu_data : cMenu_Base*                 (the CURRENT menu; new'd in Load, deleted in Unload) menu.cpp:544-593,694-703
```

The handler and its level are constructed **once** (`cMenuCore::cMenuCore`) and reused; `Load` only
`Reset()`s the item list (`menu.cpp:548`). So the "menu level" background (a real `cLevel` with its own
sprite manager, drawn via `m_level->Draw_Layer_1()` at `menu.cpp:243`) and the cloud emitters persist
across menu switches — that is the flying-background machinery. `cMenu_Base::Update` only advances
`pMenuCore->m_animation_manager` when the menu is *not* overlaid on a level/world
(`menu_data.cpp:119-123`), and `cMenuCore::Update` only ticks the handler's level in the same case
(`menu.cpp:713-716`). `cMenu_Credits::Update` adds fireworks emitters straight into
`pMenuCore->m_animation_manager` (`menu_data.cpp:3493-3536`).

### `MenuID` and the transition table

`core/global_game.h:367-376`: `MENU_NOTHING=0, MENU_MAIN=1, MENU_OPTIONS=2, MENU_LOAD=3, MENU_SAVE=4,
MENU_START=5, MENU_CREDITS=6` (note MAIN..SAVE are 1-4 and START is 5: numbering is historical). Unscoped
enum, serialised as an integer string through `Game_Action_Data_Middle.add("load_menu", int_to_string(id))`
and read back with `static_cast<MenuID>(getValueAsInteger(...))` at `core/game_core.cpp:163`.

Menus never construct each other. Every transition is a **deferred request** through the global
`Game_Action` / `Game_Action_Data_{Start,Middle,End}` triple, executed later by `core/game_core.cpp`
(`Handle_Game_Events`): `"unload_menu"` → `pMenuCore->Unload()`, `"load_menu"` → `pMenuCore->Load(menu,
"menu_exit_back_to")`, and `MENU_START` + `"menu_start_current_level"` → `cMenu_Start::Highlight_Level`
(`game_core.cpp:157-171`; that is the only caller of `Highlight_Level`).

| From | Trigger | Sets | Anchor |
|---|---|---|---|
| `cMenu_Main` | item 0..3 | `GA_ENTER_MENU` + `load_menu`=START/OPTIONS/LOAD/SAVE | `menu_data.cpp:291-313` |
| `cMenu_Main` | item 4 | `game_exit = 1` | `:315-318` |
| `cMenu_Main` | item 5 (Credits; only exists when `m_exit_to_gamemode == MODE_NOTHING`) | `load_menu`=CREDITS, `music_fadeout`=500 | `:320-325`, item created `:227-234` |
| `cMenu_Main::Exit` (ESC) | overlaid on level/world | `GA_ENTER_LEVEL`/`GA_ENTER_WORLD` + `unload_menu` | `:265-277` |
| `cMenu_Start` | Enter/double-click/`button_enter` | `Load_Campaign`/`Load_World`/`Load_Level` → `GA_ENTER_LEVEL`/`GA_ENTER_WORLD`, `Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM`, fades, `unload_menu`, `reset_save` | `:671-768` |
| `cMenu_Start::Exit` | ESC/`button_back` | `load_menu`=MAIN (+`menu_exit_back_to`) | `:514-522` |
| `cMenu_Options::Exit` | ESC/`button_back` | `pPreferences->Save()` then `load_menu`=MAIN | `:1844-1853` |
| `cMenu_Options` reset/apply buttons | | `load_menu`=OPTIONS (reload self) | `:2400-2413,2576-2597,2706-2719,2744-2757,2893-2906,2952-2965` |
| `cMenu_Savegames` | item 0..8 | load: `GA_ENTER_LEVEL`/`WORLD` + `load_savegame`; save: `Set_Save_Description` (modal `Box_Text_Input`) then `pSavegame->Save_Game`, back to MAIN | `:3096-3184` |
| `cMenu_Savegames` | item 9 (Back) | `Exit()` → MAIN | `:3074-3078` |
| `cMenu_Credits` | item 0 (Back) | `Exit()` → MAIN, `music_fadeout`=500 | `:3557-3560, 3443-3452` |

External entry points that push a menu: `core/main.cpp:209` (startup → MAIN), `level_player.cpp:438,444,3586-3595`,
`level_editor.cpp:475-478`, `level_manager.cpp:333`, `world_player.cpp:317-318,363-364`.

### Class table

| Class | Layout (`data/gui/layout/`) | Owns | Selection model |
|---|---|---|---|
| `cMenu_Base` (`menu_data.cpp:59-179`) | `m_layout_file` → `CEGUI::WindowManager::loadWindowLayout`, attached to `pGuiSystem->getGUISheet()`; destroyed in dtor | `m_gui_window`, `m_draw_list` (decorative `cHudSprite`s, deleted in dtor), `m_action`, `m_exit_to_gamemode` | — |
| `cMenu_Main` | `menu/main.layout` (`text_version`, `text_website`) | 5–6 `cMenu_Item`s via `pMenuCore->Auto_Menu` (`menu.cpp:521-542` loads `pixmaps/menu/<x>.png` + `menu/items/<x>.png`) | sprite items; **index-coded** (`m_handler->m_active == 0..5`) |
| `cMenu_Start` | `menu/start.layout` (tabcontrol_main, 3 listboxes, 2 description editboxes, 5 buttons) | search buffer `m_listbox_search_buffer` + timeout counter | CEGUI widgets; `Load_Selected` dispatches on `getSelectedTabIndex()` 0/1/else (`:634-669`) |
| `cMenu_Options` | `menu/options_main.layout` + 6 tabs added by `m_tabcontrol->addTab(wmgr.loadWindowLayout("menu/tab_*.layout"))` (`:1225-1241`) | 21 widget pointers (`menu_data.h:253-285`), staged video settings `m_vid_*` applied only on `Video_Button_Apply_Clicked` | CEGUI widgets; the sprite-item path is dead (§7) |
| `cMenu_Savegames(bool type)` | **none** (`m_layout_file` stays `""`, so `Init_GUI` returns at `:93-96`) | 9 pre-rendered description sprites `m_savegame_temp` (built in ctor `:2974-2977`, text refreshed by `Update_Saved_Games_Text`) + 10 `cMenu_Item`s (9 slots + Back) | sprite items; `m_active + 1` = slot number, 9 = Back |
| `cMenu_Credits` | **none** | ~100 `cHudSprite` lines in `m_draw_list` scrolled upward with fade-in/out + particle bursts (`:3454-3561`), 1 Back item | sprite items |

### Input: who gets what

- **Mouse hover selects**: `cMenuCore::Handle_Event` (`menu.cpp:321-338`) forwards only `SDL_MOUSEMOTION`
  to `cMenuHandler::Update_Mouse`, which hit-tests `m_col_rect` against `pMouseCursor` and calls
  `Set_Active(found)` (`menu.cpp:207-228`; it ignores the first synthetic motion by comparing the global
  `input_event.motion.x`). Left click on the hovered item sets `m_menu_data->m_action = 1`
  (`cMenuCore::Mouse_Down`, `menu.cpp:481-502`).
- **Keyboard**: `input/keyboard.cpp:135-139` injects into CEGUI *first* (`CEGUI_Handle_Key_Down`), so a
  focused listbox/tabcontrol's `EventKeyDown` handlers in `cMenu_Start` win; only unconsumed keys reach
  `pMenuCore->Key_Down` (`keyboard.cpp:205`). `cMenuCore::Key_Down` (`menu.cpp:340-420`) cycles
  `m_active` on `SDLK_DOWN`/`SDLK_UP` *or* `pPreferences->m_key_down/up` with wrap-around, sets
  `m_action` on Return/KP_Enter, `Exit()`s on ESC, and has a **Ctrl+X debug shortcut** that loads a random
  level (see §7). `Key_Up`, `Joy_Button_Up`, `Mouse_Up` are `if(0){}` stubs (`menu.cpp:422-437,464-479,504-519`).
- **Joystick**: `cMenuCore::Joy_Button_Down` maps `m_joy_button_action` → `m_action`, `m_joy_button_exit`
  → `Exit()` (`menu.cpp:439-462`); axis movement is translated to key events upstream (the `// todo:
  detect event for joystick better` comments at `menu.cpp:342,354`).
- **Within CEGUI widgets** (`cMenu_Start`): `TabControl_Keydown` (`:791-850`) maps Return→`Load_Selected`,
  the *configured* left/right keys (`pKeyboard->SDLKey_to_CEGUIKey(pPreferences->m_key_left)`) and
  Shift+Tab to tab switching; `Listbox_Keydown` (`:852-945`) implements arrow/page/home/end with wrap;
  `Listbox_Character_Key` (`:947-991`) is a type-ahead prefix search (1-second buffer, `speedfactor_fps`).
- **`m_action` protocol**: the `cMenu_*::Update` overrides check `if(!m_action) return; m_action = 0;`
  then act on `pMenuCore->m_handler->m_active` — the item index is the *only* identity an item has
  (`cMenu_Item::m_is_quit` is written at 3 sites and **never read**: `menu.cpp:45,539`, `menu_data.cpp:3013,3399`).

### Enter / Leave

`cMenuCore::Enter` (`menu.cpp:595-672`) swaps the four "active" globals (`pActive_Camera`,
`pActive_Player`, `pActive_Animation_Manager`, and the HUD/mouse sprite managers), pre-runs particle
emitters when coming from `MODE_NOTHING/LEVEL/OVERWORLD`, delegates to `m_menu_data->Enter`, then picks
music: credits → `land/hyper_1.ogg`; overlaid on a loaded level/world → that one's music; else
`game/menu.ogg`. `Leave` hides the mouse unless the editor is on and delegates. Only `cMenu_Credits`
overrides `Enter/Leave` (for `glClearColor` and the blocking `Menu_Fade`, `menu_data.cpp:3415-3441`).

## 3. CEGUI seams

### `subscribeEvent` inventory

`rg -c subscribeEvent gui/` → **menu_data.cpp 58, generic.cpp 6, spinner.cpp 1 (commented out)**.
Every one is `CEGUI::Event::Subscriber(&cX::Handler, this)`; **no `Event::Connection` is stored and nothing
is ever disconnected** (`rg "Event::Connection|removeEvent|disconnect" gui/` → none). Lifetime safety
rests entirely on the subscribed *window* dying with (or before) the subscriber object:

| Owner | Where subscribed | Windows | Torn down by |
|---|---|---|---|
| `cMenu_Start` (16) | `Init_GUI` `:381-508` | tabcontrol ×2, listbox_campaigns ×4, listbox_worlds ×4, listbox_levels ×4, 5 buttons | `~cMenu_Base` `destroyWindow(m_gui_window)` `:72-76` — child windows go with the layout root |
| `cMenu_Options` (36) | `Init_GUI*` `:1220-1840` | back button, 6 tab layouts' widgets (incl. `getEditbox()` of the menu-level combobox `:1355`) | same |
| `cMenu_Start` (temporary) | `cMenuCore::Key_Down` Ctrl+X `menu.cpp:383-394` | a second `start.layout` while another may be loaded | `delete menu_start` |
| `cDialogBox_Text` (1) | `Init` `generic.cpp:121` close button | `Exit()` `destroyWindow(window)` |
| `cDialogBox_Question` (5) | `Init`/`Enter` `generic.cpp:219,223,261-263` | `Exit()` |

Handler signatures are uniform `bool f(const CEGUI::EventArgs&)`, recovering the sender via
`static_cast<const CEGUI::WindowEventArgs&>(e).window` (or `KeyEventArgs` for key events). Three of
them are also called *directly* with a hand-built `CEGUI::WindowEventArgs(widget)` from the dead
`Change_*_Setting` path (`menu_data.cpp:1927,1949,1976,2049`).

### Window creation / destruction

- Layout loads: `WindowManager::getSingleton().loadWindowLayout(name)` at `menu_data.cpp:98,1225-1240`,
  `generic.cpp:61,359`, `hud.cpp:887`; every root is parented with
  `pGuiSystem->getGUISheet()->addChildWindow(w)` and removed with `removeChildWindow` + `destroyWindow`
  (`menu_data.cpp:74-75`, `generic.cpp:80-81,431-432`, `hud.cpp:935-936`).
- **Global-name lookups**: every widget is fetched by *flat global name* via
  `WindowManager::getSingleton().getWindow("listbox_levels")` etc. — 80+ sites. This is why
  `cMenuCore::Key_Down` can probe `isWindowPresent("listbox_levels")` (`menu.cpp:380`) to decide whether
  the start menu is loaded. A second `start.layout` load while one exists would collide on names.
- **Never destroyed**: the `ListboxTextItem`s created with `new` (`menu_data.cpp:392,443,583,1262,...` — 25
  sites) are owned by their listbox and freed by CEGUI (item auto-delete default); the ones removed via
  `removeItem` (`:594,1153`) are likewise CEGUI-freed.

### String conversions

- `reinterpret_cast<const CEGUI::utf8*>(std_string.c_str())` — 12 sites (`menu_data.cpp:392,443,583,1005,1042`,
  `generic.cpp:119,132-133,233,366,694,718`, `hud.cpp:971`) — the CEGUI-0.7 idiom for "this `char*` is UTF-8".
- `UTF8_(x)` = `reinterpret_cast<CEGUI::utf8*>(gettext(x))` (`core/i18n.h:30`) — 80+ sites in
  `cMenu_Options` for translated widget captions.
- Back to `std::string`: `.getText().c_str()` everywhere (`CEGUI::String::c_str()` returns UTF-8).
- `CEGUI::PropertyHelper::intToString` / `colourToString` (`menu_data.cpp:255,702,706,...` 9 sites; `generic.cpp:365`).

### Clipboard / dialog helpers (`generic.cpp`)

- `GUI_Copy_To_Clipboard(cut)` / `GUI_Paste_From_Clipboard()` (`:586-730`): find
  `System::getSingleton().getGUISheet()->getActiveChild()`, sniff the widget's *type string* with
  `getType().find("/MultiLineEditbox")` / `("/Editbox")`, then splice
  `getSelectionStartIndex()/getSelectionLength()` on `getText()` and `setCaratIndex`. The two
  branches are near-duplicates (both widget types expose the same 5 methods but share no base that does).
  Called only from `input/keyboard.cpp:160-176` on Ctrl+C/X/V.
- `Get_Clipboard_Content` / `Set_Clipboard_Content`: `_WIN32` Win32 API, `__APPLE__` Carbon Scrap (untested),
  `__unix__` SDL2 (`SDL_GetClipboardText`/`SDL_SetClipboardText`, from `d438165`).
- `Box_Text_Input` / `Box_Question` (`:435-451`) wrap `cDialogBox_Text::Enter` / `cDialogBox_Question::Enter`
  — each runs its **own nested SDL event loop** (`:139-178`, `:267-309`) pumping `pKeyboard->CEGUI_Handle_Key_*`
  and `pMouseCursor->Handle_Event`, redrawing the game underneath via `Draw_Game()` + a gradient
  (`cDialogBox::Draw`, `:84-92`). `Draw_Static_Text` (`:353-433`) is the same shape for a one-shot message
  (`wait_for_input` loops until any key/button; false = draw once and return).
- `Gui_Handle_Time` (`:339-351`): `System::injectTimePulse(dt)` from `SDL_GetTicks`, called by `core/main.cpp`.

### CEGUI 0.7 API surface used by `gui/` (the compatibility list)

`WindowManager::getSingleton()`, `.loadWindowLayout(String)`, `.getWindow(String)` (flat namespace),
`.isWindowPresent`, `.destroyWindow`; `System::getSingleton()`, `.getGUISheet()`, `.injectTimePulse`,
`.renderGUI()` (`menu_data.cpp:2581`), `System::create(NullRenderer …)` (in video.cpp, not here);
`Window::addChildWindow/removeChildWindow/activate/hide/moveToFront/setVisible/setText/setTooltipText/
setProperty/setWidth/setHeight/setXPosition/setSize/getType/getActiveChild/getFont`; `UDim`, `UVector2`;
`CEGUI::colour` (lower-case; `setRed/setGreen`, `d_top_left…`); `PropertyHelper::intToString/colourToString`;
`FontManager::getSingleton().get(name)`, `Font::getTextExtent/getLineSpacing/isCodepointAvailable`;
`Editbox::setCaratIndex/getCaratIndex/getSelectionStartIndex/getSelectionLength/isReadOnly` (also on
`MultiLineEditbox`); `FrameWindow::getCloseButton/setSizingEnabled`; `PushButton::EventClicked`;
`Listbox::addItem/removeItem/resetList/findItemWithText(text, NULL)/getListboxItemFromIndex/getItemIndex/
getItemCount/getFirstSelectedItem/setItemSelectState(item|index, bool)/ensureItemIsVisible/
setSortingEnabled`, events `EventSelectionChanged`, `EventMouseDoubleClick`;
`ListboxTextItem(text, id, userdata)`, `setTextColours(1|4 colours)/getTextColours/setSelectionColours/
setSelectionBrushImage("TaharezLook", "ListboxSelectionBrush")` (two-arg imageset form);
`Combobox::getSelectedItem/getEditbox`, `EventListSelectionAccepted`; `Spinner::setCurrentValue/
getCurrentValue`, `EventValueChanged`, protected `d_inputMode/d_currentValue/FloatingPoint`
(`spinner.cpp:40-42`); `Slider` same pair; `TabControl::addTab/setSelectedTab(name)/
setSelectedTabAtIndex/getSelectedTabIndex/getTabCount`, `EventSelectionChanged`;
`MultiColumnList::addColumn/addRow/setItem(item, col, row)/resetList/getFirstSelectedItem`;
`Window::EventKeyDown/EventCharacterKey/EventMouseClick`, `KeyEventArgs::scancode/codepoint/window`,
`CEGUI::Key::{Return,NumpadEnter,Tab,ArrowUp,ArrowDown,PageUp,PageDown,Home,End}`;
`CEGUI_DECLARE_WINDOW_FACTORY` + `WindowFactoryManager::addFactory<>`; `CEGUI::utf8`, `CEGUI::String`.
Of these, `getWindow`/`isWindowPresent`/`loadWindowLayout`/`getGUISheet`/`renderGUI`/`setCaratIndex`/
`colour`/`PropertyHelper::intToString`/two-arg `setSelectionBrushImage`/`UVector2`-as-size are the
ones CEGUI 0.8 renamed or removed — this list *is* the port cost if the pin ever moves.

## 4. HUD

`cHud_Manager : cObject_Manager<cHudSprite>` (`hud.h:39-67`, `pHud_Manager` at `hud.cpp:247`) is created
once and `Load()`ed (`hud.cpp:79-114`) to build seven widgets **in a fixed order** — `cMenuBackground`
(index 0, relied on by `Update_Text` `:141`), then the globals `pHud_Points`, `pHud_Time`, `pHud_Lives`,
`pHud_Goldpieces`, `pHud_Itembox`, `pHud_Debug`. `Unload` (`:116-133`) `Delete_All()`s and nulls the six
globals. `Update_Text` (`:135-214`) is the layout pass: positions depend on `Game_Mode != MODE_OVERWORLD`
(y = 18 vs 4) and it forces a re-render via `Add_Lives(0)`/`Add_Gold(0)`/`Add_Points(0)`.
`Set_Sprite_Manager` (`:234-243`) re-parents all widgets when the active level/world/menu changes
(called from `cMenuCore::Enter` `menu.cpp:606`).

| Widget | Type tag | Reads/writes | Draw gate |
|---|---|---|---|
| `cMenuBackground` | `TYPE_HUD_BACKGROUND` | blits `game/maryo_l.png`, `game/gold_m.png` at fixed `GL_point`s | not in editor/menu; head not in overworld (`:294-309`) |
| `cPlayerPoints : cStatusText` | `TYPE_HUD_POINTS` | **writes** `pLevel_Player->m_points` (`Set_Points` `:437-444`); floating `PointsText` list (max 50, `reserve(50)`) with velocity/alpha fade + screen clamping (`:353-435`) | not in editor/menu |
| `cGoldDisplay` | `TYPE_HUD_GOLD` | **writes** `pLevel_Player->m_goldpieces`; at ≥100 plays `item/live_up_2.ogg`, `pHud_Lives->Add_Lives(1)`, spawns a "1UP" points text (`:524-541`) — game logic living in the HUD | not in editor/menu |
| `cLiveDisplay` | `TYPE_HUD_LIFE` | **writes** `pLevel_Player->m_lives`; right-aligns via `TTF_SizeText` (`:576-606`) | not in editor/menu |
| `cTimeDisplay` | `TYPE_HUD_TIME` | accumulates `pFramerate->m_elapsed_ticks`, re-renders only when the second changes (`:629-659`) | not in editor/overworld/menu |
| `cItemBox` | `TYPE_HUD_ITEMBOX` | owns a `cMovingSprite *m_item` that *falls* through the level when requested, collides with `TYPE_PLAYER` via `Collision_Check(..., COLLIDE_ONLY_BLOCKING)` and calls `pLevel_Player->Get_Item` (`:720-767`); 3-way `if` on `item_type` for mushroom/fireplant/blue (`:806-820`) | not in editor/overworld/menu |
| `cDebugDisplay` | `TYPE_HUD_DEBUG` | 23 pre-positioned `cHudSprite`s for the `game_debug` panel (`:898-928`), a CEGUI `debugtext.layout` window for `Set_Text` messages (`:886-892`, sized with `Font::getTextExtent`), and the `game_debug_performance` panel built from `pFramerate->m_perf_timer[PERF_*]` (`:1229-1394`) | `Draw` always runs; each panel gated by its flag |

**Text rendering path**: `pFont->Render_Text(TTF_Font*, const std::string&, Color)` (`video/font.h:48`)
returns a fresh `cGL_Surface*`; the widget calls `Set_Image(surface, new_start_image, del_img=1)` so
the sprite takes ownership and deletes the previous surface on the next `Set_Image`
(`objects/sprite.cpp:467-483` — `m_delete_image` → `delete m_image`). Every status update therefore
re-rasterises a TTF string to a new GL texture (`rg -c "Render_Text" hud.cpp` → 48 sites; the
performance panel renders ~20 strings *per frame* and deletes them, `hud.cpp:1375-1390`, marking
`request->m_delete_texture = 1` and `surface_temp->m_auto_del_img = 0` to hand the texture to the renderer).

**The three `sprintf`s into `char[]`** (`rg -n sprintf gui/`):

```cpp
hud.cpp:441  char text[70];
hud.cpp:442  sprintf( text, _("Points %08d"), static_cast<int>(pLevel_Player->m_points) );
hud.cpp:657  sprintf( m_text, _("Time %02d:%02d"), minutes, seconds - ( minutes * 60 ) );   // m_text is char[50], hud.h:181
hud.cpp:673  sprintf( m_text, "Time" );
```

Line 657 passes `Uint32` for `%d` and a *translated* format string (`_()`), so a translator can change the
conversion count — the classic gettext+`sprintf` hazard. Everything else builds strings with `+` and
`int_to_string`/`float_to_string` (`hud.cpp` 48 sites, `menu_data.cpp` 43).

## 5. Type tags and dispatch

| Site | Discriminant | Form | Default? | Classification |
|---|---|---|---|---|
| `cMenuCore::Load` `menu.cpp:561-589` | `m_menu_id : MenuID` | 6-arm `if/else if` → `new cMenu_X` | **none** — `MENU_NOTHING` leaves `m_menu_data` dangling from `Unload()`'s null, then `:591` dereferences it | factory; a `switch` with `default: abort` or a table |
| `cMenuCore::Handle_Event` `:323-335` | `ev->type` | `switch` with `default` | yes | fine |
| `cMenu_Main::Update` `menu_data.cpp:291-325` | `m_handler->m_active` int | 6-arm `if` | silent | index-coded items; wants an enum or per-item callback |
| `cMenu_Savegames::Update`/`Update_Load`/`Update_Save` `:3074-3176` | `m_active` (9 = back, else slot `m_active+1`) | `if` | — | same |
| `cMenu_Start::Load_Selected` `:640-668`, `TabControl_Selection_Changed` `:775-786` | `getSelectedTabIndex()` 0/1/2 | `if/else if/else` | last arm is the catch-all | fine but index-coupled to the layout |
| `cMenu_Options::Change_*_Setting` `:1881-2126` | `m_active` 5..12 | `if` chains | silent | **dead** (§7) |
| `cMenu_Base::Draw`/`Update` `:119,133-161`; `cMenuCore::Update/Enter` | `m_exit_to_gamemode : GameMode` | `if/else if/else` | else = standalone | mode overlay logic |
| `cMenuCore::Enter` `:655-670` | `m_menu_id == MENU_CREDITS` + mode | `if` chain | else = menu music | fine |
| `cItemBox::Set_Item` `hud.cpp:806-820` | `item_type : SpriteType` | 3-arm `if`, **no else** — unknown type leaves the old image visible with `m_item_id` set | none | wants `switch` + default |
| `cDebugDisplay::Draw_Performance_Debug_Mode` `:1257-1346` | `Game_Mode` | 4-arm `if` ×2, `else` = level | yes | data-driven table would remove 40 `push_back("- ")` lines |
| `cMenuBackground/cStatusText/…::Draw` | `Game_Mode`, `editor_enabled` | guard `if` | — | fine |
| `GUI_Copy_To_Clipboard`/`Paste` `generic.cpp:608-650,677-727` | `window->getType().find("/Editbox")` **string sniffing** | `if/else if/else return 0` | yes | wants `dynamic_cast` |
| `cMenuCore::Enter` `menu.cpp:623-643` | `obj->m_type == TYPE_PARTICLE_EMITTER` then `static_cast` | loop + `if` | — | type-tag downcast, twice (level sprites and animation objects) |

## 6. Old-pattern counts (all `rg` over `smc/src/gui/`)

| Pattern | Command | Count |
|---|---|---|
| raw `new` | `rg -c "\bnew\b"` | generic 9, hud 18, menu 18, menu_data 63 = **108** (≈25 are `new CEGUI::ListboxTextItem`, CEGUI-owned) |
| raw `delete` | `rg -c "\bdelete\b"` | generic 3, hud 6, menu 10, menu_data 3 = **22** |
| `NULL` | `rg -c "\bNULL\b"` | 76 (hud.cpp 23, menu_data.cpp 30, …) |
| `typedef` | `rg -n typedef` | 5 — three separate `HudSpriteList` typedefs of the same `vector<cHudSprite*>` (`hud.h:63,267`, `menu_data.h:67`), `PointsTextList`, `MenuList` |
| iterator loops | `rg -c "::(const_)?iterator"` | menu_data 13, hud 13, menu 4 = **30** |
| `virtual` / `override` | `rg -c "\bvirtual\b"` / `"\boverride\b"` | 77 / **0** |
| unscoped enums | `rg -n "\benum\b" gui/` | 0 defined here; consumes `MenuID`, `GameMode`, `SpriteType`, `PERF_*` from `core/global_game.h` (all unscoped) |
| `#define` | | 5, all include guards |
| `bool = 0/1` literals | `rg -c "(bool [a-z_]+ = [01]\b\|= [01];\|return [01];\|/\* = [01] \*/)"` | **140** (menu_data.cpp 83, generic.cpp 22, menu.cpp 18, …) — the codebase never spells `true`/`false` |
| `sprintf` / `char[]` | | 3 / 2 (§4) |
| C-style casts | `rg -n "\((int\|float\|unsigned int\|Uint8\|char \*\|size_t)\)\s*[a-zA-Z(]"` | **0** — the author used `static_cast`/`reinterpret_cast` consistently: 299 sites (menu_data.cpp 214) |
| `ostringstream` | | 0 |
| `int_to_string`/`float_to_string`/`string_to_int` | | menu_data 43, hud 48, spinner 1 = **92** |
| `printf` diagnostics | `rg -n "\bprintf"` | 7 |
| nested modal event loops | `rg -n "while\( (!finished\|draw\|!sub_done\|1) \)"` | 5 (`generic.cpp:139,267,382`, `menu_data.cpp:2258,3623`) |
| `subscribeEvent` w/o stored connection | | 64 (§3) |

## 7. Dead or suspicious code

**Dead**

1. `cMenu_Options::Change_{Game,Video,Audio,Keyboard,Joystick,Editor}_Setting` (`menu_data.cpp:1881-2126`,
   ~250 lines): the only caller is `cMenu_Options::Update` `:1867-1878`, which first does
   `if( pMenuCore->m_handler->m_active > 0 ) return;` and the options menu never adds any `cMenu_Item`
   (its `Init` `:1185-1208` leaves `temp_item` unused), so `m_active` is always -1 and every
   `m_active == 5..12` branch is unreachable. The `// todo : use this functionality again` at `:1872` confirms
   it is a leftover from the pre-CEGUI sprite-item options screen. Inside it: `Change_Video_Setting` tests
   `m_active == 8` twice (`:2007,2021` — VSync and FPS-limit share an index) and `Change_Editor_Setting`'s
   auto-hide branch writes the wrong combobox (`:2119,2123` set `m_game_combo_editor_show_item_images`, not
   `m_game_combo_editor_mouse_auto_hide`). `Change_Keyboard_Setting`/`Change_Joystick_Setting` are `// todo` empties.
2. `cMenu_Item::m_is_quit` — written 3×, never read (§2).
3. `cMenuCore::Key_Up`, `Joy_Button_Up`, `Mouse_Up` — `if( 0 ) { // }` stubs returning 0 (`menu.cpp:422-437,464-479,504-519`); still wired from `input/*.cpp`.
4. `cMenu_Start::Level_Select` (`:1067-1084`) — handler body is two `// todo : needs level manager` comments; subscribed at `:488`.
5. `SMC_Spinner::Mouse_Wheel` — declared and defined only inside comments (`spinner.h:45`, `spinner.cpp:30,48-55`), waiting on a virtual `Spinner::initialiseComponents` that 0.7 lacks.
6. `cMenuHandler::m_player`, `m_camera` — exist only so `pActive_Player`/`pActive_Camera` are non-null in menu mode (`menu.h:95` comment, `menu.cpp:598-600`).
7. `cMenu_Options::Init` `cMenu_Item *temp_item = NULL;` `:1199` — unused local.
8. `cHudSprite::Copy` (`hud.cpp:51-63`) — no direct HUD-typed caller found; reachable only polymorphically via `cSprite::Copy()` (`level.cpp:1410-1431`, `editor.cpp`, …) on HUD-typed sprites, which no level file contains.

**Suspicious / real bug smells**

9. **Wrong window name, uncaught exception**: `cMenu_Options::Joystick_Name_Click` `:2762`
   `getWindow( "combo_joy" )` — the layout defines `Name="joystick_combo"` (`tab_joystick.layout:9`) and
   `Init_GUI_Joystick` fetches `"joystick_combo"` (`:1681`). The handler is live (subscribed to
   `EventMouseClick` on the "Joystick" label, `:1679`), so clicking that label makes CEGUI throw
   `UnknownObjectException`; the only `try/catch` (`core/main.cpp:181-190`) wraps `Init_Game`, not the loop → `std::terminate`.
10. **Division by zero**: Ctrl+X debug shortcut `menu.cpp:389,401` `rand() % listbox_levels->getItemCount()` with an empty level list.
11. **`m_menu_data` null deref**: `menu.cpp:410,452` `m_menu_data->Exit()` on ESC / joy-exit lack the `if( m_menu_data )` guard the sibling branches have; and `Load` with `MENU_NOTHING` leaves `m_menu_data == NULL` then calls `:591` `m_menu_data->Set_Exit_To_Game_Mode`.
12. **No-op call**: `cDebugDisplay::Set_Text` `hud.cpp:1012` `m_text_old.empty();` — a query, not `clear()`.
13. **Off-by-one guard**: `cMenuHandler::Get_Active_Item` `menu.cpp:255` `m_active > m_items.size()` should be `>=` (masked because `Set_Active` never stores `size()`).
14. **`return false;` from a `std::string` function**: `generic.cpp:506,513` (`__APPLE__` branch) constructs `std::string(nullptr)` → UB. Untested platform, author-flagged.
15. **Reading the wrong union member / busy-wait**: `cMenu_Options::Set_Shortcut` `:2266` reads
    `input_event.key.keysym.sym` before checking `input_event.type`, and `:2261-2264` spins with `continue` on no event (100 % CPU until a key).
16. **gettext + `sprintf`** at `hud.cpp:442,657` (§4).
17. **Two different "current joystick" mechanisms** — `Joystick_Name_Click` (label click cycles) and `Joystick_Name_Select` (combobox) both call `Joy_Default/Joy_Disable`; the first draws a blocking `Draw_Static_Text` (`:2803`).
18. `cGoldDisplay::Set_Gold` (`hud.cpp:524-541`) performs the 100-coins → 1-up rule — game logic in a display widget, and it calls `pHud_Lives`/`pHud_Points` which may be `NULL` between `Unload` and `Load`.
19. `Draw_Static_Text` and `cDialogBox_Question::Enter` are annotated `// fixme : Can't handle multiple lines` (`generic.cpp:238,355`) yet `cMenu_Savegames::Set_Save_Description` `:3197` and `Audio_Music_Select` `:2655` pass `\n`.
20. Duplicated handler bodies: `Campaign_Select/World_Select` (`:993-1051`), the 4 pairs `*_final_list`, 9 `On/Off`-combobox handlers in `cMenu_Options` (`Game_Always_Run_Select`, `Video_Fullscreen_Select`, `Video_Vsync_Select`, `Audio_Music_Select`, `Audio_Sound_Select`, `Joystick_Analog_Jump_Select`, `Game_Editor_*` ×2), and the 6 identical "reset → reload MENU_OPTIONS" tails (`:2405-2411` ×6).
21. `menu_data.cpp:48-51` `#ifdef None #undef None` — X11's `None` macro leaking from `SDL_syswm.h` into `CEGUIMultiColumnList.h`; harmless post-SDL2 but still present.

`todo`/`fixme` total: `rg -n -i "todo|fixme" gui/` → 22 (menu_data 16, menu 2, generic 2, hud 1, spinner 1).

## 8. Modern-C++ hot spots (top five)

1. **`Event::Subscriber(&cX::f, this)` → lambdas with stored `CEGUI::Event::Connection`** (64 sites, §3).
   Capture `this` (or the widget pointer, killing the `WindowEventArgs` recovery boilerplate at the top of
   every handler — 40 identical two-line preambles) and keep a `std::vector<CEGUI::Event::Connection>`
   in `cMenu_Base`, disconnected in its dtor; the 9 On/Off combobox handlers collapse to one helper taking
   a `bool&`. Also lets `Change_*_Setting` (dead) be deleted rather than kept for its direct calls.
2. **Owning raw pointers → `std::unique_ptr` / value members** (108 `new`, 22 `delete`): `cMenuCore`
   owns `m_handler`, `m_animation_manager`, `m_menu_data`; `cMenu_Item` owns two `cHudSprite`s;
   `cMenuHandler::m_items`, `cMenu_Base::m_draw_list`, `cMenu_Savegames::m_savegame_temp`,
   `cPlayerPoints::m_points_objects`, `cDebugDisplay::m_sprites` are `vector<T*>` + manual delete loops
   (`menu.cpp:165-176`, `menu_data.cpp:78-83,2982-2987`, `hud.cpp:488-496,938-943`) → `vector<unique_ptr<T>>`.
   The three `HudSpriteList` typedefs become one alias. `Box_Text_Input`/`Box_Question` (`generic.cpp:435-451`)
   new/delete a dialog that could be a stack object.
3. **`sprintf` + `char[]` → `std::format`** at `hud.cpp:441-443,657-658,673` (`std::format(_("Points {:08d}"), …)`
   also survives translator-reordered args), and the 92 `int_to_string( … ) + " " + …` concatenations → `std::format`.
4. **Index-coded menu items → an item type with a callback / enum** (`cMenu_Main::Update` `:291-325`,
   `cMenu_Savegames::Update` `:3074-3087`, `cMenu_Credits` `:3557`): give `cMenu_Item` a
   `std::function<void()> on_activate` (or a `MenuAction` scoped enum) and let `cMenuHandler` invoke it,
   deleting `m_is_quit`, the `m_action` flag round-trip, and the `m_active == N` chains. `MenuID`/`GameMode`
   become `enum class` and stop travelling through `int_to_string` in `Game_Action_Data`.
5. **`cShortcut_item{void*, const void*, bool joystick}` → `std::variant<SDLKey*, Uint8*>` or two typed
   lists** (`menu_data.h:288-301`, `menu_data.cpp:2134-2298`): `Build_Shortcut_List` and `Set_Shortcut`
   `static_cast` the `void*` back based on a bool and stash it in `ListboxTextItem` user-data
   (`:2192,2729,2887`). A `std::variant` + `std::visit` (or a small `struct Binding { CEGUI::String name;
   std::variant<…> current, dflt; }`) removes all the casts and the duplicated keyboard/joystick branches.

Runners-up: iterator loops → range-`for` (30); `NULL` → `nullptr` (76); `0/1` → `false/true` (140);
`virtual` overrides → `override` (77/0); `getType().find("/Editbox")` → `dynamic_cast<CEGUI::Editbox*>`;
the five nested modal loops → a single reusable `Run_Modal(predicate)`; the credits list → a `constexpr`
table of `{name, role, colour}`.

## 9. Cross-references

**Depends on** (includes from `gui/`): `core/{global_basic,global_game,game_core,main,framerate,i18n,
campaign_manager,sprite_manager,obj_manager,camera,editor?}` (`Game_Action*`, `Game_Mode`, `game_exit`,
`pFramerate`, `Draw_Game`, `Clear_Input_Events`, `Correct_Frame_Time`, `Loading_Screen_*`, `Preload_Sounds`),
`core/filesystem/{filesystem,resource_manager}` (`Get_Directory_Files`, `Delete_File`, `File_Exists`,
`pResource_Manager->user_data_dir`), `core/math/{utilities,size}`, `video/{video,renderer,font,gl_surface,
animation,img_manager}` (`pVideo`, `pRenderer`, `pFont`, `pImage_Manager`, `pGuiSystem`, `global_upscalex`…),
`audio/audio.h` (`pAudio`, `pSound_Manager`), `input/{keyboard,mouse,joystick}` (`pKeyboard->
CEGUI_Handle_Key_*`, `SDLKey_to_CEGUIKey`, `Is_Ctrl_Down`, `pMouseCursor`, `pJoystick`), `level/{level,
level_player,level_editor,level_manager}` (`pActive_Level`, `pLevel_Player` — the HUD **writes** its
points/lives/gold, `pLevel_Editor->Function_New`, `pLevel_Manager->{Get_Path,Load,m_camera}`),
`overworld/overworld.h` (`pActive_Overworld`, `pOverworld_Manager`, `pOverworld_Player`),
`user/{preferences,savegame}` (`pPreferences` read/written/`Save()`/`Reset_*`/`Apply_Video`, `pSavegame`),
`objects/{movingsprite,bonusbox}`, CEGUI 0.7 headers listed in §3, SDL2 (`SDL_GetClipboardText`,
`SDL_JoystickNameForIndex`, `SDL_GL_SwapWindow`, `TTF_SizeText`).

**Depended on by**: 38 non-gui files include a `gui/*.h`. Concretely — `core/main.cpp` (event/update/draw
of `pMenuCore`, `Gui_Handle_Time`), `core/game_core.cpp` (`Load/Unload/Enter/Leave`, `Highlight_Level`,
`cMenu_Start` downcast at `:168`), `input/keyboard.cpp` (`Key_Down/Key_Up`, clipboard helpers,
`Box_Text_Input`, `Draw_Static_Text`), `input/mouse.cpp` and `input/joystick.cpp` (button forwarding),
`core/editor.cpp`, `level/level_editor.cpp`, `level/level_settings.cpp`, `overworld/world_editor.cpp`
(`Box_Question`/`Box_Text_Input`), `overworld/{overworld,world_player}.h` (own `cHudSprite` members for
world/level-name and waypoint-debug text), `video/video.cpp` (`SMC_Spinner` factory), and the 15/13/3/2/3/3
files that touch `pHud_Points/pHud_Debug/pHud_Goldpieces/pHud_Lives/pHud_Time/pHud_Itembox` (`pHud_Manager`
in 6). Layout data consumed: `data/gui/layout/{menu/*.layout, box_text, box_question, statictext, debugtext}.layout`
(the 6 `menu/tab_*.layout` files and 2 `level_settings/*.layout` instantiate `CEGUI/SMC_Spinner`).
