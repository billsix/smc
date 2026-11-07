#!/usr/bin/env python3
"""Codemod: replace rand() modulo idioms in smc/src with Random_Below (core/math/utilities.h).

Rewrites, in order:
  static_cast<unsigned int>(rand()) % tiles_num          -> Random_Below(tiles_num)
  static_cast<size_t>(rand()) % listbox_levels->getItemCount()
                                                         -> Random_Below(listbox_levels->getItemCount())
  rand() % (EXPR)                                        -> Random_Below(EXPR)
  rand() % NAME_OR_NUMBER                                -> Random_Below(NAME_OR_NUMBER)
Then lists any rand() left (expected: none). Idempotent: a second run changes nothing.

    python3 rand_to_random.py            # from anywhere; finds the repo root itself
"""

import pathlib
import re
import subprocess

ROOT: pathlib.Path = pathlib.Path(
    subprocess.run(["git", "rev-parse", "--show-toplevel"],
                   cwd=pathlib.Path(__file__).parent, capture_output=True,
                   text=True, check=True).stdout.strip())
SRC: pathlib.Path = ROOT / "smc" / "src"

RULES: list[tuple[re.Pattern[str], str]] = [
    (re.compile(r"static_cast<unsigned int>\(rand\(\)\) % tiles_num"), "Random_Below(tiles_num)"),
    (re.compile(r"static_cast<size_t>\(rand\(\)\) %\s*listbox_levels->getItemCount\(\)"),
     "Random_Below(listbox_levels->getItemCount())"),
    (re.compile(r"\brand\(\) % \(([^()]+)\)"), r"Random_Below(\1)"),
    (re.compile(r"\brand\(\) % ([A-Za-z0-9_.]+)"), r"Random_Below(\1)"),
]


def main() -> None:
    changed: int = 0
    for path in sorted(SRC.rglob("*.cpp")) + sorted(SRC.rglob("*.h")):
        text: str = path.read_text()
        new: str = text
        for pattern, replacement in RULES:
            new = pattern.sub(replacement, new)
        if new != text:
            path.write_text(new)
            changed += 1
            print(f"rewrote {path.relative_to(ROOT)}")
    print(f"{changed} file(s) changed")
    for path in sorted(SRC.rglob("*.[ch]*")):
        for number, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
            if re.search(r"\brand\(\)", line):
                print(f"LEFT {path.relative_to(ROOT)}:{number}: {line.strip()}")


if __name__ == "__main__":
    main()
