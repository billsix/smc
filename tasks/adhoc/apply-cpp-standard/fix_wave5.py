#!/usr/bin/env python3
"""Phase C, wave 5: an inline constructor also instantiates its members' destructors (for
unwinding), so cSoftware_Image's special members move out of line into video.cpp where the
settings type is complete; and the editor's tagged-image list declaration (which wave 4 missed).

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
     "    // (m_settings default-constructs to null; assigning nullptr here would need the\n"
     "    // complete settings type in every file that includes this header)\n"
     "    cSoftware_Image(void) { m_sdl_surface = nullptr; };",
     "    // The special members are defined in video.cpp: an inline body (even an empty one)\n"
     "    // instantiates the settings unique_ptr's deleter, which needs the complete type that\n"
     "    // this header only forward-declares. Move-only, like the unique_ptr it holds.\n"
     "    cSoftware_Image(void);\n"
     "    ~cSoftware_Image(void);\n"
     "    cSoftware_Image(cSoftware_Image&& other) noexcept;\n"
     "    cSoftware_Image& operator=(cSoftware_Image&& other) noexcept;")
if "cVideo::cSoftware_Image ::cSoftware_Image(void)" not in (SRC / "video/video.cpp").read_text():
  edit("video/video.cpp",
     "cVideo::cSoftware_Image cVideo ::Load_Image(",
       "cVideo::cSoftware_Image ::cSoftware_Image(void) : m_sdl_surface(nullptr) {}\n"
       "cVideo::cSoftware_Image ::~cSoftware_Image(void) = default;\n"
       "cVideo::cSoftware_Image ::cSoftware_Image(cSoftware_Image&& other) noexcept =\n"
       "    default;\n"
       "cVideo::cSoftware_Image& cVideo::cSoftware_Image ::operator=(\n"
       "    cSoftware_Image&& other) noexcept = default;\n\n"
       "cVideo::cSoftware_Image cVideo ::Load_Image(")
sub("core/editor.h", r"vector<cImage_Settings_Data\*>", "vector<std::unique_ptr<cImage_Settings_Data>>", 1)
print("wave 5 applied")
