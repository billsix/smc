#!/usr/bin/env python3
"""Phase F codemod: observation loops default to `const auto&`.

Mode `apply`: every `for (auto& NAME : EXPR)` becomes `for (const auto& NAME : EXPR)`.
Mode `revert <build-log>`: for each compile error the log reports inside such a loop (the compiler
rejecting a mutation through the const handle -- `discards qualifiers`, `read-only`,
`binding reference`), the nearest enclosing `for (const auto& …)` above the error line is turned
back into `for (auto& …)`. The compiler is the oracle: it decides which handles are mutated.

Both modes are idempotent. Paths are repo-relative; the log is read as given.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SRC = ROOT / "smc" / "src"
APPLY = re.compile(r"for \(auto& (\w+) : ")
ERROR = re.compile(r"^(?:[a-z_./]+/)*([a-z_]+/[a-z_]+\.(?:cpp|h)):(\d+):\d+: error: .*(?:discards qualifiers|read-only|binding reference of type|no match for .operator=|cannot convert .const|use of deleted function)")


def apply() -> None:
    n = 0
    for path in sorted(SRC.rglob("*.cpp")) + sorted(SRC.rglob("*.h")):
        text = path.read_text()
        new, k = APPLY.subn(r"for (const auto& \1 : ", text)
        if k:
            path.write_text(new)
            n += k
    print(f"const auto&: {n} loops converted")


def revert(log: Path) -> None:
    reverted: set[tuple[str, int]] = set()
    for line in log.read_text().split("\n"):
        line = re.sub(r"\x1b\[[0-9;]*m", "", line)
        m = ERROR.match(line)
        if not m:
            continue
        rel, lineno = m.group(1), int(m.group(2))
        path = SRC / rel
        if not path.exists():
            continue
        lines = path.read_text().split("\n")
        # walk up to the enclosing const loop (bounded: a loop body here is never 200 lines)
        for i in range(lineno - 1, max(lineno - 200, -1), -1):
            if "for (const auto& " in lines[i]:
                if (rel, i) not in reverted:
                    lines[i] = lines[i].replace("for (const auto& ", "for (auto& ", 1)
                    reverted.add((rel, i))
                    path.write_text("\n".join(lines))
                break
    for rel, i in sorted(reverted):
        print(f"reverted to auto&: {rel}:{i + 1}")
    print(f"{len(reverted)} loops mutate their handle and stay auto&")


if __name__ == "__main__":
    if len(sys.argv) >= 2 and sys.argv[1] == "apply":
        apply()
    elif len(sys.argv) == 3 and sys.argv[1] == "revert":
        revert(Path(sys.argv[2]))
    else:
        sys.exit("usage: const_loops.py apply | revert <build-log>")
