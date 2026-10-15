#!/usr/bin/env python3
"""Phase D, wave 8: every raw-owning vector becomes `vector<std::unique_ptr<T>>` (rule of zero).

  cHud_Manager/cDebugDisplay HudSpriteList, cPlayerPoints PointsTextList, cMenu_Base HudSpriteList
  (m_draw_list, m_savegame_temp), the animation BlinkPointList/FireAnimList/ParticleList,
  Saved_Texture_List (fonts + image manager), AudioSoundList, cBonusBox MovingSpriteList, the
  mouse SelectedObjectList/CopyObjectList, cMenuHandler MenuList, cSprite's editor settings items,
  and cEditor_Item_Object::list_text. The delete loops become `clear()`, erase-with-delete sites
  just erase, by-value loops become `const auto&`, and the factories that fed these lists
  (Auto_Menu, Get_Software_Texture) return unique_ptr. Where the code keeps using the object after
  pushing it, it borrows the pointer first (Add_Menu_Item returns it).

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


def sub(rel: str, pattern: str, repl: str, count: int | None, flags: int = 0) -> None:
    path = SRC / rel
    text = path.read_text()
    new_text, n = re.subn(pattern, repl, text, flags=flags)
    if n == 0:
        return
    assert count is None or n == count, f"{rel}: expected {count} of {pattern!r}, got {n}"
    path.write_text(new_text)


# --- 1. the vector aliases -------------------------------------------------------------------
for rel, name, n in [("gui/hud.h", "HudSpriteList", 2), ("gui/hud.h", "PointsTextList", 1),
                     ("gui/menu_data.h", "HudSpriteList", 1), ("video/animation.h", "BlinkPointList", 1),
                     ("video/animation.h", "FireAnimList", 1), ("video/animation.h", "ParticleList", 1),
                     ("input/mouse.h", "SelectedObjectList", 1), ("input/mouse.h", "CopyObjectList", 1),
                     ("video/img_manager.h", "Saved_Texture_List", 1), ("gui/menu.h", "MenuList", 1),
                     ("audio/audio.h", "AudioSoundList", 1), ("objects/bonusbox.h", "MovingSpriteList", 1),
                     ("objects/sprite.h", "Editor_Object_Settings_List", 1)]:
    sub(rel, r"using " + name + r" = vector<(\w+)\*>;", r"using " + name + r" = vector<std::unique_ptr<\1>>;", n)

VECTORS = r"(?:m_sprites|m_points_objects|m_draw_list|m_savegame_temp|m_objects|m_software_textures|m_saved_textures|m_active_sounds|m_active_items|m_copy_objects|m_selected_objects|m_items|m_editor_windows)"

# --- 2. delete loops -> clear(); by-value loops -> const auto& --------------------------------
DELETE_LOOP = re.compile(
    r"[ \t]*for \((?:const )?auto&? (\w+) : (" + VECTORS + r")\) \{\n(?:[ \t]*//[^\n]*\n)*[ \t]*delete \1;\n[ \t]*\}\n\n?([ \t]*)\2\.clear\(\);")
BY_VALUE = re.compile(r"for \(auto (\w+) : ((?:pAudio->|pMouseCursor->)?" + VECTORS + r")\)")
for path in sorted(SRC.rglob("*.cpp")):
    text = path.read_text()
    new = DELETE_LOOP.sub(lambda m: f"{m.group(3)}{m.group(2)}.clear();", text)
    new = BY_VALUE.sub(r"for (const auto& \1 : \2)", new)
    if new != text:
        path.write_text(new)

# --- 3. erase-with-delete sites ----------------------------------------------------------------
edit("gui/hud.cpp", "    PointsText* obj = (*itr);", "    PointsText* obj = itr->get();")
edit("gui/hud.cpp", "      itr = m_points_objects.erase(itr);\n      delete obj;", "      itr = m_points_objects.erase(itr);")
edit("video/animation.cpp", "    cParticle* obj = (*itr);", "    cParticle* obj = itr->get();")
edit("video/animation.cpp", "      itr = m_objects.erase(itr);\n      delete obj;", "      itr = m_objects.erase(itr);")
edit("input/mouse.cpp", "    cCopyObject* copy_obj = (*itr);", "    cCopyObject* copy_obj = itr->get();")
edit("input/mouse.cpp", "      m_copy_objects.erase(itr);\n      delete copy_obj;", "      m_copy_objects.erase(itr);")
edit("input/mouse.cpp", "    cSelectedObject* sel_obj = (*itr);", "    cSelectedObject* sel_obj = itr->get();")
edit("input/mouse.cpp", "      m_selected_objects.erase(itr);\n      delete sel_obj;", "      m_selected_objects.erase(itr);")
edit("audio/audio.cpp", "    // delete data\n    delete *(last_itr);\n", "")
edit("objects/bonusbox.cpp", "    cPowerUp* powerup = static_cast<cPowerUp*>(*itr);", "    cPowerUp* powerup = static_cast<cPowerUp*>(itr->get());")
edit("objects/bonusbox.cpp",
     "      // m_active_items owned it (a raw-owning list until the member phase); it is erased\n"
     "      // from that list right below, so the sprite manager takes it over here\n"
     "      m_sprite_manager->Add(std::unique_ptr<cSprite>(powerup));",
     "      // hand the item over to the sprite manager (it is erased from this list below)\n"
     "      m_sprite_manager->Add(std::move(*itr));")

# --- 4. the menu item factory and handler -----------------------------------------------------
edit("gui/menu.h", "  void Add_Menu_Item(cMenu_Item* item, float shadow_pos = 0,",
     "  // Takes ownership of the item; returns the borrowed pointer\n  cMenu_Item* Add_Menu_Item(std::unique_ptr<cMenu_Item> item, float shadow_pos = 0,")
sub("gui/menu.h", r"  cMenu_Item\* Auto_Menu\(", "  std::unique_ptr<cMenu_Item> Auto_Menu(", 1)
edit("gui/menu.cpp", "void cMenuHandler ::Add_Menu_Item(\n    cMenu_Item* item, float shadow_pos /* = 0 */,",
     "cMenu_Item* cMenuHandler ::Add_Menu_Item(\n    std::unique_ptr<cMenu_Item> item, float shadow_pos /* = 0 */,")
edit("gui/menu.cpp", "    printf(\"Menu item is NULL ( current Menu size : %d )\\n\", Get_Size());\n    return;",
     "    printf(\"Menu item is NULL ( current Menu size : %d )\\n\", Get_Size());\n    return nullptr;")
edit("gui/menu.cpp", "  m_items.push_back(item);\n", "  cMenu_Item* added = item.get();\n  m_items.push_back(std::move(item));\n")
sub("gui/menu.cpp", r"(  m_items\.push_back\(std::move\(item\)\);\n(?:.*\n)*?)\}\n\nvoid cMenuHandler ::Reset", r"\1\n  return added;\n}\n\nvoid cMenuHandler ::Reset", 1)
sub("gui/menu.cpp", r"^cMenu_Item\* cMenuCore ::Auto_Menu\(", "std::unique_ptr<cMenu_Item> cMenuCore ::Auto_Menu(", 1, re.M)
edit("gui/menu.cpp", "  cMenu_Item* temp_item = new cMenu_Item(&m_handler.m_level.m_sprite_manager);",
     "  auto temp_item = std::make_unique<cMenu_Item>(&m_handler.m_level.m_sprite_manager);")
edit("gui/menu.cpp", "  return m_items[m_active];", "  return m_items[m_active].get();")
sub("gui/menu.cpp", r"cMenu_Start\* menu_start = new cMenu_Start\(\);", "auto menu_start = std::make_unique<cMenu_Start>();", 1)
sub("gui/menu.cpp", r"[ \t]*delete menu_start;\n", "", 1)
# the one post-push use: borrow the pointer Add_Menu_Item returns
edit("gui/menu_data.cpp",
     "    pMenuCore->m_handler.Add_Menu_Item(temp_item, 1.5f, grey);\n\n    m_menu_pos_y += temp_item->m_image_default.m_col_rect.m_h;",
     "    cMenu_Item* added =\n        pMenuCore->m_handler.Add_Menu_Item(std::move(temp_item), 1.5f, grey);\n\n    m_menu_pos_y += added->m_image_default.m_col_rect.m_h;")
sub("gui/menu_data.cpp", r"cMenu_Item\* temp_item = nullptr;", "std::unique_ptr<cMenu_Item> temp_item;", None)
sub("gui/menu_data.cpp", r"temp_item = new cMenu_Item\(", "temp_item = std::make_unique<cMenu_Item>(", None)
sub("gui/menu_data.cpp", r"Add_Menu_Item\(temp_item([,)])", r"Add_Menu_Item(std::move(temp_item)\1", None)

# --- 5. the post-push borrow in the mouse selection -------------------------------------------
edit("input/mouse.cpp",
     "  cSelectedObject* selected_object = new cSelectedObject();\n  selected_object->m_obj = sprite;\n  selected_object->m_user = from_user;\n  m_selected_objects.push_back(selected_object);\n\n  Update_Selected_Object_Offset(selected_object);",
     "  auto owned = std::make_unique<cSelectedObject>();\n  cSelectedObject* selected_object = owned.get();\n  selected_object->m_obj = sprite;\n  selected_object->m_user = from_user;\n  m_selected_objects.push_back(std::move(owned));\n\n  Update_Selected_Object_Offset(selected_object);")

# --- 6. the bonus box item ------------------------------------------------------------------------
edit("objects/bonusbox.cpp", "  cMovingSprite* box_item = nullptr;", "  std::unique_ptr<cMovingSprite> box_item;")
sub("objects/bonusbox.cpp", r"cMushroom\* mushroom = new cMushroom\(m_sprite_manager\);", "auto mushroom = std::make_unique<cMushroom>(m_sprite_manager);", 2)
sub("objects/bonusbox.cpp", r"box_item = static_cast<cMovingSprite\*>\(mushroom\);", "box_item = std::move(mushroom);", 2)
edit("objects/bonusbox.cpp", "box_item = static_cast<cMovingSprite*>(new cFirePlant(m_sprite_manager));", "box_item = std::make_unique<cFirePlant>(m_sprite_manager);")
edit("objects/bonusbox.cpp", "    m_active_items.push_back(box_item);", "    m_active_items.push_back(std::move(box_item));")

# --- 7. the audio pool, the texture snapshots, the editor list text ----------------------------
edit("audio/audio.cpp",
     "    // m_active_sounds owns its entries (freed in its clear loops) -- a raw list for now\n    cAudio_Sound* sound = new cAudio_Sound();\n    m_active_sounds.push_back(sound);\n    return sound;",
     "    auto owned = std::make_unique<cAudio_Sound>();\n    cAudio_Sound* sound = owned.get();\n    m_active_sounds.push_back(std::move(owned));\n    return sound;")
sub("video/gl_surface.h", r"  cSaved_Texture\* Get_Software_Texture\(", "  std::unique_ptr<cSaved_Texture> Get_Software_Texture(", 1)
sub("video/gl_surface.cpp", r"^cSaved_Texture\* cGL_Surface ::Get_Software_Texture\(", "std::unique_ptr<cSaved_Texture> cGL_Surface ::Get_Software_Texture(", 1, re.M)
edit("video/gl_surface.cpp", "  cSaved_Texture* soft_tex = new cSaved_Texture();", "  auto soft_tex = std::make_unique<cSaved_Texture>();")
for rel in ["video/font.cpp", "video/img_manager.cpp"]:
    sub(rel, r"Load_Software_Texture\(soft_tex\);", "Load_Software_Texture(soft_tex.get());", 1)
    sub(rel, r"[ \t]*delete soft_tex;\n", "", None)
edit("core/editor.h", "  CEGUI::ListboxTextItem* list_text;", "  std::unique_ptr<CEGUI::ListboxTextItem> list_text;")
edit("core/editor.cpp", "  list_text = new CEGUI::ListboxTextItem(", "  list_text = std::make_unique<CEGUI::ListboxTextItem>(")
sub("core/editor.cpp", r"[ \t]*delete list_text;\n", "", 1)
print("wave 8 applied")
