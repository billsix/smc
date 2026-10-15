#!/usr/bin/env python3
"""Phase A codemod: make ownership transfer flow through `std::unique_ptr`.

Rewrites the mechanical majority of raw-`new` + `Add(raw)` sites under `smc/src` so that the
object is created with `std::make_unique` and handed over with `std::move`:

  1. inline sinks          `mgr->Add(new cX(args))`           -> `mgr->Add(std::make_unique<cX>(args))`
  2. local-then-sink       `cX* v = new cX(args); ... v->Set(); ... mgr->Add(v);`
                            -> `auto v = std::make_unique<cX>(args); ... mgr->Add(std::move(v));`
                           also the `cX* v = nullptr; ... v = new cY(...)` reassignment shape, the
                           `return std::unique_ptr<cSprite>(v);` ending of every `Copy()`, and a
                           plain `return v;` (only legal once the factory returns a unique_ptr --
                           the compiler flags the rest). A `delete v;` on an error path before the
                           sink is dropped (the unique_ptr frees on return). Raw uses of `v` between
                           creation and the sink (`f(v)`, `x = v`) become `v.get()`.
                           Any use of `v` AFTER the sink is a use-after-move: the site is left
                           untouched and reported for a hand edit.
  3. create-if-null draws  `bool create_request = false; if (!request) { ...; request = new cT(); }`
                            ... `if (create_request) { pRenderer->Add(request); }`
                            -> an `owned_request` unique_ptr that is moved into the renderer.
  4. XML factories         `return new cX(args);` -> `return std::make_unique<cX>(args);`
                           (only in the two factory files; their return types are changed by hand).

Idempotent: a second run finds no raw `new` of these shapes and changes nothing. The compiler
is the second oracle (signatures of the managers' `Add` are changed by hand in the same phase).
Paths are relative to the repo root (resolved from this file's location).
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SRC = ROOT / "smc" / "src"
FACTORY_FILES = {SRC / "level" / "level.cpp", SRC / "overworld" / "overworld.cpp"}

CLASS = r"c[A-Za-z_0-9]+"
NAME = r"[A-Za-z_][A-Za-z_0-9]*"

# 1. inline `Add(new cX(`
RE_INLINE = re.compile(r"(Add\(\s*)new (" + CLASS + r")\(")
# 2. declarations
RE_DECL_NEW = re.compile(r"^(\s*)(" + CLASS + r")\*\s+(" + NAME + r")\s*=\s*new (" + CLASS + r")\(")
RE_DECL_NEW_2LINE = re.compile(r"^(\s*)(" + CLASS + r")\*\s+(" + NAME + r")\s*=\s*$")
RE_NEXT_NEW = re.compile(r"^(\s*)new (" + CLASS + r")\(")
RE_DECL_NULL = re.compile(r"^(\s*)(" + CLASS + r")\*\s+(" + NAME + r")\s*=\s*nullptr;\s*$")
# 3. create-if-null draw pattern
RE_CREATE = re.compile(
    r"(?P<ind>[ \t]*)bool create_request = false;\n\n"
    r"(?P=ind)if \(!request\) \{\n"
    r"(?P=ind)  create_request = true;\n"
    r"(?P=ind)  // create request\n"
    r"(?P=ind)  request = new (?P<cls>" + CLASS + r")\(\);\n"
    r"(?P=ind)\}\n"
)
RE_CREATE_ADD = re.compile(
    r"(?P<ind>[ \t]*)if \(create_request\) \{\n"
    r"(?P=ind)  // add request\n"
    r"(?P=ind)  pRenderer->Add\(request\);\n"
    r"(?P=ind)\}\n"
)
# 4. factory returns
RE_RETURN_NEW = re.compile(r"return new (" + CLASS + r")\(")

manual: list[str] = []

# Sites deliberately left as raw owners for a later phase (file, variable, declared type):
# their sink is a raw-owning list or a raw-returning factory that the member/transient phases
# convert. Listed here so a re-run of this script on the finished tree changes nothing.
# locals pushed into NON-owning view vectors (cSprite_List etc.) -- leave raw
VIEW_LOCALS = {"new_object"}

SKIP = {
    ("smc/src/video/gl_surface.cpp", "new_surface", "cGL_Surface"),  # cGL_Surface::Copy -> raw owner (members)
    ("smc/src/objects/movingsprite.cpp", "col_list", "cObjectCollisionType"),  # transient-owners phase
}


def function_ranges(lines: list[str]) -> list[tuple[int, int]]:
    """Approximate top-level function bodies as (first_line, closing_brace_line) index pairs."""
    ranges: list[tuple[int, int]] = []
    prev_close = -1
    for i, line in enumerate(lines):
        if line.startswith("}"):
            start = None
            for j in range(prev_close + 1, i):
                l = lines[j]
                if l and not l[0].isspace() and "(" in l and not l.startswith(("#", "//", "/*", "*", "namespace", "using", "class", "struct", "}")):
                    start = j
                    break
            if start is not None:
                ranges.append((start, i))
            prev_close = i
    return ranges


def raw_use_to_get(line: str, name: str) -> str:
    """Turn `(name)`, `, name)`, `(name,`, `= name;` into `.get()` forms; leave `name->`, `!name`, `if (name)`."""
    pat = re.compile(r"(?<![\w.>])" + re.escape(name) + r"(?![\w])(?!\s*->)(?!\s*=)")
    out = []
    pos = 0
    for m in pat.finditer(line):
        before = line[: m.start()].rstrip()
        after = line[m.end() :].lstrip()
        # argument / assignment position only; `if (name)` / `while (name)` stay as they are
        if before.endswith(("(", ",", "=")) and after.startswith((")", ",", ";")):
            head = before[:-1].rstrip() if before.endswith("(") else ""
            if before.endswith("(") and head.endswith(("if", "while")):
                continue
            out.append(line[pos : m.end()])
            out.append(".get()")
            pos = m.end()
    out.append(line[pos:])
    return "".join(out)


def transform_function(lines: list[str], start: int, end: int, fname: str) -> int:
    """Rewrite local-then-sink shapes inside one function body. Returns the number of sites changed."""
    changed = 0
    i = start
    while i <= end:
        line = lines[i]
        m = RE_DECL_NEW.match(line)
        two_line = False
        null_decl = False
        if not m:
            m2 = RE_DECL_NEW_2LINE.match(line)
            if m2 and i + 1 <= end and RE_NEXT_NEW.match(lines[i + 1]):
                m = m2
                two_line = True
            else:
                m3 = RE_DECL_NULL.match(line)
                if m3:
                    m = m3
                    null_decl = True
        if not m:
            i += 1
            continue
        indent, decl_type, name = m.group(1), m.group(2), m.group(3)
        if "Copy()" in line or "static_cast" in line or (fname, name, decl_type) in SKIP or name in VIEW_LOCALS:
            i += 1
            continue
        use_re = re.compile(r"(?<![\w.>])" + re.escape(name) + r"(?![\w])")
        reassign_re = re.compile(r"^\s*" + re.escape(name) + r"\s*=\s*(new (" + CLASS + r")\(|nullptr;)")
        # sinks: the managers' Add, the editor's Add_Item_Object, the menu handler's Add_Menu_Item,
        # and push_back into an owning vector (phase D: the vectors hold unique_ptr)
        sink_add_re = re.compile(r"(?:Add|Add_Item_Object|Add_Menu_Item|push_back)\(\s*" + re.escape(name) + r"\s*[,)]")
        sink_ret_re = re.compile(r"^\s*return " + re.escape(name) + r";\s*$")
        sink_copy_re = re.compile(r"^(\s*)return std::unique_ptr<cSprite>\(" + re.escape(name) + r"\);\s*$")
        delete_re = re.compile(r"^\s*delete " + re.escape(name) + r";\s*$")

        # walk the function and classify every use
        first = i + 2 if two_line else i + 1
        state = "live" if not null_decl else "null"
        edits: list[tuple[int, str | None]] = []  # (line index, replacement or None=delete line)
        ok = True
        reason = ""
        any_sink = False
        for j in range(first, end + 1):
            l = lines[j]
            code = l.split("//", 1)[0]  # a mention inside a trailing comment is not a use
            if not use_re.search(code):
                continue
            # a fresh declaration of the same name (a sibling or nested block) ends this one's segment
            dm = RE_DECL_NEW.match(code) or RE_DECL_NEW_2LINE.match(code) or RE_DECL_NULL.match(code)
            if (dm and dm.group(3) == name) or re.match(r"^\s*auto " + re.escape(name) + r"\s*=", code):
                break
            if reassign_re.match(l):
                rm = reassign_re.match(l)
                if rm.group(1).startswith("new"):
                    edits.append((j, re.sub(r"=\s*new (" + CLASS + r")\(", r"= std::make_unique<\1>(", l, count=1)))
                    state = "live"
                else:
                    state = "null"
                continue
            if state == "moved":
                ok = False
                reason = f"use after the sink at line {j + 1}: {l.strip()[:70]}"
                break
            if state == "null" and not sink_add_re.search(l) and not sink_ret_re.match(l):
                # a use while the pointer is known-null (e.g. `if (!anim)`) -- leave it; it compiles
                edits.append((j, raw_use_to_get(l, name)))
                continue
            if delete_re.match(l):
                edits.append((j, None))
                continue
            if sink_add_re.search(l):
                edits.append((j, re.sub(r"(Add|Add_Item_Object|Add_Menu_Item|push_back)\(\s*" + re.escape(name) + r"\s*([,)])", r"\1(std::move(" + name + r")\2", l, count=1)))
                state = "moved"
                any_sink = True
                continue
            cm = sink_copy_re.match(l)
            if cm:
                edits.append((j, f"{cm.group(1)}return {name};"))
                state = "moved"
                any_sink = True
                continue
            if sink_ret_re.match(l):
                state = "moved"
                any_sink = True
                continue
            # ordinary live use
            edits.append((j, raw_use_to_get(l, name)))
        # a `cX* v = nullptr;` declaration only becomes a unique_ptr when a `v = new cY(...)`
        # assignment follows; otherwise it is a plain borrowed pointer (a search result, an alias)
        has_new_assignment = any(rep is not None and "std::make_unique" in rep for _, rep in edits)
        if null_decl and not has_new_assignment:
            i += 1
            continue
        if not ok or not any_sink:
            manual.append(f"{fname}:{i + 1}: {name} ({decl_type}) -- {reason or 'no Add/return sink found'}")
            i += 1
            continue
        # apply: declaration first
        if null_decl:
            lines[i] = f"{indent}std::unique_ptr<{decl_type}> {name};"
        elif two_line:
            lines[i] = f"{indent}auto {name} ="
            lines[i + 1] = re.sub(r"new (" + CLASS + r")\(", r"std::make_unique<\1>(", lines[i + 1], count=1)
        else:
            lines[i] = re.sub(r"^(\s*)" + CLASS + r"\*\s+" + re.escape(name) + r"\s*=\s*new (" + CLASS + r")\(",
                              r"\1auto " + name + r" = std::make_unique<\2>(", line, count=1)
        for j, rep in edits:
            lines[j] = rep if rep is not None else None  # type: ignore[call-overload]
        changed += 1
        i += 1
    return changed


def process(path: Path) -> tuple[int, int, int, int]:
    text = path.read_text()
    original = text
    n_inline = len(RE_INLINE.findall(text))
    text = RE_INLINE.sub(r"\1std::make_unique<\2>(", text)
    n_create = len(RE_CREATE.findall(text))
    text = RE_CREATE.sub(
        lambda m: (
            f"{m.group('ind')}// no request given: make one here and hand it to the renderer at the end\n"
            f"{m.group('ind')}std::unique_ptr<{m.group('cls')}> owned_request;\n"
            f"{m.group('ind')}if (!request) {{\n"
            f"{m.group('ind')}  owned_request = std::make_unique<{m.group('cls')}>();\n"
            f"{m.group('ind')}  request = owned_request.get();\n"
            f"{m.group('ind')}}}\n"
        ),
        text,
    )
    text = RE_CREATE_ADD.sub(
        lambda m: f"{m.group('ind')}if (owned_request) {{\n{m.group('ind')}  pRenderer->Add(std::move(owned_request));\n{m.group('ind')}}}\n",
        text,
    )
    n_ret = 0
    if path in FACTORY_FILES:
        n_ret = len(RE_RETURN_NEW.findall(text))
        text = RE_RETURN_NEW.sub(r"return std::make_unique<\1>(", text)
    lines: list[str | None] = text.split("\n")  # type: ignore[assignment]
    n_local = 0
    for start, end in function_ranges(lines):  # type: ignore[arg-type]
        n_local += transform_function(lines, start, end, str(path.relative_to(ROOT)))  # type: ignore[arg-type]
    text = "\n".join(l for l in lines if l is not None)
    if text != original:
        path.write_text(text)
    return n_inline, n_local, n_create, n_ret


def main() -> int:
    totals = [0, 0, 0, 0]
    for path in sorted(SRC.rglob("*.cpp")):
        counts = process(path)
        if any(counts):
            print(f"{path.relative_to(ROOT)}: inline={counts[0]} local={counts[1]} create_request={counts[2]} return_new={counts[3]}")
        totals = [a + b for a, b in zip(totals, counts)]
    print(f"TOTAL inline={totals[0]} local={totals[1]} create_request={totals[2]} return_new={totals[3]}")
    if manual:
        print(f"\nMANUAL ({len(manual)} sites left untouched):")
        for m in manual:
            print("  " + m)
    return 0


if __name__ == "__main__":
    sys.exit(main())
