#!/usr/bin/env python3
"""Phase C codemod: the collision-list transients become `std::unique_ptr` returns.

`Collision_Check`, `Collision_Check_Relative/Absolute` and `Col_Move_in_Steps` returned a raw
`cObjectCollisionType*` that every caller had to `delete` on every exit path (~30 sites, fourteen
in the level player alone). Now they return `std::unique_ptr<cObjectCollisionType>`:

  * the four signatures (header + definitions) change;
  * every `cObjectCollisionType* v = <factory>(…);` becomes `std::unique_ptr<cObjectCollisionType> v = …;`
    and `cObjectCollisionType* v = nullptr;` becomes `std::unique_ptr<cObjectCollisionType> v;`;
  * every `delete v;` line and `if (v) { delete v; }` block for such a `v` is removed;
  * raw uses of `v` as an argument (`Add_Collisions(v, …)`) become `v.get()`;
  * inside the factories, `new cObjectCollisionType()` becomes `std::make_unique`.

Idempotent: a second run changes nothing. The compiler is the second oracle. Paths are relative
to the repo root (resolved from this file's location).
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SRC = ROOT / "smc" / "src"
TYPE = "cObjectCollisionType"
FACTORIES = r"(?:Collision_Check_Relative|Collision_Check_Absolute|Collision_Check|Col_Move_in_Steps)"

RE_SIG = re.compile(r"^(\s*)" + TYPE + r"\* ((?:cMovingSprite ::)?" + FACTORIES + r")\(", re.M)
RE_DECL = re.compile(r"^(\s*)" + TYPE + r"\* (\w+) =\s*(.*)$")
RE_DECL_NULL = re.compile(r"^(\s*)" + TYPE + r"\* (\w+) = nullptr;\s*$")
RE_NEW = re.compile(TYPE + r"\* (\w+) = new " + TYPE + r"\(\);")


def raw_use_to_get(line: str, name: str) -> str:
    pat = re.compile(r"(?<![\w.>])" + re.escape(name) + r"(?![\w])(?!\s*->)(?!\s*=)")
    out: list[str] = []
    pos = 0
    for m in pat.finditer(line):
        before = line[: m.start()].rstrip()
        after = line[m.end() :].lstrip()
        if before.endswith(("(", ",")) and after.startswith((")", ",")):
            head = before[:-1].rstrip() if before.endswith("(") else ""
            if before.endswith("(") and head.endswith(("if", "while")):
                continue
            out.append(line[pos : m.end()])
            out.append(".get()")
            pos = m.end()
    out.append(line[pos:])
    return "".join(out)


def process(path: Path) -> int:
    text = path.read_text()
    original = text
    text = RE_SIG.sub(r"\1std::unique_ptr<" + TYPE + r"> \2(", text)
    text = RE_NEW.sub(r"auto \1 = std::make_unique<" + TYPE + r">();", text)
    lines = text.split("\n")
    out: list[str | None] = list(lines)
    names: set[str] = set()
    for i, line in enumerate(lines):
        m = RE_DECL.match(line)
        rest = m.group(3) if m else ""
        # a two-line declaration: `cObjectCollisionType* v =` then the factory call on the next line
        if m and rest == "" and i + 1 < len(lines) and re.search(FACTORIES + r"\(", lines[i + 1]):
            rest = "two-line Collision_Check"
        if m and ("Collision_Check" in rest or "Col_Move_in_Steps" in rest):
            out[i] = f"{m.group(1)}std::unique_ptr<{TYPE}> {m.group(2)} = {m.group(3)}"
            names.add(m.group(2))
        elif RE_DECL_NULL.match(line):
            mn = RE_DECL_NULL.match(line)
            out[i] = f"{mn.group(1)}std::unique_ptr<{TYPE}> {mn.group(2)};"
            names.add(mn.group(2))
    for name in names:
        del_re = re.compile(r"^\s*delete " + re.escape(name) + r";\s*$")
        if_re = re.compile(r"^\s*if \(" + re.escape(name) + r"\) \{\s*$")
        for i in range(len(out)):
            l = out[i]
            if l is None:
                continue
            if del_re.match(l):
                # `if (name) {` / `delete name;` / `}` -> drop the whole block
                if i >= 1 and out[i - 1] is not None and if_re.match(out[i - 1]) and i + 1 < len(out) and out[i + 1] is not None and out[i + 1].strip() == "}":
                    out[i - 1] = None
                    out[i + 1] = None
                out[i] = None
                continue
            if re.search(r"(?<![\w.>])" + re.escape(name) + r"(?![\w])", l) and "std::unique_ptr" not in l and "delete " not in l:
                out[i] = raw_use_to_get(l, name)
    text = "\n".join(l for l in out if l is not None)
    if text != original:
        path.write_text(text)
        return 1
    return 0


def main() -> int:
    changed = [str(p.relative_to(ROOT)) for p in sorted(SRC.rglob("*.[ch]pp")) if process(p)]
    for h in sorted(SRC.rglob("*.h")):
        if process(h):
            changed.append(str(h.relative_to(ROOT)))
    print(f"changed {len(changed)} files:")
    for c in changed:
        print("  " + c)
    return 0


if __name__ == "__main__":
    sys.exit(main())
