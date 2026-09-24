#!/usr/bin/env python3
"""Ownership flip helper: `for (auto X : <expr>objects)` -> `for (auto& X : <expr>objects)`.

After cObject_Manager's `objects` became vector<unique_ptr<T>>, a by-value range-for tries to
COPY a unique_ptr (deleted). Binding by reference fixes it, and is equally correct for the raw
`vector<T*>` containers (cLayer's borrowed list). This ONLY changes loops whose range expression
ends in `objects`; the remaining raw-pointer USES inside those bodies (static_cast, return,
assignment, passing to a T*-taking function) still need a hand `.get()` -- the compiler pinpoints
each after this runs.

Idempotent: `for (auto& ...` no longer matches `for (auto NAME :`. Run from anywhere.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT: Path = Path(
    subprocess.check_output(["git", "rev-parse", "--show-toplevel"], text=True).strip()
)

# `for (auto NAME : <range ending in objects>)` -- NAME is a bare identifier (not `auto&`,
# not `const auto&`, which already dodge the copy).
LOOP: re.Pattern[str] = re.compile(r"for \(auto (\w+) : ([^)]*\bobjects)\)")


def fix_file(path: Path) -> int:
    text: str = path.read_text(encoding="utf-8", errors="surrogateescape")
    new_text, n = LOOP.subn(r"for (auto& \1 : \2)", text)
    if n:
        path.write_text(new_text, encoding="utf-8", errors="surrogateescape")
    return n


def main() -> int:
    src: Path = REPO_ROOT / "smc" / "src"
    total: int = 0
    for path in sorted(src.rglob("*.cpp")) + sorted(src.rglob("*.h")):
        n: int = fix_file(path)
        if n:
            print(f"{path.relative_to(REPO_ROOT)}: {n}")
            total += n
    print(f"=== total loops converted to auto&: {total}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
