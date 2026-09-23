# The editors — cEditor base, level/world editors, per-object settings, the CEGUI census (L2)

Cross-cutting synthesis over `core/editor.cpp`, `level/level_editor`, `level/level_settings`,
`overworld/world_editor`, `input/mouse`, and every object/enemy's `Editor_Activate`. Provenance:
commit `d438165`. Baseline: an in-game level/world editor built entirely on CEGUI 0.7 windows.

## L2 — the structure

`cEditor` (in `core/editor.cpp`, 1778 lines) is both a `CEGUI::XMLHandler` (parses `editor_items`
and `editor_menu` XML into prototype objects and menu entries) and the runtime controller. Two
subclasses: `cEditor_Level` (`level/level_editor`) and `cEditor_World` (`overworld/world_editor`),
each overriding `Init`, `Enable`, `Disable`, `Key_Down`, `Set_Sprite_Manager`, `Get_Object` (→ the
factory), and the `Function_*` menu verbs.

It owns: the CEGUI window tree from `editor.layout`; `m_tagged_item_objects` (prototype `cSprite*`s
the subclass's `Get_Object` builds during XML parse); `m_tagged_item_images` (`cImage_Settings_Data*`).
Each palette row is a `cEditor_Item_Object` owned by CEGUI's Listbox, holding a **copy** of the
prototype sprite plus a `cEditor_CEGUI_Texture` wrapper (whose `cleanupOpenGLTexture` override
deliberately does not `glDeleteTextures` — the GL texture belongs to `cGL_Surface`). `Activate_Item`
copies the row's sprite again and hands it to `m_sprite_manager->Add`; from then the level owns it.
The editor's selection/copy/paste/mover machinery lives in `cMouseCursor` ([[input-flow]]).

`cLevel_Settings` (`MODE_LEVEL_SETTINGS`) is the per-level settings screen — a separate mode,
entered via `GA_ENTER_LEVEL_SETTINGS`, whose `Leave()` **is the save step**. It edits filename,
music, author, difficulty, land type, camera limits, gradient colours, and the background-image
layer list, all through CEGUI widgets fetched by global name.

## The CEGUI subscribeEvent census (fire-and-forget)

`subscribeEvent` binds a widget event to `CEGUI::Event::Subscriber(&cX::Handler, this)`. Across the
whole game there are ~216, and **not one stores the returned `Event::Connection` or ever
disconnects** — safety rests entirely on the widget dying with (or before) the subscriber. Census:

| Area | count | notes |
|---|---|---|
| `gui/menu_data.cpp` | 58 | the options menu dominates |
| `video/animation.cpp` | 31 live (+6 in comment blocks) | per particle-emitter editor field |
| `level/level_settings.cpp` | 21 | the level settings screen |
| `objects/` | 36 | moving_platform 12, path 10, level_exit 5, bonusbox 4, … |
| `enemies/` | 34 | per-enemy editor dialog |
| `core/editor.cpp` | 7 | tabs, listboxes, help |
| `overworld/` | 6 | waypoint + layer |
| `gui/generic.cpp` | 6 | dialog boxes |
| `level/level_player.cpp` | 1 | player direction combobox |

Every handler is `bool f(const CEGUI::EventArgs&)` and opens with the same two-line
`static_cast<const WindowEventArgs&>(e).window` preamble. The modernization target is lambdas with
captured `this`/widget and a stored `std::vector<Event::Connection>` disconnected in the owner's
destructor — this alone removes ~40 identical preambles and lets the dead `Change_*_Setting` path go.

## CEGUI 0.7 API surface (the compatibility cost if the pin moves)

The GUI reader enumerated the exact 0.7 calls the game relies on ([[gui]] §3). The ones CEGUI 0.8
renamed or removed — `WindowManager::getWindow`/`isWindowPresent`/`loadWindowLayout`,
`System::getGUISheet`/`renderGUI`, `Editbox::setCaratIndex`, lower-case `CEGUI::colour`,
`PropertyHelper::intToString`, two-arg `setSelectionBrushImage`, `UVector2`-as-size — *are* the port
cost. This is why `third_party/cegui-0.7.9` is vendored ([[../building-2010-era-smc-today]]) rather
than using Fedora's 0.8, and why "replace CEGUI" is a large project, not a swap.

## Notable editor bugs found (verified)

- `gui/menu_data.cpp:Joystick_Name_Click` looks up `getWindow("combo_joy")` but the layout names it
  `joystick_combo` → clicking the label throws `UnknownObjectException`, uncaught in the loop → terminate.
- `cEditor::Key_Down` maps Ctrl+Shift+S but Ctrl+S matches first (unreachable), and the F8 exit fakes
  a keypress through `pKeyboard->Key_Down(SDLK_F8)`.
- The whole `cMenu_Options::Change_*_Setting` subsystem (~250 lines) is dead (the options menu adds no
  sprite items, so its `m_active` guard is never satisfied) — a leftover from the pre-CEGUI screen.

Related: [[input-flow]], [[data-formats]], [[core]], [[gui]], [[../building-2010-era-smc-today]].
