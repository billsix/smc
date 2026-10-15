#!/usr/bin/env python3
"""Phase C, wave 3: the image-settings ownership chain.

`cVideo::cSoftware_Image` carried a raw `cImage_Settings_Data*` that its two consumers had to
`delete` after use, and `Load_Image` built it from a raw local. Now the struct owns the settings
through a unique_ptr (the struct is returned by value, which moves), the local in `Load_Image` is
a unique_ptr moved into it, and the consumers borrow with `.get()` and free nothing.

Asserting and idempotent. Paths are repo-relative.
"""

from __future__ import annotations

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


edit("video/video.h",
     "    SDL_Surface* m_sdl_surface;\n    cImage_Settings_Data* m_settings;",
     "    // the SDL surface is a C handle the consumer frees with SDL_FreeSurface\n"
     "    SDL_Surface* m_sdl_surface;\n"
     "    // the image's settings (owned; freed with this struct)\n"
     "    std::unique_ptr<cImage_Settings_Data> m_settings;")
edit("video/video.cpp", "  cImage_Settings_Data* settings = nullptr;",
     "  std::unique_ptr<cImage_Settings_Data> settings;")
edit("video/video.cpp",
     "  if (!sdl_surface) {\n    if (settings) {\n      delete settings;\n      settings = nullptr;\n    }\n",
     "  if (!sdl_surface) {\n    settings.reset();\n")
edit("video/video.cpp", "  software_image.m_settings = settings;", "  software_image.m_settings = std::move(settings);")
edit("video/video.cpp", "    cImage_Settings_Data* settings = software_image.m_settings;",
     "    cImage_Settings_Data* settings = software_image.m_settings.get();")
edit("video/video.cpp", "  cImage_Settings_Data* settings = software_image.m_settings;",
     "  cImage_Settings_Data* settings = software_image.m_settings.get();")
edit("video/video.cpp",
     "    cSize_Int size = settings->Get_Surface_Size(sdl_surface);\n    delete settings;\n    int new_width = size.m_width;",
     "    cSize_Int size = settings->Get_Surface_Size(sdl_surface);\n    int new_width = size.m_width;")
edit("video/video.cpp", "    settings->Apply(image.get());\n    delete settings;\n  }",
     "    settings->Apply(image.get());\n  }")
print("wave 3 applied")
