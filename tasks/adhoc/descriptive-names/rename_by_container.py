#!/usr/bin/env python3
"""Descriptive names, pass 2: rename generic range-for vars (`obj`/`object`) to a type-anchored
name chosen from the CONTAINER expression (reliable across mixed files, unlike a per-file map).

For `for ((const )?auto& VAR : CONTAINER)` with VAR in {obj, object}: pick ELEMENT from CONTAINER
by the ordered rules below; if the block already uses ELEMENT as a word (a param/local collision),
SKIP and report; otherwise scoped-rename VAR -> ELEMENT within the block. Simple-form loops only
(these vars are used via `->`/`.get()`; extraction loops use `*_ptr` names, done by hand).

Idempotent; run from anywhere; the compiler + a play-test are the gates.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

REPO = Path(subprocess.check_output(["git", "rev-parse", "--show-toplevel"], text=True).strip())
SRC = REPO / "smc" / "src"

# ordered (container substring -> element); first match wins
RULES: list[tuple[str, str]] = [
    ("col_list_temp->objects", "collision"),
    ("col_list->objects", "collision"),
    ("m_sprite_manager->objects", "sprite"),
    ("sprite_manager->objects", "sprite"),
    ("m_background_manager->objects", "background"),
    ("Image_Manager->objects", "surface"),
    ("Campaign_Manager->objects", "campaign"),
    ("Overworld_Manager->objects", "overworld"),
    ("Level_Manager->objects", "level"),
    ("animation_manager->objects", "animation"),
    ("m_spawned_objects", "sprite"),
    ("m_level_spawned_objects", "sprite"),
    ("m_active_waypoints", "save_waypoint"),
    ("m_waypoints", "waypoint"),
    ("m_level_objects", "save_object"),
    ("m_levels", "save_level"),
    ("m_overworlds", "save_overworld"),
    ("m_collisions", "collision"),
    ("m_render_data", "request"),
    ("m_active_sounds", "sound"),
    ("m_active_fonts", "font"),
    ("m_linked_path_states", "path_state"),
    ("m_editor_windows", "editor_window"),
    ("m_selected_objects", "selected"),
    ("m_active_items", "item"),
    ("m_points_objects", "points_text"),
    ("m_draw_list", "item"),
    ("m_tagged_item_objects", "tagged_object"),
    ("m_images", "image"),
    ("m_waypoints", "waypoint"),
    ("spritelist", "sprite"),
    ("sprite_objects", "sprite"),
    ("new_objects", "sprite"),
]
# bare container `objects` (a manager's own member) -> element, by file
BARE_OBJECTS_BY_FILE: dict[str, str] = {
    "hud.cpp": "hud_sprite",
    "editor.cpp": "sprite",
    "animation.cpp": "animation",
    "world_manager.cpp": "overworld",
}
GENERIC = {"obj", "object"}
FOR = re.compile(r"for \(\s*(?:const\s+)?auto\s*&?\s*(\w+)\s*:\s*([^)]+?)\)")


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


def element_for(container: str, fname: str) -> str | None:
    if container.strip() == "objects":
        return BARE_OBJECTS_BY_FILE.get(fname)
    for sub, elem in RULES:
        if sub in container:
            return elem
    return None


def fix(path: Path) -> tuple[int, list[str]]:
    text = path.read_text(encoding="utf-8", errors="surrogateescape")
    n, pos, skips = 0, 0, []
    while True:
        m = FOR.search(text, pos)
        if not m:
            break
        var, container = m.group(1), m.group(2)
        if var not in GENERIC:
            pos = m.end()
            continue
        elem = element_for(container, path.name)
        if not elem:
            pos = m.end()
            continue
        brace = text.find("{", m.end())
        if brace == -1:
            pos = m.end()
            continue
        end = block_end(text, brace)
        block = text[m.start():end]
        # collision guard: ELEMENT already used (not as VAR) in the block?
        others = [w for w in re.findall(rf"\b{elem}\b", block)]
        if others:
            skips.append(f"{path.name}: {var}->{elem} skipped ({elem} already in block)")
            pos = m.end()
            continue
        new_block = re.sub(rf"\b{re.escape(var)}\b", elem, block)
        text = text[:m.start()] + new_block + text[end:]
        n += 1
        pos = m.start() + len(new_block)
    if n:
        path.write_text(text, encoding="utf-8", errors="surrogateescape")
    return n, skips


def main() -> int:
    total, all_skips = 0, []
    for path in sorted(SRC.rglob("*.cpp")) + sorted(SRC.rglob("*.h")):
        n, skips = fix(path)
        if n:
            print(f"{path.relative_to(REPO)}: {n}")
            total += n
        all_skips += skips
    print(f"=== total renamed: {total}")
    if all_skips:
        print("=== skipped (hand-review):")
        print("\n".join(all_skips))
    return 0


if __name__ == "__main__":
    sys.exit(main())
