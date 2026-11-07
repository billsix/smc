#!/usr/bin/env python3
"""Phase 2 codemod: the nine cObject_Manager<T> subclasses hold a private Owning_Vector<T>.

The mechanical part of un-deriving: rewrite each class header (drop the base, `override` on the
destructor and Add, the base Get_Pointer re-export; add `Objects()` and the private
`m_objects`), each class .cpp (constructor base initializer, base Add/Delete_All calls, bare
`objects` -> `m_objects`), and the outside range-for loops (`for (const auto& x : e.objects)` ->
`for (auto* x : e.Objects())`). The compiler then lists what is left, which is fixed by hand:
loop bodies that called `.get()`, and the few operations an outside caller used from the base.

Run from anywhere: python3 tasks/adhoc/smc-manager-classes-to-std-containers/underive.py
Idempotent: a second run changes nothing.
"""

import pathlib
import re

SRC: pathlib.Path = pathlib.Path(__file__).resolve().parents[3] / "smc" / "src"

# class name, element type, file stem (relative to smc/src)
CLASSES: list[tuple[str, str, str]] = [
    ("cSprite_Manager", "cSprite", "core/sprite_manager"),
    ("cObjectCollisionType", "cObjectCollision", "core/collision"),
    ("cSound_Manager", "cSound", "audio/sound_manager"),
    ("cCampaign_Manager", "cCampaign", "core/campaign_manager"),
    ("cHud_Manager", "cHudSprite", "gui/hud"),
    ("cLevel_Manager", "cLevel", "level/level_manager"),
    ("cOverworld_Manager", "cOverworld", "overworld/world_manager"),
    ("cAnimation_Manager", "cAnimation", "video/animation"),
    ("cImage_Manager", "cGL_Surface", "video/img_manager"),
]

# cWorld_Sprite_Manager derives from cSprite_Manager, so its Add and destructor stay virtual.
VIRTUAL_KEPT: set[str] = {"cSprite_Manager"}

# `objects` used as a bare identifier in code (not `x.objects`, not the `../objects/` include dir)
BARE_OBJECTS: re.Pattern[str] = re.compile(r"(?<![\w./>])objects\b(?!/)")


def rename_bare_objects(text: str) -> str:
    """Rename bare `objects` to `m_objects` on every line that is not a comment."""

    def fix(line: str) -> str:
        stripped: str = line.lstrip()
        is_comment: bool = stripped.startswith(("//", "*", "/*"))
        return line if is_comment else BARE_OBJECTS.sub("m_objects", line)

    return "".join(fix(line) for line in text.splitlines(keepends=True))


def rewrite_header(text: str, cls: str, elem: str) -> str:
    """Un-derive the class declaration and give it its private storage."""
    head: str = f"class {cls} : public cObject_Manager<{elem}> {{\n public:\n"
    if head not in text:
        return text
    virtual: str = "virtual " if cls in VIRTUAL_KEPT else ""
    accessor: str = (
        f"  // The owned objects, borrowed ({elem}* each), for range-for\n"
        f"  auto Objects(void) const {{ return Borrowed(m_objects); }}\n\n"
    )
    text = text.replace(head, f"class {cls} {{\n public:\n" + accessor)
    text = text.replace(f"  ~{cls}(void) override;", f"  {virtual}~{cls}(void);")
    text = re.sub(
        rf"  {elem}\* Add\((std::unique_ptr<{elem}> \w+)\) override;",
        rf"  {virtual}{elem}* Add(\1);",
        text,
    )
    # the base Get_Pointer(size_t) re-export (and the comment above it) becomes the class's own
    text = re.sub(
        rf"(?:  //[^\n]*\n)*  using cObject_Manager<{elem}>::Get_Pointer;\n",
        f"  // The object at the given index, or nullptr when out of range\n"
        f"  {elem}* Get_Pointer(size_t identifier) const {{\n"
        f"    return Borrow_At(m_objects, identifier);\n"
        f"  }}\n",
        text,
    )
    text = text.replace(
        f"cObject_Manager<{elem}>::Get_Pointer(identifier)",
        "Borrow_At(m_objects, identifier)",
    )
    text = text.replace('#include "../core/obj_manager.h"', '#include "../core/owning_vector.h"')
    # the private storage goes last in the class body
    start: int = text.index(f"class {cls} {{")
    end: int = text.index("\n};", start)
    storage: str = f"\n\n private:\n  Owning_Vector<{elem}> m_objects;"
    if "Owning_Vector<" + elem + "> m_objects;" not in text[start:end]:
        text = text[:end] + storage + text[end:]
    return text


def rewrite_source(text: str, elem: str) -> str:
    """Drop the base from the constructor and route base calls to m_objects."""
    base: str = rf"cObject_Manager<{elem}>\(\)"
    text = re.sub(rf"\s*:\s*{base}\s*,\s*", " : ", text)
    text = re.sub(rf"\s*:\s*{base}\s*\{{", " {", text)
    text = re.sub(
        rf"return cObject_Manager<{elem}>::Add\((std::move\(\w+\))\);",
        r"return m_objects.emplace_back(\1).get();",
        text,
    )
    return text.replace(f"cObject_Manager<{elem}>::Delete_All();", "m_objects.clear();")


# outside loops: `for (const auto& x : <expr>.objects)` / `->objects`, possibly wrapped
OUTSIDE_LOOP: re.Pattern[str] = re.compile(
    r"for \(const auto& (\w+) :(\s+)((?:(?!m_layer)[\w>.\-])+?)(\.|->)objects\)"
)


def rewrite_loops(text: str) -> str:
    return OUTSIDE_LOOP.sub(r"for (auto* \1 :\2\3\4Objects())", text)


def main() -> None:
    for cls, elem, stem in CLASSES:
        header: pathlib.Path = SRC / f"{stem}.h"
        source: pathlib.Path = SRC / f"{stem}.cpp"
        header.write_text(rename_bare_objects(rewrite_header(header.read_text(), cls, elem)))
        source.write_text(rename_bare_objects(rewrite_source(source.read_text(), elem)))
    for path in sorted(SRC.rglob("*.cpp")) + sorted(SRC.rglob("*.h")):
        old: str = path.read_text()
        new: str = rewrite_loops(old)
        if new != old:
            path.write_text(new)


if __name__ == "__main__":
    main()
