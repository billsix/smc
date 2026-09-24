#!/usr/bin/env python3
"""Descriptive-variable-names: scoped rename of generic range-for loop variables to type-anchored
names (e.g. `obj_ptr`/`obj`/`object` over a cSprite container -> `sprite`).

For each configured file, for every `for ((const )?auto& VAR : CONTAINER)` whose VAR is a key in
that file's rename map, apply the map (whole-word) WITHIN that loop's brace block only. Applying the
whole map inside the block also fixes the raw handle the body extracts (e.g.
`cSprite* obj = sprite_ptr.get()` when the map has both obj_ptr->sprite_ptr and obj->sprite).

Scoped to the block so identically-named variables elsewhere are untouched; the compiler verifies
nothing broke. Idempotent (a renamed loop var is no longer a map key). Run from anywhere.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

REPO = Path(subprocess.check_output(["git", "rev-parse", "--show-toplevel"], text=True).strip())

# file -> { generic loop-var : type-anchored name }.  Only files whose loops iterate ONE element
# type (mixed files are done by hand).
RENAMES: dict[str, dict[str, str]] = {
    "smc/src/core/sprite_manager.cpp": {"obj_ptr": "sprite_ptr", "obj": "sprite", "object": "sprite"},
    "smc/src/core/sprite_manager.h": {"object": "sprite"},
    "smc/src/audio/sound_manager.cpp": {"obj": "sound"},
    "smc/src/core/campaign_manager.cpp": {"obj": "campaign"},
    "smc/src/overworld/world_manager.cpp": {"obj": "overworld"},
    "smc/src/level/level_manager.cpp": {"obj": "level"},
    "smc/src/video/img_manager.cpp": {"obj": "surface"},
    "smc/src/core/collision.cpp": {"obj": "collision"},
}

FOR = re.compile(r"for \(\s*(?:const\s+)?auto\s*&\s*(\w+)\s*:")


def block_end(text: str, brace: int) -> int:
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return i + 1
    raise ValueError("unbalanced")


def fix(path: Path, rmap: dict[str, str]) -> int:
    text = path.read_text(encoding="utf-8", errors="surrogateescape")
    n, pos = 0, 0
    while True:
        m = FOR.search(text, pos)
        if not m:
            break
        var = m.group(1)
        if var not in rmap:
            pos = m.end()
            continue
        brace = text.find("{", m.end())
        if brace == -1:
            pos = m.end()
            continue
        end = block_end(text, brace)
        block = text[m.start():end]
        new_block = block
        for old, new in rmap.items():
            new_block = re.sub(rf"\b{re.escape(old)}\b", new, new_block)
        text = text[:m.start()] + new_block + text[end:]
        n += 1
        pos = m.start() + len(new_block)
    if n:
        path.write_text(text, encoding="utf-8", errors="surrogateescape")
    return n


def main() -> int:
    total = 0
    for rel, rmap in RENAMES.items():
        k = fix(REPO / rel, rmap)
        print(f"{rel}: {k} loop(s) renamed")
        total += k
    print(f"=== total loops renamed: {total}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
