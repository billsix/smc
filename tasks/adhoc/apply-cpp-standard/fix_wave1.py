#!/usr/bin/env python3
"""Phase A, compiler wave 1: the hand fixes the codemod could not make, applied as one script so
they are reviewable and re-runnable (each edit asserts its anchor; a second run finds nothing).

Shapes handled:
  * Save_To_Savegame() factories return std::unique_ptr<cSave_Level_Object> (12 signatures + 2 callers)
  * Create_Texture() / the mouse-collision getters return unique_ptr (their results were already
    unique_ptr locals after the codemod; the callers drop their `delete`)
  * the "create the emitter if none was passed" shape (three sites) gets an owned_anim unique_ptr
  * multi-line Draw_*(…, request) calls get `.get()` (the codemod only handled single-line args)
  * cCopyObject::m_obj comparisons/uses get `.get()`; the moving platform's draw requests move
  * reverts (left raw on purpose, converted by a later phase): cAudio::m_active_sounds entries,
    cMenu_Item for cMenuHandler::m_items, cGL_Surface::Copy / Get_Software_Texture (raw-owning
    lists), Col_Move_in_Steps' collision list (phase C), the sprite manager's Get_First/Get_Last
    search results (borrowed pointers that the codemod wrongly made owning).
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
        return  # already applied
    assert found == count, f"{rel}: expected {count} of {old[:60]!r}, found {found}"
    path.write_text(text.replace(old, new))


def sub(rel: str, pattern: str, repl: str, count: int, flags: int = 0) -> None:
    path = SRC / rel
    text = path.read_text()
    new_text, n = re.subn(pattern, repl, text, flags=flags)
    if n == 0:
        return  # already applied
    assert n == count, f"{rel}: expected {count} matches of {pattern!r}, got {n}"
    path.write_text(new_text)


# --- Save_To_Savegame returns unique_ptr -------------------------------------------------------
for rel in ["enemies/turtle.h", "enemies/thromp.h", "enemies/flyon.h", "enemies/static.h",
            "enemies/enemy.h", "objects/powerup.h", "objects/box.h", "objects/goldpiece.h",
            "objects/path.h", "objects/moving_platform.h", "objects/ball.h"]:
    sub(rel, r"cSave_Level_Object\* Save_To_Savegame\(void\) override;",
        "std::unique_ptr<cSave_Level_Object> Save_To_Savegame(void) override;", 1)
edit("objects/sprite.h",
     "virtual cSave_Level_Object* Save_To_Savegame(void) { return nullptr; };",
     "virtual std::unique_ptr<cSave_Level_Object> Save_To_Savegame(void) {\n    return nullptr;\n  };")
for rel in ["enemies/turtle.cpp", "enemies/thromp.cpp", "enemies/flyon.cpp", "enemies/static.cpp",
            "enemies/enemy.cpp", "objects/powerup.cpp", "objects/box.cpp", "objects/goldpiece.cpp",
            "objects/path.cpp", "objects/moving_platform.cpp", "objects/ball.cpp"]:
    sub(rel, r"^cSave_Level_Object\* (c\w+) ::Save_To_Savegame\(void\) \{",
        r"std::unique_ptr<cSave_Level_Object> \1 ::Save_To_Savegame(void) {", 1, re.M)
# overrides that forward to the base and decorate the result
for rel in ["enemies/static.cpp", "enemies/turtle.cpp", "enemies/thromp.cpp", "enemies/flyon.cpp"]:
    sub(rel, r"cSave_Level_Object\* save_object = (c\w+)::Save_To_Savegame\(\);",
        r"std::unique_ptr<cSave_Level_Object> save_object = \1::Save_To_Savegame();", 1)
    sub(rel, r"m_path_state\.Save_To_Savegame\(save_object\);",
        "m_path_state.Save_To_Savegame(save_object.get());", 1)
edit("user/savegame.cpp",
     """        cSave_Level_Object* save_obj = sprite->Save_To_Savegame();

        // nothing to save
        if (!save_obj) {
          continue;
        }

        // add (adopt the raw save-object)
        save_level->m_level_objects.emplace_back(save_obj);""",
     """        std::unique_ptr<cSave_Level_Object> save_obj =
            sprite->Save_To_Savegame();

        // nothing to save
        if (!save_obj) {
          continue;
        }

        save_level->m_level_objects.push_back(std::move(save_obj));""")

# --- Create_Texture returns unique_ptr ---------------------------------------------------------
sub("video/video.h", r"cGL_Surface\* Create_Texture\(", "std::unique_ptr<cGL_Surface> Create_Texture(", 1)
sub("video/video.cpp", r"^cGL_Surface\* cVideo ::Create_Texture\(", "std::unique_ptr<cGL_Surface> cVideo ::Create_Texture(", 1, re.M)

# --- mouse collision getters return unique_ptr; callers drop delete ----------------------------
sub("input/mouse.h", r"cObjectCollision\* Get_First_(Editor_Collsion|Mouse_Collision)\(",
    r"std::unique_ptr<cObjectCollision> Get_First_\1(", 2)
sub("input/mouse.cpp", r"^cObjectCollision\* cMouseCursor ::Get_First_(Editor_Collsion|Mouse_Collision)\(",
    r"std::unique_ptr<cObjectCollision> cMouseCursor ::Get_First_\1(", 2, re.M)
for rel, n in [("input/mouse.cpp", 2), ("core/editor.cpp", 1)]:
    sub(rel, r"cObjectCollision\* col = (pMouseCursor->)?Get_First_Editor_Collsion\(",
        r"std::unique_ptr<cObjectCollision> col = \1Get_First_Editor_Collsion(", n)
    sub(rel, r"^[ \t]*delete col;\n", "", n, re.M)

# --- cCopyObject::m_obj is a unique_ptr now ----------------------------------------------------
edit("input/mouse.cpp",
     "cCopyObject ::~cCopyObject(void) {\n  if (m_obj) {\n    delete m_obj;\n  }\n}",
     "cCopyObject ::~cCopyObject(void) = default;")
sub("input/mouse.cpp", r"(copy_obj(?:ect)?->m_obj) == sprite\)", r"\1.get() == sprite)", 2)
edit("input/mouse.cpp", "Copy(copy_obj->m_obj, px + base_vec.x, py + base_vec.y);",
     "Copy(copy_obj->m_obj.get(), px + base_vec.x, py + base_vec.y);")

# --- create-the-emitter-if-none-passed (enemy hit animation and two siblings) ------------------
CREATE_ANIM = re.compile(
    r"(?P<ind>[ \t]*)bool create_anim = false;\n\n"
    r"(?P=ind)if \(!anim\) \{\n"
    r"(?P=ind)  create_anim = true;\n"
    r"(?P=ind)  // create animation\n"
    r"(?P=ind)  anim = new cParticle_Emitter\(m_sprite_manager\);\n"
    r"(?P=ind)\}\n")
CREATE_ANIM_ADD = re.compile(
    r"(?P<ind>[ \t]*)if \(create_anim\) \{\n"
    r"(?P=ind)  anim->Emit\(\);\n"
    r"(?P=ind)  pActive_Animation_Manager->Add\(anim\);\n"
    r"(?P=ind)\}\n")
for rel in ["enemies/enemy.cpp", "level/level_player.cpp", "objects/ball.cpp"]:
    path = SRC / rel
    text = path.read_text()
    text, n1 = CREATE_ANIM.subn(
        lambda m: (f"{m.group('ind')}// no emitter given: make one here and hand it to the animation manager at the end\n"
                   f"{m.group('ind')}std::unique_ptr<cParticle_Emitter> owned_anim;\n"
                   f"{m.group('ind')}if (!anim) {{\n"
                   f"{m.group('ind')}  owned_anim = std::make_unique<cParticle_Emitter>(m_sprite_manager);\n"
                   f"{m.group('ind')}  anim = owned_anim.get();\n"
                   f"{m.group('ind')}}}\n"), text)
    text, n2 = CREATE_ANIM_ADD.subn(
        lambda m: (f"{m.group('ind')}if (owned_anim) {{\n"
                   f"{m.group('ind')}  anim->Emit();\n"
                   f"{m.group('ind')}  pActive_Animation_Manager->Add(std::move(owned_anim));\n"
                   f"{m.group('ind')}}}\n"), text)
    assert n1 == n2, f"{rel}: create_anim {n1} vs add {n2}"
    path.write_text(text)

# --- multi-line Draw_*(…, request) calls: the argument sits on a continuation line ----------
# (only these files; every `pVideo->Draw_Rect/Gradient(…, <x>request)` there passes a unique_ptr
# local, so the borrowed pointer is what the draw function wants)
DRAW_CALL = re.compile(r"(pVideo->Draw_(?:Rect|Gradient)\([^;]*?[,(]\s*)(\w*request)\);", re.S)
for rel in ["gui/menu_data.cpp", "input/mouse.cpp", "objects/sprite.cpp", "video/animation.cpp", "video/video.cpp"]:
    path = SRC / rel
    text = path.read_text()
    text = DRAW_CALL.sub(lambda m: f"{m.group(1)}{m.group(2)}.get());", text)
    path.write_text(text)

# --- moving platform: the reused surface_request local becomes owning -------------------------
sub("objects/moving_platform.cpp", r"cSurface_Request\* surface_request(\s*=\s*nullptr)?;",
    "std::unique_ptr<cSurface_Request> surface_request;", 1)
sub("objects/moving_platform.cpp", r"surface_request = new cSurface_Request\(\);",
    "surface_request = std::make_unique<cSurface_Request>();", 3)
sub("objects/moving_platform.cpp", r"cAnimated_Sprite::Draw(_Image)?\(surface_request\);",
    r"cAnimated_Sprite::Draw\1(surface_request.get());", 3)
sub("objects/moving_platform.cpp", r"pRenderer->Add\(surface_request\);",
    "pRenderer->Add(std::move(surface_request));", 3)

# --- bonusbox: hand-off from the raw-owning m_active_items list (converted in a later phase) --
edit("objects/bonusbox.cpp", "      m_sprite_manager->Add(powerup);",
     "      // m_active_items owned it (a raw-owning list until the member phase); it is erased\n"
     "      // from that list right below, so the sprite manager takes it over here\n"
     "      m_sprite_manager->Add(std::unique_ptr<cSprite>(powerup));")

# --- reverts: borrowed search results the codemod made owning ----------------------------------
edit("core/sprite_manager.cpp",
     """  std::unique_ptr<cSprite> first;

  for (const auto& sprite_ptr : objects) {
    cSprite* sprite = sprite_ptr.get();
    if (sprite->m_type == type && (!first || sprite->m_pos_z < first->m_pos_z)) {
      first = sprite;
    }
  }""",
     """  cSprite* first = nullptr;

  for (const auto& sprite : objects) {
    if (sprite->m_type == type && (!first || sprite->m_pos_z < first->m_pos_z)) {
      first = sprite.get();
    }
  }""")
edit("core/sprite_manager.cpp",
     """  std::unique_ptr<cSprite> last;

  for (const auto& sprite_ptr : objects) {
    cSprite* sprite = sprite_ptr.get();
    if (sprite->m_type == type && (!last || sprite->m_pos_z > last->m_pos_z)) {
      last = sprite;
    }
  }""",
     """  cSprite* last = nullptr;

  for (const auto& sprite : objects) {
    if (sprite->m_type == type && (!last || sprite->m_pos_z > last->m_pos_z)) {
      last = sprite.get();
    }
  }""")

# --- reverts: raw-owning lists/factories a later phase converts ------------------------------
edit("audio/audio.cpp",
     "    auto sound = std::make_unique<cAudio_Sound>();\n    m_active_sounds.push_back(sound.get());\n    return sound;",
     "    // m_active_sounds owns its entries (freed in its clear loops) -- a raw list for now\n"
     "    cAudio_Sound* sound = new cAudio_Sound();\n    m_active_sounds.push_back(sound);\n    return sound;")
sub("gui/menu.cpp", r"auto temp_item = std::make_unique<cMenu_Item>\(",
    "cMenu_Item* temp_item = new cMenu_Item(", 1)
edit("video/gl_surface.cpp", "  auto new_surface = std::make_unique<cGL_Surface>();",
     "  cGL_Surface* new_surface = new cGL_Surface();")
edit("video/gl_surface.cpp", "  auto soft_tex = std::make_unique<cSaved_Texture>();",
     "  cSaved_Texture* soft_tex = new cSaved_Texture();")
edit("objects/movingsprite.cpp", "  auto col_list = std::make_unique<cObjectCollisionType>();\n\n  bool move_x_valid = true;",
     "  cObjectCollisionType* col_list = new cObjectCollisionType();\n\n  bool move_x_valid = true;")
print("wave 1 applied")
