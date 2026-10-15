#!/usr/bin/env python3
"""Phase C, hand fixes beyond the collision-list codemod: the other transient owners.

  * cSavegame::Load / cSavegame_XML_Handler::Acquire_Savegame return std::unique_ptr<cSave>;
    the handler's m_savegame is a unique_ptr; Save_Game builds its cSave with make_unique; the
    XML handler is a stack object; every `delete savegame` goes.
  * cImage_Settings_Parser::Get returns std::unique_ptr<cImage_Settings_Data>; m_settings_temp
    is a unique_ptr; the nested "base settings" parser is a stack object.
  * the two dialog-box helpers use stack objects.
  * the screenshot buffer, the PNG row buffers and the line parser's parts array are std::vector.

Asserting and idempotent (a second run finds nothing). Paths are repo-relative.
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


# --- savegame ----------------------------------------------------------------------------------
edit("user/savegame.h", "  cSave* Load(unsigned int save_slot);",
     "  std::unique_ptr<cSave> Load(unsigned int save_slot);")
edit("user/savegame.h", "  cSave* Acquire_Savegame(void);",
     "  std::unique_ptr<cSave> Acquire_Savegame(void);")
edit("user/savegame.h", "  cSave* m_savegame;", "  std::unique_ptr<cSave> m_savegame;")
sub("user/savegame.cpp", r"^  cSave\* savegame = Load\(save_slot\);", "  std::unique_ptr<cSave> savegame = Load(save_slot);", 2, re.M)
sub("user/savegame.cpp", r"^  delete savegame;\n", "", 3, re.M)
edit("user/savegame.cpp", "  cSave* savegame = new cSave();", "  auto savegame = std::make_unique<cSave>();")
edit("user/savegame.cpp", "  Save(save_slot, savegame);", "  Save(save_slot, savegame.get());")
edit("user/savegame.cpp", "cSave* cSavegame ::Load(unsigned int save_slot) {",
     "std::unique_ptr<cSave> cSavegame ::Load(unsigned int save_slot) {")
edit("user/savegame.cpp",
     """  cSavegame_XML_Handler* loader = new cSavegame_XML_Handler(filename);
  cSave* savegame = loader->Acquire_Savegame();
  delete loader;

  return savegame;
}""",
     """  // the parser builds the save; take it over when parsing is done
  cSavegame_XML_Handler loader(filename);
  return loader.Acquire_Savegame();
}""")
edit("user/savegame.cpp", "  m_savegame = new cSave();", "  m_savegame = std::make_unique<cSave>();")
edit("user/savegame.cpp",
     "cSavegame_XML_Handler ::~cSavegame_XML_Handler(void) {\n  if (m_savegame) {\n    delete m_savegame;\n  }\n}",
     "cSavegame_XML_Handler ::~cSavegame_XML_Handler(void) = default;")
edit("user/savegame.cpp",
     "cSave* cSavegame_XML_Handler ::Acquire_Savegame(void) {\n  cSave* save = m_savegame;\n  m_savegame = nullptr;\n  return save;\n}",
     "std::unique_ptr<cSave> cSavegame_XML_Handler ::Acquire_Savegame(void) {\n  return std::move(m_savegame);\n}")
edit("gui/menu_data.cpp",
     "  cSave* savegame = pSavegame->Load(save_num);\n  std::string level_name = savegame->Get_Active_Level();\n  delete savegame;",
     "  std::unique_ptr<cSave> savegame = pSavegame->Load(save_num);\n  std::string level_name = savegame->Get_Active_Level();")

# --- image settings parser -------------------------------------------------------------------
sub("video/img_settings.h", r"  cImage_Settings_Data\* Get\(", "  std::unique_ptr<cImage_Settings_Data> Get(", 1)
edit("video/img_settings.h", "  cImage_Settings_Data* m_settings_temp;",
     "  // the settings being parsed (owned until Get hands them to the caller)\n  std::unique_ptr<cImage_Settings_Data> m_settings_temp;")
edit("video/img_settings.cpp", "cImage_Settings_Parser ::cImage_Settings_Parser(void) : cFile_parser() {\n  m_settings_temp = nullptr;\n  m_load_base = true;\n}",
     "cImage_Settings_Parser ::cImage_Settings_Parser(void) : cFile_parser() {\n  m_load_base = true;\n}")
edit("video/img_settings.cpp",
     """cImage_Settings_Data* cImage_Settings_Parser ::Get(
    const std::string& filename, bool load_base_settings /* = 1 */) {
  m_load_base = load_base_settings;
  m_settings_temp = new cImage_Settings_Data();

  Parse(filename);
  cImage_Settings_Data* settings = m_settings_temp;
  m_settings_temp = nullptr;
  return settings;
}""",
     """std::unique_ptr<cImage_Settings_Data> cImage_Settings_Parser ::Get(
    const std::string& filename, bool load_base_settings /* = 1 */) {
  m_load_base = load_base_settings;
  m_settings_temp = std::make_unique<cImage_Settings_Data>();

  Parse(filename);
  return std::move(m_settings_temp);
}""")
edit("video/img_settings.cpp",
     """          cImage_Settings_Parser* temp_parser = new cImage_Settings_Parser();
          cImage_Settings_Data* base_settings = temp_parser->Get(settings_file);
          // finished loading base settings
          delete temp_parser;
          settings_file.clear();""",
     """          cImage_Settings_Parser temp_parser;
          std::unique_ptr<cImage_Settings_Data> base_settings =
              temp_parser.Get(settings_file);
          settings_file.clear();""")
edit("video/img_settings.cpp", "            m_settings_temp->Apply_Base(base_settings);",
     "            m_settings_temp->Apply_Base(base_settings.get());")
edit("video/img_settings.cpp", "              settings_file = base_settings->m_base;\n            }\n\n            delete base_settings;\n",
     "              settings_file = base_settings->m_base;\n            }\n")

# --- dialog boxes ------------------------------------------------------------------------------
edit("gui/generic.cpp",
     "  cDialogBox_Text* box = new cDialogBox_Text();\n  std::string return_value = box->Enter(default_text, title_text, auto_no_text);\n  delete box;\n\n  return return_value;",
     "  cDialogBox_Text box;\n  return box.Enter(default_text, title_text, auto_no_text);")
edit("gui/generic.cpp",
     "  cDialogBox_Question* box = new cDialogBox_Question();\n  int return_value = box->Enter(text, with_cancel);\n  delete box;\n\n  return return_value;",
     "  cDialogBox_Question box;\n  return box.Enter(text, with_cancel);")

# --- raw arrays -> std::vector ----------------------------------------------------------------
edit("video/video.cpp",
     """      GLubyte* data = new GLubyte[pPreferences->m_video_screen_w *
                                  pPreferences->m_video_screen_h * 3];
      // read opengl screen
      glReadPixels(0, 0, pPreferences->m_video_screen_w,
                   pPreferences->m_video_screen_h, GL_RGB, GL_UNSIGNED_BYTE,
                   static_cast<GLvoid*>(data));
      // save
      Save_Surface(filename, data, pPreferences->m_video_screen_w,
                   pPreferences->m_video_screen_h, 3, true);
      // clear data
      delete[] data;
""",
     """      std::vector<GLubyte> data(pPreferences->m_video_screen_w *
                                pPreferences->m_video_screen_h * 3);
      // read opengl screen
      glReadPixels(0, 0, pPreferences->m_video_screen_w,
                   pPreferences->m_video_screen_h, GL_RGB, GL_UNSIGNED_BYTE,
                   static_cast<GLvoid*>(data.data()));
      // save
      Save_Surface(filename, data.data(), pPreferences->m_video_screen_w,
                   pPreferences->m_video_screen_h, 3, true);
""")
edit("video/video.cpp",
     "  png_byte* image = new png_byte[png_height * row_bytes];\n  png_bytep* row_pointers = new png_bytep[png_height];",
     "  std::vector<png_byte> image(png_height * row_bytes);\n  std::vector<png_bytep> row_pointers(png_height);")
sub("video/video.cpp", r"row_pointers\[(png_height - 1 - i|i)\] = image \+ \(i \* row_bytes\);",
    r"row_pointers[\1] = image.data() + (i * row_bytes);", 2)
edit("video/video.cpp", "  png_write_image(png_ptr, row_pointers);", "  png_write_image(png_ptr, row_pointers.data());")
edit("video/video.cpp", "  delete[] image;\n  delete[] row_pointers;\n\n  fclose(fp);", "  fclose(fp);")
edit("core/file_parser.cpp", "  std::string* parts = new std::string[count + 1];", "  std::vector<std::string> parts(count + 1);")
edit("core/file_parser.cpp", "  bool success = HandleMessage(parts, part_count, line_num);\n\n  delete[] parts;\n",
     "  bool success = HandleMessage(parts.data(), part_count, line_num);\n")
print("wave 2 applied")
