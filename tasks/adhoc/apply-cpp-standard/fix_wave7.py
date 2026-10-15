#!/usr/bin/env python3
"""Phase D, wave 7: a value member needs the complete type, so the owner headers include what
they used to forward-declare. Each include is inserted after the header's last `#include "…"`.

Asserting and idempotent. Paths are repo-relative.
"""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SRC = ROOT / "smc" / "src"

NEEDED = {
    "level/level.h": ["../core/sprite_manager.h", "../level/level_background.h", "../video/animation.h"],
    "overworld/overworld.h": ["../overworld/world_sprite_manager.h", "../video/animation.h", "../overworld/world_layer.h", "../gui/hud.h"],
    "gui/menu.h": ["../level/level.h", "../core/camera.h", "../objects/sprite.h", "../video/animation.h"],
    "level/level_settings.h": ["../core/camera.h"],
    "level/level_manager.h": ["../core/camera.h"],
    "overworld/world_manager.h": ["../core/camera.h"],
    "level/level_editor.h": ["../level/level_settings.h"],
    "overworld/world_player.h": ["../gui/hud.h"],
}

for rel, includes in NEEDED.items():
    path = SRC / rel
    text = path.read_text()
    missing = [inc for inc in includes if f'#include "{inc}"' not in text]
    if not missing:
        continue
    lines = text.split("\n")
    last = max(i for i, l in enumerate(lines) if re.match(r'#include "', l))
    for inc in reversed(missing):
        lines.insert(last + 1, f'#include "{inc}"')
    path.write_text("\n".join(lines))
    print(f"{rel}: +{len(missing)} includes")
print("wave 7 applied")
