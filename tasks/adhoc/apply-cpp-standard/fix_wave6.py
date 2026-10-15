#!/usr/bin/env python3
"""Phase D, wave 6: owning members that are always present, non-polymorphic and never replaced
become VALUE members (the standard's R.5 "prefer scoped objects"), with the rule of zero.

Converted to values (owner: members):
  cLevel: m_sprite_manager, m_background_manager, m_animation_manager
  cOverworld: m_sprite_manager (cWorld_Sprite_Manager), m_animation_manager, m_description,
              m_layer, m_hud_world_name, m_hud_level_name
  cMenuHandler: m_level, m_camera, m_player        cMenuCore: m_handler, m_animation_manager
  cMenu_Item: m_image_default, m_image_menu        cItemBox: m_item
  cLevel_Manager / cLevel_Settings / cOverworld_Manager: m_camera
  cEditor_Level: m_settings_screen                 cOverworld_Player: the three debug sprites
  cFramerate: m_perf_timer (vector<cPerformance_Timer> of values)
Converted to unique_ptr (polymorphic, replaced per menu): cMenuCore::m_menu_data.

Access sites are rewritten by rule -- `X->m_camera->f()` becomes `X->m_camera.f()` and a pointer
pass `f(X->m_camera)` becomes `f(&X->m_camera)` -- then the compiler drives the residue. The
`m_sprite_manager`/`m_animation_manager` rules are prefix-scoped because every sprite also has a
BORROWED pointer member of the same name that must stay a pointer.

Asserting and idempotent (the rules are no-ops on already-rewritten text). Paths are repo-relative.
"""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SRC = ROOT / "smc" / "src"


def edit(rel: str, old: str, new: str, count: int = 1) -> None:
    path = SRC / rel
    text = path.read_text()
    found = text.count(old)
    if found == 0 and new in text:
        return
    assert found == count, f"{rel}: expected {count} of {old[:70]!r}, found {found}"
    path.write_text(text.replace(old, new))


def sub(rel: str, pattern: str, repl: str, count: int, flags: int = 0) -> None:
    path = SRC / rel
    text = path.read_text()
    new_text, n = re.subn(pattern, repl, text, flags=flags)
    if n == 0:
        return
    assert n == count, f"{rel}: expected {count} of {pattern!r}, got {n}"
    path.write_text(new_text)


# ---------------------------------------------------------------------------------------------
# 1. declarations, constructors, destructors
# ---------------------------------------------------------------------------------------------
edit("level/level.h",
     "  // background manager\n  cBackground_Manager* m_background_manager;\n  // animation manager\n  cAnimation_Manager* m_animation_manager;\n  // sprite manager\n  cSprite_Manager* m_sprite_manager;",
     "  // the level's managers (owned values; constructed with the level)\n  cSprite_Manager m_sprite_manager;\n  cBackground_Manager m_background_manager;\n  cAnimation_Manager m_animation_manager;")
edit("level/level.cpp",
     "  m_sprite_manager = new cSprite_Manager();\n  m_background_manager = new cBackground_Manager();\n  m_animation_manager = new cAnimation_Manager();\n\n  // add default gradient layer\n  auto gradient_background = std::make_unique<cBackground>(m_sprite_manager);",
     "  // add default gradient layer\n  auto gradient_background = std::make_unique<cBackground>(&m_sprite_manager);")
edit("level/level.cpp", "cLevel ::~cLevel(void) {\n  Unload();\n\n  // delete\n  delete m_background_manager;\n  delete m_animation_manager;\n  delete m_sprite_manager;\n}",
     "cLevel ::~cLevel(void) { Unload(); }")

edit("overworld/overworld.h",
     "  // map objects\n  cWorld_Sprite_Manager* m_sprite_manager;\n  // animation manager\n  cAnimation_Manager* m_animation_manager;",
     "  // map objects (owned values; constructed with the overworld)\n  cWorld_Sprite_Manager m_sprite_manager;\n  // animation manager\n  cAnimation_Manager m_animation_manager;")
edit("overworld/overworld.h", "  // description\n  cOverworld_description* m_description;\n  // current Layer for collision checking\n  cLayer* m_layer;",
     "  // description\n  cOverworld_description m_description;\n  // current Layer for collision checking\n  cLayer m_layer;")
edit("overworld/overworld.h", "  // HUD world name\n  cHudSprite* m_hud_world_name;\n  // HUD current level name\n  cHudSprite* m_hud_level_name;",
     "  // HUD world name\n  cHudSprite m_hud_world_name;\n  // HUD current level name\n  cHudSprite m_hud_level_name;")
edit("overworld/overworld.cpp",
     "cOverworld ::cOverworld(void) {\n  m_sprite_manager = new cWorld_Sprite_Manager(this);\n  m_animation_manager = new cAnimation_Manager();\n  m_description = new cOverworld_description();\n  m_layer = new cLayer(this);\n\n  m_engine_version = -1;",
     "cOverworld ::cOverworld(void)\n    : m_sprite_manager(this),\n      m_layer(this),\n      m_hud_world_name(&m_sprite_manager),\n      m_hud_level_name(&m_sprite_manager) {\n  m_engine_version = -1;")
edit("overworld/overworld.cpp",
     "  m_hud_world_name = new cHudSprite(m_sprite_manager);\n  m_hud_world_name->Set_Pos(10, static_cast<float>(game_res_h) - 30);\n  m_hud_world_name->Set_Shadow(black, 1.5f);\n  m_hud_level_name = new cHudSprite(m_sprite_manager);\n  m_hud_level_name->Set_Pos(350, 2);\n  m_hud_level_name->Set_Shadow(black, 1.5f);",
     "  m_hud_world_name.Set_Pos(10, static_cast<float>(game_res_h) - 30);\n  m_hud_world_name.Set_Shadow(black, 1.5f);\n  m_hud_level_name.Set_Pos(350, 2);\n  m_hud_level_name.Set_Shadow(black, 1.5f);")
edit("overworld/overworld.cpp",
     "cOverworld ::~cOverworld(void) {\n  Unload();\n\n  delete m_sprite_manager;\n  delete m_animation_manager;\n  delete m_description;\n  delete m_layer;\n  delete m_hud_level_name;\n  delete m_hud_world_name;\n}",
     "cOverworld ::~cOverworld(void) { Unload(); }")

edit("gui/menu.h", "  // The menu images\n  cHudSprite* m_image_default;\n  // The additional Menu Graphic\n  cHudSprite* m_image_menu;",
     "  // The menu images (owned values)\n  cHudSprite m_image_default;\n  // The additional Menu Graphic\n  cHudSprite m_image_menu;")
edit("gui/menu.cpp",
     "    : cHudSprite(sprite_manager) {\n  Set_Scale_Directions(true, true, true, true);\n  m_active = false;\n  m_is_quit = false;\n\n  m_image_default = new cHudSprite(sprite_manager);\n  m_image_menu = new cHudSprite(sprite_manager);\n}\n\ncMenu_Item ::~cMenu_Item(void) {\n  if (m_image_default) {\n    delete m_image_default;\n  }\n\n  if (m_image_menu) {\n    delete m_image_menu;\n  }\n}",
     "    : cHudSprite(sprite_manager),\n      m_image_default(sprite_manager),\n      m_image_menu(sprite_manager) {\n  Set_Scale_Directions(true, true, true, true);\n  m_active = false;\n  m_is_quit = false;\n}\n\ncMenu_Item ::~cMenu_Item(void) = default;")
edit("gui/menu.h", "  // menu camera\n  cCamera* m_camera;\n  // menu level\n  cLevel* m_level;\n  // menu player (currently only used to set the pActive_Player)\n  cSprite* m_player;",
     "  // menu level (owned; declared first: the camera and player are built on its sprite manager)\n  cLevel m_level;\n  // menu camera\n  cCamera m_camera;\n  // menu player (currently only used to set the pActive_Player)\n  cSprite m_player;")
edit("gui/menu.cpp",
     "cMenuHandler ::cMenuHandler(void) {\n  m_level = new cLevel();\n  m_camera = new cCamera(m_level->m_sprite_manager);\n  m_player = new cSprite(m_level->m_sprite_manager);\n  m_player->Set_Massive_Type(MASS_PASSIVE);\n  Reset();",
     "cMenuHandler ::cMenuHandler(void)\n    : m_camera(&m_level.m_sprite_manager), m_player(&m_level.m_sprite_manager) {\n  m_player.Set_Massive_Type(MASS_PASSIVE);\n  Reset();")
edit("gui/menu.cpp", "cMenuHandler ::~cMenuHandler(void) {\n  Reset();\n\n  delete m_camera;\n  delete m_level;\n  delete m_player;\n}",
     "cMenuHandler ::~cMenuHandler(void) { Reset(); }")
edit("gui/menu.h", "  // Menu class\n  cMenu_Base* m_menu_data;\n\n  // Menu handler\n  cMenuHandler* m_handler;\n  // Menu Animation Manager\n  cAnimation_Manager* m_animation_manager;",
     "  // Menu class (owned; polymorphic, replaced on every menu change)\n  std::unique_ptr<cMenu_Base> m_menu_data;\n\n  // Menu handler (owned value)\n  cMenuHandler m_handler;\n  // Menu Animation Manager (owned value)\n  cAnimation_Manager m_animation_manager;")
edit("gui/menu.cpp", "  m_menu_data = nullptr;\n  m_handler = new cMenuHandler();\n\n  // particle animation\n  m_animation_manager = new cAnimation_Manager();\n", "")
edit("gui/menu.cpp", "cMenuCore ::~cMenuCore(void) {\n  Unload();\n\n  delete m_handler;\n  delete m_animation_manager;\n}", "cMenuCore ::~cMenuCore(void) { Unload(); }")
sub("gui/menu.cpp", r"m_menu_data = static_cast<cMenu_Base\*>\(new (cMenu_\w+)\(([^)]*)\)\);", r"m_menu_data = std::make_unique<\1>(\2);", 6)
edit("gui/menu.cpp", "  if (m_menu_data) {\n    delete m_menu_data;\n    m_menu_data = nullptr;", "  if (m_menu_data) {\n    m_menu_data.reset();")

edit("level/level_manager.h", "  // level camera\n  cCamera* m_camera;", "  // level camera (owned value)\n  cCamera m_camera;")
edit("level/level_manager.cpp", "cLevel_Manager ::cLevel_Manager(void) : cObject_Manager<cLevel>() {\n  m_camera = new cCamera(nullptr);\n\n  // set the first camera available\n  if (pActive_Camera == nullptr) {\n    pActive_Camera = m_camera;\n  }",
     "cLevel_Manager ::cLevel_Manager(void)\n    : cObject_Manager<cLevel>(), m_camera(nullptr) {\n  // set the first camera available\n  if (pActive_Camera == nullptr) {\n    pActive_Camera = &m_camera;\n  }")
edit("level/level_manager.cpp", "cLevel_Manager ::~cLevel_Manager(void) {\n  Delete_All();\n  delete m_camera;\n}", "cLevel_Manager ::~cLevel_Manager(void) { Delete_All(); }")
edit("level/level_settings.h", "  // settings camera\n  cCamera* m_camera;", "  // settings camera (owned value)\n  cCamera m_camera;")
edit("level/level_settings.cpp", "cLevel_Settings ::cLevel_Settings(cSprite_Manager* sprite_manager,\n                                  cLevel* level) {\n  m_active = false;\n\n  m_level = level;\n  m_camera = new cCamera(sprite_manager);",
     "cLevel_Settings ::cLevel_Settings(cSprite_Manager* sprite_manager,\n                                  cLevel* level)\n    : m_camera(sprite_manager) {\n  m_active = false;\n\n  m_level = level;")
edit("level/level_settings.cpp", "cLevel_Settings ::~cLevel_Settings(void) {\n  Unload();\n\n  delete m_camera;\n}", "cLevel_Settings ::~cLevel_Settings(void) { Unload(); }")
edit("overworld/world_manager.h", "  // world camera\n  cCamera* m_camera;", "  // world camera (owned value)\n  cCamera m_camera;")
edit("overworld/world_manager.cpp", "cOverworld_Manager ::cOverworld_Manager(cSprite_Manager* sprite_manager)\n    : cObject_Manager<cOverworld>() {\n  m_debug_mode = false;\n  m_draw_layer = false;\n  m_camera_mode = false;\n\n  m_camera = new cCamera(sprite_manager);\n",
     "cOverworld_Manager ::cOverworld_Manager(cSprite_Manager* sprite_manager)\n    : cObject_Manager<cOverworld>(), m_camera(sprite_manager) {\n  m_debug_mode = false;\n  m_draw_layer = false;\n  m_camera_mode = false;\n")
edit("overworld/world_manager.cpp", "cOverworld_Manager ::~cOverworld_Manager(void) {\n  Delete_All();\n\n  delete m_camera;\n}", "cOverworld_Manager ::~cOverworld_Manager(void) { Delete_All(); }")
edit("level/level_editor.h", "  // Level Settings\n  cLevel_Settings* m_settings_screen;", "  // Level Settings (owned value)\n  cLevel_Settings m_settings_screen;")
edit("level/level_editor.cpp", "cEditor_Level ::cEditor_Level(cSprite_Manager* sprite_manager, cLevel* level)\n    : cEditor(sprite_manager) {",
     "cEditor_Level ::cEditor_Level(cSprite_Manager* sprite_manager, cLevel* level)\n    : cEditor(sprite_manager), m_settings_screen(sprite_manager, level) {")
edit("level/level_editor.cpp", "  m_level = level;\n  m_settings_screen = new cLevel_Settings(sprite_manager, m_level);\n}\n\ncEditor_Level ::~cEditor_Level(void) { delete m_settings_screen; }",
     "  m_level = level;\n}\n\ncEditor_Level ::~cEditor_Level(void) = default;")
edit("overworld/world_player.h", "  // Debug text sprites\n  cHudSprite* m_debug_current_waypoint;\n  cHudSprite* m_debug_lines;\n  cHudSprite* m_debug_current_line;",
     "  // Debug text sprites (owned values)\n  cHudSprite m_debug_current_waypoint;\n  cHudSprite m_debug_lines;\n  cHudSprite m_debug_current_line;")
edit("overworld/world_player.cpp",
     "  m_debug_lines = new cHudSprite(m_sprite_manager);\n  m_debug_lines->Set_Pos(150, game_res_h * 0.97f, true);\n  m_debug_current_line = new cHudSprite(m_sprite_manager);\n  m_debug_current_line->Set_Pos(300, game_res_h * 0.97f, true);\n  m_debug_current_waypoint = new cHudSprite(m_sprite_manager);\n  m_debug_current_waypoint->Set_Pos(500, game_res_h * 0.97f, true);",
     "  m_debug_lines.Set_Pos(150, game_res_h * 0.97f, true);\n  m_debug_current_line.Set_Pos(300, game_res_h * 0.97f, true);\n  m_debug_current_waypoint.Set_Pos(500, game_res_h * 0.97f, true);")
edit("overworld/world_player.cpp", "cOverworld_Player ::~cOverworld_Player(void) {\n  delete m_debug_lines;\n  delete m_debug_current_line;\n  delete m_debug_current_waypoint;\n\n  Unload_Images();\n}",
     "cOverworld_Player ::~cOverworld_Player(void) { Unload_Images(); }")
edit("gui/hud.h", "  // stored item\n  cMovingSprite* m_item;", "  // stored item (owned value)\n  cMovingSprite m_item;")
edit("gui/hud.cpp", "  m_item = new cMovingSprite(sprite_manager);\n", "")
edit("gui/hud.cpp", "cItemBox ::~cItemBox(void) { delete m_item; }", "cItemBox ::~cItemBox(void) = default;")
edit("core/framerate.h", "  using Performance_Timer_List = vector<cPerformance_Timer*>;", "  using Performance_Timer_List = vector<cPerformance_Timer>;")
edit("core/framerate.cpp", "  // create performance timers\n  for (unsigned int i = 0; i < 24; i++) {\n    m_perf_timer.push_back(new cPerformance_Timer());\n  }\n}\n\ncFramerate ::~cFramerate(void) {\n  // clear performance timer\n  for (auto& itr : m_perf_timer) {\n    delete itr;\n  }\n\n  m_perf_timer.clear();\n}",
     "  // create performance timers\n  m_perf_timer.resize(24);\n}\n\ncFramerate ::~cFramerate(void) = default;")

# ---------------------------------------------------------------------------------------------
# 2. access-site rules
# ---------------------------------------------------------------------------------------------
# members that are values everywhere they are reached through an owner expression
UNSCOPED = ["m_background_manager", "m_camera", "m_layer", "m_handler",
            "m_hud_world_name", "m_hud_level_name", "m_settings_screen", "m_image_default",
            "m_image_menu", "m_debug_lines", "m_debug_current_line", "m_debug_current_waypoint"]
# members whose name is also a borrowed pointer elsewhere: only through these owner prefixes
OWNER_PREFIX = r"(?:m_level|pActive_Level|level|world|m_overworld|pActive_Overworld|overworld|created|pMenuCore)"
SCOPED = ["m_sprite_manager", "m_animation_manager", "m_description"]
# bare member access inside the owner's own translation unit
BARE = {
    "level/level.cpp": ["m_sprite_manager", "m_animation_manager", "m_background_manager"],
    "overworld/overworld.cpp": ["m_sprite_manager", "m_animation_manager", "m_description", "m_layer", "m_hud_world_name", "m_hud_level_name"],
    "gui/menu.cpp": ["m_camera", "m_level", "m_player", "m_handler", "m_animation_manager", "m_image_default", "m_image_menu"],
    "level/level_manager.cpp": ["m_camera"],
    "level/level_settings.cpp": ["m_camera"],
    "overworld/world_manager.cpp": ["m_camera"],
    "level/level_editor.cpp": ["m_settings_screen"],
    "overworld/world_player.cpp": ["m_debug_lines", "m_debug_current_line", "m_debug_current_waypoint"],
    "gui/hud.cpp": ["m_item"],
}
# `m_level` / `m_player` / `m_item` as values only through these owner prefixes
HANDLER_PREFIX = r"(?:m_handler|pHud_Itembox)"
HANDLER_MEMBERS = ["m_level", "m_player", "m_item"]

ARROW_UNSCOPED = re.compile(r"(->|\.)(" + "|".join(UNSCOPED) + r")\s*->")
ARROW_SCOPED = re.compile(r"(\b" + OWNER_PREFIX + r"(?:->|\.))(" + "|".join(SCOPED) + r")\s*->")
ARROW_HANDLER = re.compile(r"(\b" + HANDLER_PREFIX + r"(?:->|\.))(" + "|".join(HANDLER_MEMBERS) + r")->")
PERF = re.compile(r"(m_perf_timer\[[^\]]+\])->")


def pointer_pass(text: str, members: list[str], prefix: str | None) -> str:
    """`f(expr->M)` / `x = expr->M;` -> `&expr->M` for a value member M reached via an owner."""
    chain = r"(?:[A-Za-z_]\w*(?:->|\.))+" if prefix is None else r"(?:[A-Za-z_]\w*(?:->|\.))*\b" + prefix + r"(?:->|\.)"
    pat = re.compile(r"([(,=]\s*)(" + chain + r"(?:" + "|".join(members) + r"))(\s*[,);])")
    return pat.sub(lambda m: m.group(1) + ("" if m.group(2).startswith("&") else "&") + m.group(2) + m.group(3), text)


def pointer_pass_bare(text: str, members: list[str]) -> str:
    pat = re.compile(r"([(,=]\s*)(" + "|".join(members) + r")(\s*[,);])")
    return pat.sub(r"\1&\2\3", text)


for path in sorted(SRC.rglob("*.cpp")) + sorted(SRC.rglob("*.h")):
    rel = str(path.relative_to(SRC))
    text = path.read_text()
    original = text
    # corrections: `m_description` strings in cLevel/cSave and a sprite's borrowed pointer via `parent`
    text = re.sub(r"&((?:savegame->|m_level->)?m_description)\b", r"\1", text)
    text = text.replace("&parent->m_sprite_manager", "parent->m_sprite_manager")
    text = ARROW_UNSCOPED.sub(r"\1\2.", text)
    text = ARROW_SCOPED.sub(r"\1\2.", text)
    text = ARROW_HANDLER.sub(r"\1\2.", text)
    text = PERF.sub(r"\1.", text)
    text = pointer_pass(text, UNSCOPED, None)
    text = pointer_pass(text, SCOPED, OWNER_PREFIX)
    text = pointer_pass(text, HANDLER_MEMBERS, HANDLER_PREFIX)
    if rel in BARE:
        members = BARE[rel]
        text = re.sub(r"(?<![\w.>])(" + "|".join(members) + r")->", r"\1.", text)
        text = pointer_pass_bare(text, members)
    if text != original:
        path.write_text(text)
edit("core/framerate.cpp", "  for (auto& itr : m_perf_timer) {\n    itr->Reset();\n  }",
     "  for (auto& timer : m_perf_timer) {\n    timer.Reset();\n  }")
sub("core/game_core.cpp", r"static_cast<cMenu_Start\*>\(pMenuCore->m_menu_data\)", "static_cast<cMenu_Start*>(pMenuCore->m_menu_data.get())", 1)
sub("gui/menu.cpp", r"static_cast<cMenu_Start\*>\(pMenuCore->m_menu_data\)", "static_cast<cMenu_Start*>(pMenuCore->m_menu_data.get())", 1)
edit("gui/hud.cpp", "cItemBox ::cItemBox(cSprite_Manager* sprite_manager)\n    : cStatusText(sprite_manager) {",
     "cItemBox ::cItemBox(cSprite_Manager* sprite_manager)\n    : cStatusText(sprite_manager), m_item(sprite_manager) {")
edit("overworld/world_player.cpp",
     "cOverworld_Player ::cOverworld_Player(cSprite_Manager* sprite_manager,\n                                      cOverworld* overworld)\n    : cAnimated_Sprite(sprite_manager) {",
     "cOverworld_Player ::cOverworld_Player(cSprite_Manager* sprite_manager,\n                                      cOverworld* overworld)\n    : cAnimated_Sprite(sprite_manager),\n      m_debug_current_waypoint(sprite_manager),\n      m_debug_lines(sprite_manager),\n      m_debug_current_line(sprite_manager) {")
print("wave 6 applied")
