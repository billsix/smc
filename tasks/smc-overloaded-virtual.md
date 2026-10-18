# smc: re-enable `-Woverloaded-virtual` (269 sites)

**Status:** DONE 2026-10-18 (work committed by the maintainer; this doc is the retrospective).
**Priority:** 6 · **Difficulty:** 6 · **Created:** 2026-10-17 (William Emerison Six <billsix@gmail.com>)
**Follows:** `tasks/archive/2026/10/18/smc-const-sweep-and-warning-gate.md`, which dropped
`-Woverloaded-virtual` (the mechanical `using` fix broke the build) and deferred it here.

## BLUF

The 269 warnings reduced to **7 base/derived relationships** where a derived class declares a method
with a base virtual's name but a different signature, hiding it. They split by whether un-hiding is
ambiguous. Decisions (2026-10-18): un-hide the no-ambiguity ones with `using`; for the intentional,
load-bearing hides, **suppress with a reason** (not make true overrides — that changes virtual
dispatch and wants a play-test). Done = `-Woverloaded-virtual` back in the gate, green under gcc **and**
clang with `-Werror`, `make image` + smoke pass. Achieved.

## What was done

- **Group 1 — un-hidden with `using` (no ambiguity):**
  - `cObject_Manager<T>::Get_Pointer(size_t)` vs derived `Get_Pointer(const std::string&)` —
    `using` in `cSound_Manager`, `cImage_Manager` (different param types).
  - `cMovingSprite::Copy(void)` vs `cMouseCursor::Copy(const cSprite*, float, float)` —
    `using cMovingSprite::Copy;` (0-arg vs 3-arg).

- **Group 2 — intentional hides, suppressed with a reason.** In each, the base overload has a
  **default argument** overlapping the derived, so un-hiding via `using` is ambiguous and does not
  build (this is what broke the gate pass). The hides are deliberate narrowings. Relationships:
  `cMovingSprite::Set_Direction(ObjectDirection, bool=false)` ← ~10 enemy/level classes;
  `cMovingSprite::Draw(cSurface_Request*=nullptr)` ← `cMouseCursor::Draw()`;
  `cMovingSprite::Set_Image(cGL_Surface*, bool=false)` ← `cParticle_Emitter::Set_Image(cGL_Surface*)`;
  `cObject_Manager::Delete_All()` ← `cSprite_Manager::Delete_All(bool=false)`.

- **The key finding: gcc and clang attribute `-Woverloaded-virtual` to different locations.** gcc
  reports it at the **base** method; clang at the **derived** class. A pragma only suppresses the
  diagnostic at the location it is attributed to, so to be green under *both* the suppression has to
  live in **both** places: `#pragma GCC diagnostic ignored "-Woverloaded-virtual"` on the **4 base
  methods** (`movingsprite.h` Draw/Set_Image/Set_Direction, `obj_manager.h` Delete_All — covers all
  hiders for gcc) **and** on the **13 derived declarations** (clang). 17 reasoned sites total. The
  maintainer accepted the 17-site cost to keep the flag on (it now catches future accidental hides).

- **Re-enabled the flag:** removed `-Wno-overloaded-virtual` from `smc/configure.ac`.

## Verification

gcc `-Werror` = 0, clang `-Werror` = 0, `make image` builds, `make format` applied,
`tools/smoke_headless.sh` passes (ran to the 30 s deadline, ps audit clean). No gameplay change
(the smoke run hard-kills before `Exit_Game`). The gcc-vs-clang attribution finding is worth harvesting
into `cpp-build-modernization.md` if this comes up again.
