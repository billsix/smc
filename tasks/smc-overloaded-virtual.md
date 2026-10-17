# smc: resolve -Woverloaded-virtual (269 sites) properly, then re-add it to the gate

**Status:** proposed — needs go-ahead
**Priority:** 6
**Difficulty:** 6 (API-shape decisions on a game hierarchy; needs the maintainer's intent per method)
**Created:** 2026-10-17 (William Emerison Six <billsix@gmail.com>)
**Part of / follows:** `smc-const-sweep-and-warning-gate.md` — that pass dropped `-Woverloaded-virtual`
from the gate (`-Wno-overloaded-virtual` in `smc/configure.ac`) after the mechanical fix broke the
build; this task resolves it and re-enables the flag.

## BLUF

`-Woverloaded-virtual` fired 269 times: a derived class declares a method with the same name as a
base `virtual` but a **different signature**, hiding the base overload. The mechanical fix (add
`using Base::name;` in the derived to un-hide the base) **does not build** — several base methods have
default arguments whose arity overlaps the derived, so un-hiding creates **call ambiguity** (e.g.
`cMovingSprite::Draw(cSurface_Request* = nullptr)` vs `cMouseCursor::Draw()`;
`cMovingSprite::Set_Image(cGL_Surface*, bool = false)` vs `cParticle_Emitter::Set_Image(cGL_Surface*)`;
`cObject_Manager<T>::Delete_All()` vs `cSprite_Manager::Delete_All(bool = false)`). The hiding is
**intentional and load-bearing** — it makes call sites resolve unambiguously to the derived. So each
site needs a per-method **design decision**, not a codemod. "Done" = each hide is resolved (see
options) and `-Woverloaded-virtual` is back in `smc/configure.ac` with the gate green under gcc + clang.

## The relationships (from the 2026-10-17 measurement)

| base declaration (hidden) | hiding derived(s) | overlap? |
|---|---|---|
| `cMovingSprite::Set_Image(cGL_Surface*, bool=false)` | `cParticle_Emitter::Set_Image(cGL_Surface*)` | yes (1-arg) |
| `cObject_Manager<cSprite>::Delete_All()` | `cSprite_Manager::Delete_All(bool=false)` | yes (0-arg) |
| `cMovingSprite::Draw(cSurface_Request*=nullptr)` | `cMouseCursor::Draw()` | yes (0-arg) |
| `cMovingSprite::Copy()` | `cMouseCursor::Copy(const cSprite*, float, float) const` | no (arity differs) |
| `cObject_Manager<T>::Get_Pointer(size_t)` | `cSound_Manager`/`cImage_Manager::Get_Pointer(const std::string&)` | no (type differs) |
| `cMovingSprite::Set_Direction(ObjectDirection, bool)` | enemies + cLevel_Entry/Exit `Set_Direction(ObjectDirection)` | check bool default |
| `cSprite`'s Draw/Copy/Set_Image/Set_Direction | `cMovingSprite` (the mid base, movingsprite.h:71/77/84/179) | — |

## Options per method (the maintainer's intent decides)
1. **No overlap (Get_Pointer, Copy):** `using Base::name;` in the derived is safe and correct — un-hides
   the base overload, no ambiguity. Do this for those.
2. **Overlap via a base default arg (Draw, Set_Image, Delete_All, maybe Set_Direction):** the hide is
   deliberate. Either (a) make the derived a **true override** with the base signature (add the
   defaulted param and ignore it) where that is semantically right, or (b) accept the hide and
   **suppress narrowly** — `#pragma GCC diagnostic ignored "-Woverloaded-virtual"` scoped to that class
   with a one-line reason. Pick per method with the maintainer.
3. Re-add `-Woverloaded-virtual` to `smc/configure.ac` (remove the `-Wno-`), drive to zero, keep `-Werror`.

## Verification
Gate green under clang + gcc with `-Woverloaded-virtual` on; `make image` + `tools/smoke_headless.sh`
pass; no gameplay change. Worklog of the original sites: `tasks/adhoc/smc-const-sweep-and-warning-gate/
data/gate-g.log`.
