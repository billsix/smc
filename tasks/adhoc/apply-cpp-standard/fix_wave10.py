#!/usr/bin/env python3
"""Phase D, wave 10: the conditional-ownership pairs (decision 1b) and the last raw owners.

  * cSprite: `m_image` + `bool m_delete_image` becomes an owning `std::unique_ptr<cGL_Surface>
    m_owned_image` beside the raw `m_image` view. `Set_Image(cGL_Surface*, bool)` borrows;
    a new `Set_Image(std::unique_ptr<cGL_Surface>, bool)` overload takes ownership. The
    `del_img` flag parameter is gone: every `Set_Image(pFont->Render_Text(…), x, true)` call
    becomes `Set_Image(pFont->Render_Text(…), x)` and resolves to the owning overload because
  * cFont_Manager::Render_Text returns std::unique_ptr<cGL_Surface> (the font manager only keeps
    a borrowed reference, which the surface removes in its destructor); its non-sprite callers
    hold the unique_ptr and drop their `delete`.
  * cLevel_Entry/cLevel_Exit::m_editor_entry_name own their text surface through unique_ptr.
  * cMouseCursor::m_hovering_object is a value member.

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


# --- A. cSprite owns its image through a unique_ptr; the raw m_image is the view ---------------
edit("objects/sprite.h",
     "	 * if del_img is set the given image will be deleted\n	*/\n  virtual void Set_Image(cGL_Surface* new_image, bool new_start_image = false,\n                         bool del_img = false);",
     "	 * the sprite BORROWS the image (an image manager or another owner keeps it alive)\n	*/\n  virtual void Set_Image(cGL_Surface* new_image, bool new_start_image = false);\n"
     "  /* Set the image and take ownership of it (a text surface rendered for this sprite):\n"
     "	 * it is freed when the sprite dies or the next image is set\n	*/\n"
     "  void Set_Image(std::unique_ptr<cGL_Surface> owned_image,\n                 bool new_start_image = false);")
edit("objects/sprite.h",
     "  // delete the given image when it gets unloaded\n  bool m_delete_image;",
     "  // the image this sprite owns, if any (m_image is the view of it or of a borrowed one)\n  std::unique_ptr<cGL_Surface> m_owned_image;")
edit("objects/sprite.cpp",
     "cSprite ::~cSprite(void) {\n  if (m_delete_image && m_image) {\n    delete m_image;\n    m_image = nullptr;\n  }\n}",
     "cSprite ::~cSprite(void) = default;")
edit("objects/sprite.cpp", "  m_auto_destroy = false;\n  m_delete_image = false;\n", "  m_auto_destroy = false;\n")
edit("objects/sprite.cpp",
     "void cSprite ::Set_Image(cGL_Surface* new_image, bool new_start_image /* = 0 */,\n                         bool del_img /* = 0 */) {\n"
     "  if (m_delete_image) {\n    if (m_image) {\n      // if same image reset start_image\n      if (m_start_image == m_image) {\n        m_start_image = nullptr;\n      }\n\n      delete m_image;\n      m_image = nullptr;\n    }\n\n    m_delete_image = false;\n  }\n\n  m_image = new_image;",
     "void cSprite ::Set_Image(std::unique_ptr<cGL_Surface> owned_image,\n                         bool new_start_image /* = 0 */) {\n"
     "  cGL_Surface* view = owned_image.get();\n  Set_Image(view, new_start_image);\n  m_owned_image = std::move(owned_image);\n}\n\n"
     "void cSprite ::Set_Image(cGL_Surface* new_image, bool new_start_image /* = 0 */) {\n"
     "  // an image this sprite owned is freed once something else replaces it\n"
     "  if (m_owned_image && new_image != m_owned_image.get()) {\n    if (m_start_image == m_owned_image.get()) {\n      m_start_image = nullptr;\n    }\n    m_owned_image.reset();\n  }\n\n  m_image = new_image;")
edit("objects/sprite.cpp", "    m_delete_image = del_img;\n\n", "")
edit("objects/movingsprite.h",
     "  void Set_Image(cGL_Surface* new_image, bool new_start_image = false,\n                 bool del_img = false) override;",
     "  void Set_Image(cGL_Surface* new_image, bool new_start_image = false) override;\n  // the owning overload stays visible through this class\n  using cSprite::Set_Image;")
edit("objects/movingsprite.cpp",
     "void cMovingSprite ::Set_Image(cGL_Surface* new_image,\n                               bool new_start_image /* = 0 */,\n                               bool del_img /* = 0 */) {",
     "void cMovingSprite ::Set_Image(cGL_Surface* new_image,\n                               bool new_start_image /* = 0 */) {")
sub("objects/movingsprite.cpp", r"cSprite::Set_Image\(new_image, new_start_image, del_img\);", "cSprite::Set_Image(new_image, new_start_image);", None)

# --- B. Render_Text returns unique_ptr ----------------------------------------------------------
sub("video/font.h", r"  cGL_Surface\* Render_Text\(", "  std::unique_ptr<cGL_Surface> Render_Text(", 1)
edit("video/font.cpp", "cGL_Surface* cFont_Manager ::Render_Text(TTF_Font* font,", "std::unique_ptr<cGL_Surface> cFont_Manager ::Render_Text(TTF_Font* font,")
edit("video/font.cpp",
     "  // the caller owns the rendered surface (a raw hand-off until sprites own their images)\n  cGL_Surface* surface =\n      pVideo\n          ->Create_Texture(TTF_RenderUTF8_Blended(font, text.c_str(), sdlcolor))\n          .release();",
     "  // the caller owns the rendered surface; this manager only keeps a borrowed reference\n  std::unique_ptr<cGL_Surface> surface =\n      pVideo->Create_Texture(TTF_RenderUTF8_Blended(font, text.c_str(), sdlcolor));")
edit("video/font.cpp", "  Add_Ref(surface);", "  Add_Ref(surface.get());")

# --- C. the owning Set_Image calls: drop the `, true` flag; the unique_ptr argument selects the overload
INNER = r"(?:[^()]|\((?:[^()]|\((?:[^()]|\([^()]*\))*\))*\))*"
OWNING_CALL = re.compile(r"Set_Image\(\s*(pFont->Render_Text\(" + INNER + r"\))\s*,\s*(true|false)\s*,\s*true\s*\)", re.S)
for path in sorted(SRC.rglob("*.cpp")):
    text = path.read_text()
    new = OWNING_CALL.sub(r"Set_Image(\1, \2)", text)
    if new != text:
        path.write_text(new)
# the surfaces shared between a menu item (borrows) and a hud sprite (owns)
sub("gui/menu_data.cpp", r"cGL_Surface\* credits =", "std::unique_ptr<cGL_Surface> credits =", 1)
sub("gui/menu_data.cpp", r"cGL_Surface\* back1 =", "std::unique_ptr<cGL_Surface> back1 =", 2)
sub("gui/menu_data.cpp", r"m_image_default\.Set_Image\(credits\);", "m_image_default.Set_Image(credits.get());", 1)
sub("gui/menu_data.cpp", r"m_image_default\.Set_Image\(back1\);", "m_image_default.Set_Image(back1.get());", 2)
sub("gui/menu_data.cpp", r"hud_sprite->Set_Image\(credits, false, true\);", "hud_sprite->Set_Image(std::move(credits), false);", 1)
sub("gui/menu_data.cpp", r"hud_sprite->Set_Image\(back1, false, true\);", "hud_sprite->Set_Image(std::move(back1), false);", 2)
edit("level/level.cpp", "      cGL_Surface* text_image = pFont->Render_Text(pFont->m_font_small, text);",
     "      std::unique_ptr<cGL_Surface> text_image =\n          pFont->Render_Text(pFont->m_font_small, text);")
edit("level/level.cpp", "      sprite->Set_Image(text_image, true, true);", "      sprite->Set_Image(std::move(text_image), true);")

# --- D. the non-sprite Render_Text callers hold the unique_ptr ----------------------------------
sub("video/video.cpp", r"cGL_Surface\* surface_filename =", "std::unique_ptr<cGL_Surface> surface_filename =", 1)
sub("video/video.cpp", r"[ \t]*// delete\n[ \t]*delete surface_filename;\n", "", 1)
sub("input/mouse.cpp", r"cGL_Surface\* position_info =", "std::unique_ptr<cGL_Surface> position_info =", 1)
sub("input/mouse.cpp", r"[ \t]*delete position_info;\n", "", 2)
sub("gui/hud.cpp", r"cGL_Surface\* surface_temp =", "std::unique_ptr<cGL_Surface> surface_temp =", 1)
sub("gui/hud.cpp", r"[ \t]*delete surface_temp;\n", "", 1)

# --- E. the editor name surfaces --------------------------------------------------------------
for rel in ["objects/level_entry", "objects/level_exit"]:
    edit(rel + ".h", "  cGL_Surface* m_editor_entry_name;", "  std::unique_ptr<cGL_Surface> m_editor_entry_name;")
    sub(rel + ".cpp", r"[ \t]*(?://[^\n]*\n[ \t]*)?if \(m_editor_entry_name\) \{\n[ \t]*delete m_editor_entry_name;\n[ \t]*m_editor_entry_name = nullptr;\n[ \t]*\}\n\n?", "", 2)
sub("objects/level_entry.cpp", r"cLevel_Entry ::~cLevel_Entry\(void\) \{\n\}", "cLevel_Entry ::~cLevel_Entry(void) = default;", None)
sub("objects/level_exit.cpp", r"cLevel_Exit ::~cLevel_Exit\(void\) \{\n\}", "cLevel_Exit ::~cLevel_Exit(void) = default;", None)

# --- F. the hovering object is a value ------------------------------------------------------------
edit("input/mouse.h", "  cSelectedObject* m_hovering_object;", "  cSelectedObject m_hovering_object;")
edit("input/mouse.cpp", "  m_hovering_object = new cSelectedObject();\n", "")
edit("input/mouse.cpp", "  Clear_Selected_Objects();\n  delete m_hovering_object;\n}", "  Clear_Selected_Objects();\n}")
for path in sorted(SRC.rglob("*.cpp")) + sorted(SRC.rglob("*.h")):
    text = path.read_text()
    new = re.sub(r"->m_hovering_object->", "->m_hovering_object.", text)
    if path.name == "mouse.cpp":
        new = re.sub(r"(?<![\w.>])m_hovering_object->", "m_hovering_object.", new)
        new = re.sub(r"([(,]\s*)m_hovering_object(\s*[,)])", r"\1&m_hovering_object\2", new)
    if new != text:
        path.write_text(new)
print("wave 10 applied")
