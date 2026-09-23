# Modernize the game's C++ to the newest standard, replacing the old patterns with the features made for them

**Status:** proposed — needs go-ahead (asked for by William Emerison Six <billsix@gmail.com>,
2026-09-23: "modernize the c++ to the newest available version, using features intended to
replace older patterns. If I remember correctly, too many manager classes. This would be an
excellent example of refactoring to modern C++"). Not to be started until the container build
task is green (it is the gate this work runs under).
**Priority:** 4 · **Difficulty:** 8 (77 `.cpp` / 85 `.h`, a 6-deep sprite hierarchy dispatched by
type tags, 35 global raw pointers; each step must keep the game playable)
**Depends on:** the container build (done: `tasks/archive/2026/09/23/smc-container-build.md`); `make
image` + `tools/smoke_headless.sh` are the regression check for every step here.

## BLUF

Take the game from "C++98 that happens to compile under GCC's default" to C++23 as written: a
pinned `-std`, ownership in `unique_ptr`, the twelve hand-rolled manager classes collapsed onto
standard containers and one owning application object, Boost dropped for `<filesystem>`/
`<thread>`, and the type-tag dispatch turned into virtual functions. Done = every step is its own
small commit that builds green in the container and passes the headless smoke run; the final
tree has no raw owning `new`/`delete`, no `NULL`, no Boost link dependency, `override` on every
overrider, and a reference doc that reads as a worked example of "refactoring to modern C++".

## Context

- Read first: `tasks/reference/building-2010-era-smc-today.md` (how the tree builds now, the SDL 2
  port), then `smc/src/core/obj_manager.h` (the template every manager is built from),
  `smc/src/core/main.cpp:250-506` (the hand-ordered `new`/`delete` of every singleton), and
  `smc/src/core/global_game.h` (`SpriteType`, `ArrayType`, the `#define` constants).
- **The standard today is accidental.** `smc/configure.ac` sets only `-Wall`; no `-std=` anywhere,
  so the code is C++98/03 compiled under whatever GCC defaults to (gnu++17 on GCC 11+). Step 0
  is pinning it (`AX_CXX_COMPILE_STDCXX([23], [noext], [mandatory])` or `-std=c++23`).
- **The managers** (survey 2026-09-23, counts from `rg` in `smc/src`): one template base
  `cObject_Manager<T>` (`core/obj_manager.h:27`, an owning `vector<T*>` with manual `delete`,
  and two real bugs — `Delete(size_t)` erases then deletes the shifted element, and
  `Switch_Array_Num` assigns to an iterator under a `// todo: does that work ?`), and twelve
  concrete managers: `cResource_Manager`, `cFont_Manager`, `cImage_Manager`, `cSound_Manager`,
  `cSprite_Manager` (+ `cWorld_Sprite_Manager`), `cBackground_Manager`, `cAnimation_Manager`,
  `cHud_Manager`, `cLevel_Manager`, `cOverworld_Manager`, `cCampaign_Manager`. Nine are
  process-wide singletons behind `extern cX *pX` globals, `new`ed in `main.cpp:250-338` and
  `delete`d in `main.cpp:358-506` in an order that already disagrees with construction. Eight of
  the twelve are typed instantiations of the template — they collapse to
  `std::vector<std::unique_ptr<T>>` members on their real owner.
- **Global raw pointers:** 35 `extern cX *pX` (`pVideo`, `pAudio`, `pPreferences`, `pKeyboard`,
  `pMouseCursor`, `pRenderer`, `pActive_Level`, `pActive_Player`, six `pHud_*`, …).
- **Old-pattern counts** (`rg`, 2026-09-23): raw `new` 621, `delete` 195 (+10 `delete[]`);
  `NULL` 487; `typedef` 40 (25 are `typedef vector<cX*> XList`); explicit-iterator `for` loops
  217 (23 `delete *itr` inside); `virtual` 688 vs `override` 0 (`final` 0); unscoped `enum` 36
  (`enum class` 0; `SpriteType` has 58 `TYPE_`-prefixed enumerators, dispatched by 21
  `switch( m_type )` sites and 90 tag-guarded `static_cast<cX *>` downcasts); `#define`
  constants 37; int-as-bool (`bool x = 0`, `return 1;` in bool functions) ~350; 38 `virtual cX
  *Copy() const` clone methods returning raw owning pointers; `sprintf` into `char[]` 3
  (`gui/hud.cpp:442,657,673`); home-grown `int_to_string`/`float_to_string` + `ostringstream`
  277; CEGUI `subscribeEvent` 216, all fire-and-forget (connections never stored or
  disconnected — a dangling-`this` risk). No smart pointers anywhere; no `auto_ptr`, no dynamic
  exception specs, C-style pointer casts 3.
- **Boost** is three link-time dependencies (`configure.ac:22-30`: filesystem, thread, system)
  for **nine call sites**: `boost::algorithm` ×5 (`core/property_helper.h`), `boost::thread` ×3
  (`video/video.{h,cpp}`), `boost::filesystem` ×1 (`core/filesystem/filesystem.cpp`) — all
  replaceable by `<filesystem>`, `<thread>` and three lines of string trim/case.
- **Object model:** `cCollidingSprite` → `cSprite` → `cMovingSprite` → `cAnimated_Sprite` →
  `cEnemy` → concrete (depth 5–6); 96 derived classes, 25 directly off a sprite base. No
  `Get_Type()`; dispatch is on the public data members `m_type` (349 uses) and `m_sprite_array`.
- The SDL 2 port (commit `d438165`; its script: `git show d438165:tasks/adhoc/container-build/patch_sdl2.py`) is already applied and must be kept
  as-is underneath — this task starts from that tree.

## Goal

A tree that a reader can diff step by step as a tutorial in modern C++: each commit replaces one
old pattern with the language/library feature designed to replace it, says so in its message,
and leaves the game playable. The end state has one owning application object instead of the
singleton managers, standard containers instead of `cObject_Manager<T>`, and the standard
library instead of Boost.

## Plan (ordered; each step = one or a few commits, container build + smoke run green after each)

- [ ] **0. Pin the standard.** C++23 in `configure.ac`; fix whatever stops compiling; add
      `-Wall -Wextra` and note the warning count as the progress metric.
- [ ] **1. `override`/`final` sweep** (mechanical, 688 `virtual`s; expect it to surface
      signature mismatches in the sprite hierarchy — each one is a real bug, fix and note it).
- [ ] **2. `NULL` → `nullptr`, int-as-bool → `true`/`false`, `typedef` → `using`, `#define`
      constants → `constexpr`/`inline constexpr std::string_view`** (mechanical; a codemod under
      `tasks/adhoc/modernize-cpp/`, idempotent and proven).
- [ ] **3. Drop Boost:** `<filesystem>`, `<thread>`, string helpers; remove the three
      `AC_CHECK_LIB`s and `boost-devel` from `entrypoint/01-install-base.sh` (and from the two
      sandbox base lists only if nothing else there needs it).
- [ ] **4. Ownership:** `cObject_Manager<T>` → `std::vector<std::unique_ptr<T>>` (fixes the two
      bugs); the 38 `Copy()` clones → `std::unique_ptr<cSprite> Clone() const override`; the 23
      delete-inside loops disappear; every remaining owning `new`/`delete` → `make_unique`.
- [ ] **5. The managers and globals:** one `cGame`/application object owning the former
      singletons as `unique_ptr` members in declaration order (teardown order guaranteed); the
      eight container-managers become plain members of their owner; the 35 `extern` globals go
      through it (or become references passed down where the call graph is shallow).
- [ ] **6. Enums and dispatch:** `SpriteType`/`ArrayType` → `enum class` (drop the `TYPE_`
      prefixes); then the 21 `switch( m_type )` sites and 90 downcasts → virtual functions (or
      `std::variant` where the set is closed).
- [ ] **7. Loops and strings:** 217 iterator loops → range-for/`<algorithm>`/`<ranges>`;
      `int_to_string`/`ostringstream`/`sprintf` → `std::to_string`/`std::format`.
- [ ] **8. CEGUI events:** lambdas instead of `Event::Subscriber(&cX::f, this)`; keep the
      `Event::Connection`s and disconnect in the destructor.
- [ ] **9.** Reference doc `tasks/reference/refactoring-to-modern-cpp.md` — the worked example:
      per step, the before/after pattern, the feature that replaced it, what it caught.

## Notes / decisions

- Order matters: mechanical sweeps (0–3) first, so the ownership/manager rewrite (4–5) is done
  on a tree where the compiler already checks `override` and the standard is pinned.
- Upstream is dead (last commit 2014, the successor is github.com/secretchronicles/TSC), so
  there is no upstream to keep patches small for — this task may rewrite freely, one pattern per
  commit for the tutorial value.
- Keep the SDL 2 port's `cKey_State` adapter and `typedef SDL_Keycode SDLKey` until step 2, then
  fold the typedef into `using`.

- **Decided (William Emerison Six <billsix@gmail.com>, 2026-09-23):** the standard is **C++23**;
  step 5 uses **one application object** owning every former singleton (the agent's
  recommendation, taken with "whatever you recommend") — simplest to explain in the tutorial doc,
  and destruction order falls out of member declaration order.

## Open questions

None — the two above are decided. Still needs the maintainer's go-ahead to start.
