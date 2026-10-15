#!/usr/bin/env python3
"""Phase D, wave 11: the `del_img` flag also threaded through cAnimated_Sprite::Set_Image_Num
(frames are borrowed from the image manager, never owned), and two direct three-argument calls.

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


sub("objects/animated_sprite.h",
    r"virtual void Set_Image_Num\(const int num, const bool new_startimage = false,\s*const bool del_img = false\);",
    "virtual void Set_Image_Num(const int num, const bool new_startimage = false);", 1)
edit("objects/animated_sprite.cpp",
     "                                      const bool new_startimage /* = 0 */,\n                                      const bool del_img /* = 0 */) {",
     "                                      const bool new_startimage /* = 0 */) {")
edit("objects/animated_sprite.cpp", "    cMovingSprite::Set_Image(nullptr, new_startimage, del_img);", "    cMovingSprite::Set_Image(nullptr, new_startimage);")
edit("objects/animated_sprite.cpp",
     "    cMovingSprite::Set_Image(m_images[m_curr_img].m_image, new_startimage,\n                             del_img);",
     "    cMovingSprite::Set_Image(m_images[m_curr_img].m_image, new_startimage);")
for path in sorted(SRC.rglob("*.cpp")):
    text = path.read_text()
    new = re.sub(r"Set_Image_Num\(([^;]*?),\s*(true|false)\s*,\s*false\)", r"Set_Image_Num(\1, \2)", text)
    new = re.sub(r"cMovingSprite::Set_Image\(nullptr, false, false\);", "cMovingSprite::Set_Image(nullptr, false);", new)
    if new != text:
        path.write_text(new)
print("wave 11 applied")
