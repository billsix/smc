#!/usr/bin/env python3
"""Step 5.4 (part 2): wrap the `return <var>;` in each cSprite-hierarchy Copy() body so a raw
local converts to the new std::unique_ptr<cSprite> return type.

The signatures were already changed to return unique_ptr<cSprite> (fix_copy_clone.py). Each body
still builds a raw `Type* v = new Type(...)` and ends `return v;`. Rather than retype the local
(which would need .get() at every raw use in the body), we wrap only the return:
`return v;` -> `return std::unique_ptr<cSprite>(v);`. The `new` is owned by the unique_ptr the
instant it returns -- no raw owning pointer escapes. `return nullptr;` is left alone (it already
converts). Scoped to each Copy method's brace block; idempotent (an already-wrapped return does
not match `return <ident>;`).
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

SIG: re.Pattern[str] = re.compile(
    r"std::unique_ptr<cSprite>\s+\w+\s*::Copy\(void\) const \{"
)
# a bare `return <identifier>;` (not nullptr / a call / a cast)
RET: re.Pattern[str] = re.compile(r"\breturn (?!nullptr\b)([A-Za-z_]\w*);")


def block_end(text: str, open_brace: int) -> int:
    depth = 0
    for i in range(open_brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return i + 1
    raise ValueError("unbalanced braces")


def fix_file(path: Path) -> int:
    text: str = path.read_text(encoding="utf-8", errors="surrogateescape")
    n = 0
    pos = 0
    while True:
        m = SIG.search(text, pos)
        if not m:
            break
        brace = text.index("{", m.end() - 1)
        end = block_end(text, brace)
        body = text[brace:end]
        new_body, k = RET.subn(r"return std::unique_ptr<cSprite>(\1);", body)
        if k:
            text = text[:brace] + new_body + text[end:]
            n += k
            pos = brace + len(new_body)
        else:
            pos = end
    if n:
        path.write_text(text, encoding="utf-8", errors="surrogateescape")
    return n


def main() -> int:
    total = 0
    for path in sorted(SRC.rglob("*.cpp")):
        k = fix_file(path)
        if k:
            print(f"{path.relative_to(REPO_ROOT)}: {k}")
            total += k
    print(f"=== total Copy returns wrapped: {total}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
