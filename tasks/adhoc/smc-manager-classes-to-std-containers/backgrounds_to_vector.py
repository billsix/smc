#!/usr/bin/env python3
"""Codemod: replace cLevel's cBackground_Manager with a vector + Gradient_Background().

cBackground_Manager was an empty subclass of cObject_Manager<cBackground> (a pure
container). cLevel now holds `vector<std::unique_ptr<cBackground>> m_backgrounds`
([0] is always the gradient layer) and `cBackground& Gradient_Background()`.
Asserts the exact number of rewrites per file; not idempotent (run on HEAD).

    python3 backgrounds_to_vector.py
"""

import pathlib
import re
import subprocess

ROOT: pathlib.Path = pathlib.Path(subprocess.run(
    ["git", "rev-parse", "--show-toplevel"], cwd=pathlib.Path(__file__).parent,
    capture_output=True, text=True, check=True).stdout.strip())
SRC: pathlib.Path = ROOT / "smc" / "src"


def edit(rel: str, literal: list[tuple[str, str]], regex: list[tuple[str, str, int]]) -> None:
    path: pathlib.Path = SRC / rel
    text: str = path.read_text()
    for old, new in literal:
        assert text.count(old) == 1, (rel, old[:60])
        text = text.replace(old, new)
    for pattern, replacement, count in regex:
        text, made = re.subn(pattern, replacement, text)
        assert made == count, (rel, pattern, made)
    path.write_text(text)


def main() -> None:
    edit("level/level.h", [(
        "  cBackground_Manager m_background_manager;\n",
        "  // the backgrounds, drawn in order; [0] is always the gradient layer\n"
        "  vector<std::unique_ptr<cBackground>> m_backgrounds;\n"
        "  // the permanent gradient background (created with the level, never deleted)\n"
        "  cBackground& Gradient_Background(void) { return *m_backgrounds.front(); }\n")], [])
    edit("level/level.cpp", [], [
        (r"m_background_manager\.Add\(", "m_backgrounds.push_back(", 3),
        (r"m_background_manager\.Delete_All\(\)", "m_backgrounds.clear()", 1),
        (r"m_background_manager\.objects", "m_backgrounds", 3),
        (r"m_background_manager\.Get_Pointer\(0\)->", "Gradient_Background().", 1)])
    edit("level/level_settings.cpp", [], [
        (r"m_level->m_background_manager\.Get_Pointer\(0\)->", "m_level->Gradient_Background().", 8),
        (r"m_level->m_background_manager\.Add\(", "m_level->m_backgrounds.push_back(", 1),
        (r"m_level->m_background_manager\.Delete\(background\);",
         "std::erase_if(m_level->m_backgrounds,\n"
         "                [background](const std::unique_ptr<cBackground>& owned) {\n"
         "                  return owned.get() == background;\n"
         "                });", 1),
        (r"m_level->m_background_manager\.objects", "m_level->m_backgrounds", 1),
        (r"m_level->m_background_manager\.size\(\)", "m_level->m_backgrounds.size()", 1)])
    edit("gui/menu_data.cpp", [], [
        (r"m_level\.m_background_manager\.Get_Pointer\(0\)\s*\n\s*->", "m_level.Gradient_Background().", 6)])
    edit("level/level_background.h", [
        ('#include "../core/obj_manager.h"\n', ""),
        ("/* *** *** *** *** *** cBackground_Manager *** *** *** *** *** *** *** *** *** *** *** *** */\n\n"
         "class cBackground_Manager : public cObject_Manager<cBackground> {\n"
         " public:\n  cBackground_Manager(void);\n  ~cBackground_Manager(void) override;\n};\n\n", "")], [])
    edit("level/level_background.cpp", [(
        "/* *** *** *** *** *** *** cBackground_Manager *** *** *** *** *** *** *** *** *** *** *** */\n\n"
        "cBackground_Manager ::cBackground_Manager(void)\n    : cObject_Manager<cBackground>() {\n  //\n}\n\n"
        "cBackground_Manager ::~cBackground_Manager(void) {\n  cBackground_Manager::Delete_All();\n}\n\n", "")], [])
    print("ok")


if __name__ == "__main__":
    main()
