#!/usr/bin/env python3
"""Phase E (RAII teardown) + the phase-F loop preludes.

Phase E: the application object's 24 singleton slots become `cOwned<T>` members -- an owning
pointer bound to the `extern cX* pX` alias every call site uses, which the slot clears when it
dies. The members are declared in the REVERSE of the order the original hand-written Exit_Game
destroyed them in, so the implicit destructor reproduces that order exactly; the CEGUI system,
which Exit_Game destroyed between the framerate and the video subsystem, now dies in ~cVideo.
Exit_Game shrinks to: save the preferences, unload, `g_app.reset()`, quit SDL.

Phase F preludes: `for (const auto& x_ptr : v) { T* x = x_ptr.get(); …` loops use the handle.

Asserting and idempotent. Paths are repo-relative.
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


def sub(rel: str, pattern: str, repl, count: int | None, flags: int = 0) -> None:
    # repl may be a plain string (re escape rules apply) or a callable (used verbatim)
    path = SRC / rel
    text = path.read_text()
    new_text, n = re.subn(pattern, repl, text, flags=flags)
    if n == 0:
        return
    assert count is None or n == count, f"{rel}: expected {count} of {pattern!r}, got {n}"
    path.write_text(new_text)


# --- loop preludes -----------------------------------------------------------------------------
edit("core/sprite_manager.cpp",
     "  for (const auto& sprite_ptr : objects) {\n    cSprite* sprite = sprite_ptr.get();\n    if (static_cast<int>(sprite->m_start_pos_x) != start_pos_x ||",
     "  for (const auto& sprite : objects) {\n    if (static_cast<int>(sprite->m_start_pos_x) != start_pos_x ||")
edit("core/sprite_manager.cpp", "    // found\n    return sprite;", "    // found\n    return sprite.get();")
edit("core/sprite_manager.cpp",
     "  for (const auto& sprite_ptr : objects) {\n    cSprite* sprite = sprite_ptr.get();\n    // if destroyed sprite\n    if (sprite == exclude_sprite || sprite->m_auto_destroy) {",
     "  for (const auto& sprite : objects) {\n    // if destroyed sprite\n    if (sprite.get() == exclude_sprite || sprite->m_auto_destroy) {")
edit("core/sprite_manager.cpp", "    col_objects.push_back(sprite);\n  }\n\n  if (with_player && pActive_Player != exclude_sprite) {",
     "    col_objects.push_back(sprite.get());\n  }\n\n  if (with_player && pActive_Player != exclude_sprite) {")
edit("core/sprite_manager.cpp",
     "  for (const auto& sprite_ptr : objects) {\n    cSprite* sprite = sprite_ptr.get();\n    if (sprite->m_auto_destroy) {\n      if (sprite->m_collisions.size()) {",
     "  for (const auto& sprite : objects) {\n    if (sprite->m_auto_destroy) {\n      if (sprite->m_collisions.size()) {")
edit("user/savegame.cpp",
     "      for (const auto& save_waypoint_ptr : save_overworld->m_waypoints) {\n        // get savegame waypoint pointer\n        cSave_Overworld_Waypoint* save_waypoint = save_waypoint_ptr.get();\n",
     "      for (const auto& save_waypoint : save_overworld->m_waypoints) {\n")
edit("user/savegame.cpp",
     "    for (const auto& save_level_ptr : savegame->m_levels) {\n      cSave_Level* save_level = save_level_ptr.get();\n",
     "    for (const auto& save_level : savegame->m_levels) {\n")
edit("user/savegame.cpp",
     "    for (const auto& sprite : overworld->m_sprite_manager.objects) {\n      // get waypoint\n      cSprite* obj = sprite.get();\n\n      if (obj->m_type != TYPE_OW_WAYPOINT) {\n        continue;\n      }\n\n      // get waypoint\n      cWaypoint* waypoint = static_cast<cWaypoint*>(obj);",
     "    for (const auto& sprite : overworld->m_sprite_manager.objects) {\n      if (sprite->m_type != TYPE_OW_WAYPOINT) {\n        continue;\n      }\n\n      // get waypoint\n      cWaypoint* waypoint = static_cast<cWaypoint*>(sprite.get());")
sub("level/level_player.cpp",
    r"  for \(const auto& collision_ptr : col_list->objects\) \{\n    cObjectCollision\* collision = collision_ptr\.get\(\);\n",
    "  for (const auto& collision : col_list->objects) {\n", 2)

# --- phase E: the application object ---------------------------------------------------------
OLD_CLASS = """// Owns every process-global singleton for the whole run, so each is freed exactly once and in one
// place (fixing the old hand-ordered Exit_Game -- including the cFramerate that used to leak). The
// extern cX* pX globals stay as borrowed aliases into these members, set in Init_Game and cleared
// in Exit_Game; the teardown order in Exit_Game is preserved from the original for now.
class cApplication {
 public:
  std::unique_ptr<cResource_Manager> m_resource_manager;
  std::unique_ptr<cVideo> m_video;
  std::unique_ptr<cAudio> m_audio;
  std::unique_ptr<cFont_Manager> m_font;
  std::unique_ptr<cFramerate> m_framerate;
  std::unique_ptr<cRenderQueue> m_renderer;
  std::unique_ptr<cRenderQueue> m_renderer_current;
  std::unique_ptr<cPreferences> m_preferences;
  std::unique_ptr<cImage_Manager> m_image_manager;
  std::unique_ptr<cSound_Manager> m_sound_manager;
  std::unique_ptr<cImage_Settings_Parser> m_settings_parser;
  std::unique_ptr<cCampaign_Manager> m_campaign_manager;
  std::unique_ptr<cLevel_Player> m_level_player;
  std::unique_ptr<cLevel_Manager> m_level_manager;
  std::unique_ptr<cHud_Manager> m_hud_manager;
  std::unique_ptr<cEditor_Level> m_level_editor;
  std::unique_ptr<cEditor_World> m_world_editor;
  std::unique_ptr<cMouseCursor> m_mouse_cursor;
  std::unique_ptr<cKeyboard> m_keyboard;
  std::unique_ptr<cJoystick> m_joystick;
  std::unique_ptr<cOverworld_Player> m_overworld_player;
  std::unique_ptr<cOverworld_Manager> m_overworld_manager;
  std::unique_ptr<cMenuCore> m_menu_core;
  std::unique_ptr<cSavegame> m_savegame;
};
"""
NEW_CLASS = """// A singleton slot: the owning pointer plus the `extern cX* pX` alias every call site uses. The
// alias is set when the object is created and cleared when it dies, so no alias ever dangles.
template <class T>
class cOwned {
 public:
  explicit cOwned(T*& alias) : m_alias(alias) {}
  ~cOwned(void) {
    m_ptr.reset();
    m_alias = nullptr;
  }
  cOwned(const cOwned&) = delete;
  cOwned& operator=(const cOwned&) = delete;

  // Create the object (forwarding the constructor arguments) and point the alias at it
  template <class... Args>
  void Create(Args&&... args) {
    m_ptr = std::make_unique<T>(std::forward<Args>(args)...);
    m_alias = m_ptr.get();
  }

 private:
  std::unique_ptr<T> m_ptr;
  T*& m_alias;
};

// Owns every process-global singleton for the whole run. Destruction is the implicit destructor's
// job: members die in reverse declaration order, so they are declared in the REVERSE of the
// teardown order the original hand-written Exit_Game used (audio first ... resource manager last).
// The CEGUI system, which that order destroyed between the framerate and the video subsystem,
// dies in ~cVideo.
class cApplication {
 public:
  cOwned<cResource_Manager> m_resource_manager{pResource_Manager};
  cOwned<cFont_Manager> m_font{pFont};
  cOwned<cImage_Settings_Parser> m_settings_parser{pSettingsParser};
  cOwned<cImage_Manager> m_image_manager{pImage_Manager};
  cOwned<cVideo> m_video{pVideo};
  cOwned<cFramerate> m_framerate{pFramerate};
  cOwned<cRenderQueue> m_renderer_current{pRenderer_current};
  cOwned<cRenderQueue> m_renderer{pRenderer};
  cOwned<cMenuCore> m_menu_core{pMenuCore};
  cOwned<cLevel_Manager> m_level_manager{pLevel_Manager};
  cOwned<cOverworld_Player> m_overworld_player{pOverworld_Player};
  cOwned<cOverworld_Manager> m_overworld_manager{pOverworld_Manager};
  cOwned<cCampaign_Manager> m_campaign_manager{pCampaign_Manager};
  cOwned<cKeyboard> m_keyboard{pKeyboard};
  cOwned<cJoystick> m_joystick{pJoystick};
  cOwned<cMouseCursor> m_mouse_cursor{pMouseCursor};
  cOwned<cSavegame> m_savegame{pSavegame};
  cOwned<cPreferences> m_preferences{pPreferences};
  cOwned<cEditor_World> m_world_editor{pWorld_Editor};
  cOwned<cEditor_Level> m_level_editor{pLevel_Editor};
  cOwned<cSound_Manager> m_sound_manager{pSound_Manager};
  cOwned<cHud_Manager> m_hud_manager{pHud_Manager};
  cOwned<cLevel_Player> m_level_player{pLevel_Player};
  cOwned<cAudio> m_audio{pAudio};
};
"""
edit("core/main.cpp", OLD_CLASS, NEW_CLASS)
sub("core/main.cpp",
    r"g_app->(m_\w+)\s*=\s*std::make_unique<c\w+>\(((?:[^;]|\n)*?)\);\n\s*p\w+ = g_app->\1\.get\(\);",
    r"g_app->\1.Create(\2);", 24)
sub("core/main.cpp",
    r"  g_app->m_audio\.reset\(\);\n  pAudio = nullptr;\n(?:.*\n)*?  // free the \(now empty\) application object\n  g_app\.reset\(\);\n",
    lambda m: "  // every singleton dies here, in cApplication's declaration order (the CEGUI system inside\n"
    "  // ~cVideo); each alias is cleared as its object goes\n"
    "  g_app.reset();\n\n"
    "  const char* last_sdl_error = SDL_GetError();\n"
    "  if (strlen(last_sdl_error) > 0) {\n"
    "    printf(\"Last known SDL Error : %s\\n\", last_sdl_error);\n"
    "  }\n", 1)
edit("video/video.cpp", "cVideo ::~cVideo(void) = default;",
     "cVideo ::~cVideo(void) {\n"
     "  // the CEGUI system, its renderer, resource provider and logger were created by Init_CEGUI\n"
     "  // and die with the video subsystem, after every object that draws into CEGUI is gone\n"
     "  if (pGuiSystem) {\n"
     "    CEGUI::ResourceProvider* rp = pGuiSystem->getResourceProvider();\n"
     "    CEGUI::Logger* logger = CEGUI::Logger::getSingletonPtr();\n"
     "    pGuiSystem->destroy();\n"
     "    pGuiSystem = nullptr;\n"
     "    delete rp;\n"
     "    delete logger;\n"
     "  }\n\n"
     "  if (pGuiRenderer) {\n"
     "    pGuiRenderer->destroy(*pGuiRenderer);\n"
     "    pGuiRenderer = nullptr;\n"
     "  }\n"
     "}")
print("wave 12 applied")
