#!/usr/bin/env python3
"""Phase D, wave 9: the compiler residue of wave 8 (eight sites).

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


def sub(rel: str, pattern: str, repl: str, count: int, flags: int = 0) -> None:
    path = SRC / rel
    text = path.read_text()
    new_text, n = re.subn(pattern, repl, text, flags=flags)
    if n == 0:
        return
    assert n == count, f"{rel}: expected {count} of {pattern!r}, got {n}"
    path.write_text(new_text)


# the pool's lookups return a borrowed pointer
edit("audio/audio.cpp", "      // return first found\n      return sound;", "      // return first found\n      return sound.get();")
edit("audio/audio.cpp", "      // found a free channel\n      sound->Free();\n      return sound;", "      // found a free channel\n      sound->Free();\n      return sound.get();")
# iterator loops over the owning selection list
sub("core/editor.cpp", r"      cSelectedObject\* sel_obj = \(\*itr\);", "      cSelectedObject* sel_obj = itr->get();", 2)
# the points text (its class name has no `c` prefix, so the codemod skipped it)
edit("gui/hud.cpp", "  PointsText* new_obj = new PointsText(m_sprite_manager);", "  auto new_obj = std::make_unique<PointsText>(m_sprite_manager);")
edit("gui/hud.cpp", "  m_points_objects.push_back(new_obj);", "  m_points_objects.push_back(std::move(new_obj));")
# an inline `push_back(new …)`
edit("gui/menu_data.cpp", "    m_savegame_temp.push_back(\n        new cHudSprite(&pMenuCore->m_handler.m_level.m_sprite_manager));",
     "    m_savegame_temp.push_back(std::make_unique<cHudSprite>(\n        &pMenuCore->m_handler.m_level.m_sprite_manager));")
# borrowed pointers into the selection helpers
edit("input/mouse.cpp", "      Update_Selected_Object_Offset(sel_obj);\n    }\n  }\n\n  // if a user selected object is not active",
     "      Update_Selected_Object_Offset(sel_obj.get());\n    }\n  }\n\n  // if a user selected object is not active")
edit("input/mouse.cpp", "    if (m_left) {\n      Set_Object_Position(sel_obj);\n    }", "    if (m_left) {\n      Set_Object_Position(sel_obj.get());\n    }")
print("wave 9 applied")
