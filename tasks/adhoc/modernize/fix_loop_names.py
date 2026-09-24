#!/usr/bin/env python3
"""Fix the m_-prefixed range-for locals that clang-tidy's modernize-loop-convert generates.

clang-tidy names each range-for variable after its container, so a member container `m_foos`
yields a LOCAL `m_foo` -- which violates the `m_` = member convention. This renames each such
loop variable to its stripped name (`m_foo` -> `foo`), scoped to that loop's brace block only,
so a same-named token elsewhere in the file is untouched. If the stripped name already occurs
inside the loop body (a real collision), the loop is left as-is and reported for a hand fix.

Idempotent: a second run finds no `for (auto ... m_x : ...)` and changes nothing.

Run from anywhere; paths resolve to the repo root via git.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT: Path = Path(
    subprocess.check_output(["git", "rev-parse", "--show-toplevel"], text=True).strip()
)

# the files clang-tidy left an m_-prefixed range-for local in (from the loop-convert run)
TARGET_FILES: list[str] = [
    "smc/src/user/savegame.cpp",
    "smc/src/video/animation.cpp",
    "smc/src/input/mouse.cpp",
    "smc/src/gui/hud.cpp",
    "smc/src/gui/menu.cpp",
    "smc/src/core/editor.cpp",
    "smc/src/audio/audio.cpp",
    "smc/src/video/font.cpp",
    "smc/src/objects/sprite.cpp",
    "smc/src/objects/path.cpp",
    "smc/src/objects/bonusbox.cpp",
    "smc/src/level/level_editor.cpp",
]

FOR_LINE: re.Pattern[str] = re.compile(
    r"for\s*\(\s*(?:const\s+)?auto\s*&?\s*&?\s*(m_\w+)\s*:"
)


def block_end(text: str, open_brace_index: int) -> int:
    """Return the index just past the `}` matching the `{` at open_brace_index."""
    depth: int = 0
    for i in range(open_brace_index, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return i + 1
    raise ValueError("unbalanced braces")


def fix_file(path: Path) -> tuple[int, list[str]]:
    # surrogateescape round-trips any non-UTF-8 bytes (a couple of files use Latin-1 degree signs)
    text: str = path.read_text(encoding="utf-8", errors="surrogateescape")
    fixed: int = 0
    skipped: list[str] = []
    # walk matches left to right, re-searching from a moving cursor (offsets change as we edit)
    search_from: int = 0
    while True:
        match = FOR_LINE.search(text, search_from)
        if match is None:
            break
        m_name: str = match.group(1)
        stripped: str = m_name[2:]  # drop the leading "m_"
        brace: int = text.find("{", match.end())
        if brace == -1:
            skipped.append(f"{m_name}: no brace body")
            search_from = match.end()
            continue
        end: int = block_end(text, brace)
        body: str = text[brace:end]
        # collision guard: does the stripped name already appear in the body as its own token?
        others: int = len(
            [
                w
                for w in re.finditer(rf"\b{re.escape(stripped)}\b", body)
            ]
        )
        if others:
            skipped.append(f"{m_name} -> {stripped}: '{stripped}' already used in body")
            search_from = match.end()
            continue
        # rename m_name -> stripped within [match.start, end): the for-line var + its body uses
        region: str = text[match.start() : end]
        region_new: str = re.sub(rf"\b{re.escape(m_name)}\b", stripped, region)
        text = text[: match.start()] + region_new + text[end:]
        fixed += 1
        # continue scanning after this (now shorter) region
        search_from = match.start() + len(region_new)
    if fixed:
        path.write_text(text, encoding="utf-8", errors="surrogateescape")
    return fixed, skipped


def main() -> int:
    total: int = 0
    all_skipped: list[str] = []
    for rel in TARGET_FILES:
        path: Path = REPO_ROOT / rel
        fixed, skipped = fix_file(path)
        total += fixed
        print(f"{rel}: fixed {fixed}" + (f", skipped {len(skipped)}" if skipped else ""))
        all_skipped.extend(f"  {rel}: {s}" for s in skipped)
    print(f"=== total renamed: {total}")
    if all_skipped:
        print("=== skipped (hand-fix these):")
        print("\n".join(all_skipped))
    return 0


if __name__ == "__main__":
    sys.exit(main())
