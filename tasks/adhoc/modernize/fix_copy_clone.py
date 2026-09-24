#!/usr/bin/env python3
"""Step 5.4: cSprite-hierarchy Copy() returns std::unique_ptr<cSprite> (was a covariant raw T*).

Rewrites the 34 Copy() signatures (impl + header decl) across the cSprite hierarchy. The BODIES
(the `new Type(...)` and any raw uses) are fixed compiler-driven afterwards. cGL_Surface::Copy is
EXCLUDED -- cGL_Surface is a texture, not a cSprite, and its Copy is unrelated.

Idempotent: an already-`std::unique_ptr<cSprite>`-returning signature no longer matches.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT: Path = Path(
    subprocess.check_output(["git", "rev-parse", "--show-toplevel"], text=True).strip()
)
SRC: Path = REPO_ROOT / "smc" / "src"
EXCLUDE: set[str] = {"gl_surface.cpp", "gl_surface.h"}  # cGL_Surface is not a cSprite

# impl:  <Type>* <Cls> ::Copy(void) const {
IMPL: re.Pattern[str] = re.compile(
    r"^[\w:]+\s*\*\s*(\w+)\s*::Copy\(void\) const \{", re.MULTILINE
)
# header decl:  <Type>* Copy(void) const [override];
DECL: re.Pattern[str] = re.compile(
    r"^(\s*)[\w:]+\s*\*\s*Copy\(void\) const( override)?;", re.MULTILINE
)


def fix_file(path: Path) -> int:
    text: str = path.read_text(encoding="utf-8", errors="surrogateescape")
    n: int = 0
    if path.suffix == ".cpp":
        text, k = IMPL.subn(r"std::unique_ptr<cSprite> \1 ::Copy(void) const {", text)
        n += k
    else:
        text, k = DECL.subn(
            r"\1std::unique_ptr<cSprite> Copy(void) const\2;", text
        )
        n += k
    if n:
        path.write_text(text, encoding="utf-8", errors="surrogateescape")
    return n


def main() -> int:
    total: int = 0
    for path in sorted(SRC.rglob("*.cpp")) + sorted(SRC.rglob("*.h")):
        if path.name in EXCLUDE:
            continue
        k: int = fix_file(path)
        if k:
            print(f"{path.relative_to(REPO_ROOT)}: {k}")
            total += k
    print(f"=== total Copy signatures rewritten: {total}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
