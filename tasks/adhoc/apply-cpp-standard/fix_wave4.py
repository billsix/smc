#!/usr/bin/env python3
"""Phase C, wave 4: the two sites the wave-3 settings chain surfaced.

  * cVideo::cSoftware_Image's inline constructor assigned nullptr to the settings unique_ptr,
    which instantiates the deleter in every translation unit that includes video.h -- where the
    settings type is only forward-declared. A default-constructed unique_ptr is already null, so
    the assignment goes (the same incomplete-type lesson as Save_To_Savegame).
  * the editor's tagged image settings (cEditor::m_tagged_item_images) own their entries through
    unique_ptr, so the parser's unique_ptr result moves straight in and the delete loop goes.

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


edit("video/video.h",
     "    cSoftware_Image(void) {\n      m_sdl_surface = nullptr;\n      m_settings = nullptr;\n    };",
     "    // (m_settings default-constructs to null; assigning nullptr here would need the\n"
     "    // complete settings type in every file that includes this header)\n"
     "    cSoftware_Image(void) { m_sdl_surface = nullptr; };")
sub("core/editor.h", r"using TaggedItemImagesList = vector<cImage_Settings_Data\*>;",
    "using TaggedItemImagesList = vector<std::unique_ptr<cImage_Settings_Data>>;", 1)
edit("core/editor.cpp",
     "  // Tagged Image Settings\n  for (auto& tagged_item_image : m_tagged_item_images) {\n    delete tagged_item_image;\n  }\n\n  m_tagged_item_images.clear();",
     "  // Tagged Image Settings (owned: clearing frees them)\n  m_tagged_item_images.clear();")
sub("core/editor.cpp", r"cImage_Settings_Data\* settings = pSettingsParser->Get\(",
    "std::unique_ptr<cImage_Settings_Data> settings = pSettingsParser->Get(", 1)
sub("core/editor.cpp", r"m_tagged_item_images\.push_back\(settings\);",
    "m_tagged_item_images.push_back(std::move(settings));", 1)
sub("core/editor.cpp", r"for \(auto settings : m_tagged_item_images\) \{",
    "for (const auto& settings : m_tagged_item_images) {", 1)
print("wave 4 applied")
