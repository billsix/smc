# Apply the updated C++ advice to smc — Phase 0 (doc) + the raised `-Werror` warning gate

**Status:** DONE 2026-10-17 (work committed by the maintainer; this doc is the retrospective).
**Priority:** 5 · **Difficulty:** 6 · **Created:** 2026-10-17 (William Emerison Six <billsix@gmail.com>)
**Decisions (2026-10-17):** full fleet warning set + `-Werror`; run on `master` (no branch); gate must
be green under **both gcc and clang**.
**Siblings:** the const-correctness sweep (Phase 1) was split into
`smc-const-double-const-sweep.md` (done) and `-Wsign-conversion` into `smc-sign-conversion-sweep.md`
(done); `-Woverloaded-virtual` was deferred to `tasks/archive/2026/10/18/smc-overloaded-virtual.md` (done 2026-10-18).

## BLUF

"Do for smc the same thing gltron/apue/Craft got this session": raise the compiler warning set to the
fleet standard + `-Werror`, and apply the const advice. smc already had a general C++ coding-standard
pass on 2026-10-15 (`tasks/archive/2026/10/15/smc-apply-cpp-coding-standard.md`, ownership-focused), so
this task was the **delta**: Phase 0 (mirror the double-`const` idiom into the shared
`cpp-coding-standard.md`) and the **warning gate**. Done = the raised gate green under gcc **and** clang
with `-Werror`, `make image` builds, `tools/smoke_headless.sh` passes, no gameplay change; `third_party/`
(CEGUI) untouched. All of that shipped; the const pass and the two noisy classes became their own tasks.

## Context (what it built on)

- smc was C++23 with only `-Wall -Wextra`; `.clang-format` google (`make format`); whole-tree reformat
  in `.git-blame-ignore-revs`. Error strategy fixed (exceptions only at startup / the CEGUI boundary —
  do not add exception machinery). Verification is nested `make image` + `tools/smoke_headless.sh`
  (hard-kill + `ps` audit); a warning/const sweep is compile-time, so the smoke run is sufficient proof.
- Method mirrored the C sweeps (gltron `tasks/archive/2026/10/17/c-modernization-sweep.md`): class by
  class, behaviour-preserving, fresh per-directory Edit-only subagents for the mechanical bulk
  (git/build forbidden), the coordinator driving the `-Werror` cascade and building after each wave.

## Phase 0 — mirror the double-`const` idiom into the C++ standard (DONE)

The pointee-/top-level-`const` independence guidance lived only in the **C** standard
(`c-coding-standard.md`). It is language-agnostic for pointers, so it was added to
`cpp-coding-standard.md` — canonical in `github.com/billsix/runClaudeInContainer`, mirrored twice in
`github.com/billsix/runCrushInContainer` (incl. the baked Crush-client copy), kept byte-identical.

## Phase 2 — the warning gate (DONE, the bulk of the work)

Chronology, including the dead ends, because the dead ends are the lesson:

1. **First measurement lied: 4368 gcc warnings.** The raw count was dominated by third-party headers.
   **Flipping SDL/CEGUI from `-I` to `-isystem` in `smc/configure.ac` dropped it to 594
   genuinely-first-party** — and corrected a wrong premise: the 1018 `-Wdeprecated-copy` were **not**
   smc's (≈923 were CEGUI's own `UDim` etc.), and all 142 `-Wdeprecated-declarations` were CEGUI's
   `CEGUIString.h`. Both vanished with `-isystem`; rule-of-five applied only to the **95** that were
   smc's own types.
2. **First-party fixes applied** (fanned out one Edit-only subagent per directory, integrated and
   built centrally):
   - `-Wdeprecated-copy` (95): rule-of-five `= default` on the three value classes `Color`,
     `GL_rect`, `GL_point`.
   - `-Wshadow`: renamed the shadowing locals/params (chiefly the `red`/`green`/`blue`-shadow sites in
     `objects/sprite.{h,cpp}`, plus `gui/hud.cpp`, `video/animation.cpp`).
   - small classes: `-Wsign-compare` boundary casts (`audio.cpp`, `video.cpp`, `world_layer.cpp`);
     `-Wunused-variable` (`game_core.cpp` removed; `i18n.cpp` `[[maybe_unused]]` — used only by a
     debug-only `debug_print`); `-Wdeprecated-enum-enum-conversion` casts (`level_player.cpp`,
     `level_entry.cpp`); `-Wformat-nonliteral` a narrow `#pragma` at the controlled `strftime` call;
     `-Warray-bounds` a narrow `#pragma` in `movingsprite.cpp::Copy()` for a **confirmed GCC -O2
     speculative-devirtualization false positive** (GCC inlines a larger subclass dtor into the
     unique_ptr deleter; clang does not warn; at runtime the object is exactly a `cMovingSprite`).
3. **`-Woverloaded-virtual` (269): the mechanical fix broke the build, so it was dropped and deferred.**
   The reflex fix (`using Base::f;` in the derived to un-hide the base) made calls **ambiguous** —
   `cMovingSprite::Draw(cSurface_Request*=nullptr)` vs `cMouseCursor::Draw()`,
   `Set_Image(…, bool=false)` vs `Set_Image(…)`, `Delete_All()` vs `Delete_All(bool=false)` — because
   the base overloads have default arguments overlapping the derived arity. The hides are intentional
   and load-bearing. Reverted the 15 `using` edits, added `-Wno-overloaded-virtual`, and spun the real
   fix into `tasks/archive/2026/10/18/smc-overloaded-virtual.md` (per-method design decision).
4. **`-Wsign-conversion` (1835) deferred** to `smc-sign-conversion-sweep.md` at the maintainer's call
   (it later turned out to be only 273 first-party once CEGUI was `-isystem`; done there).
5. **Build-plumbing the gate exposed:**
   - **The gate needs clang installed.** Only `clang-tools-extra` was in the image; added `clang` so
     the gate can be built under both compilers.
   - **`-Werror` must come AFTER the `AC_CHECK_LIB` probes.** Putting it in `CXXFLAGS` before the
     probes turned the warnings in autoconf's own `char main();` probe into errors → every library
     check failed with a misleading "Unable to find libX11". Moved `-Werror` to just before `AC_OUTPUT`.
   - **`libX11-devel` was missing**, so a from-scratch in-image `./configure`'s `-lX11` link check
     failed (previously masked by pre-generated autotools artifacts). Added it.
   - **clang is stricter than gcc on `-Wsign-conversion`** (flags in-range constant `int`→`Uint8`) — a
     recurring theme: always re-check the other compiler.

**Result: 4368 → 0 first-party warnings.** Final gate `CXXFLAGS` in `smc/configure.ac`:
`-std=c++23 -Wall -Wextra -Wpedantic -Wshadow -Wvla -Wformat=2 -Wno-unused-parameter
-Wno-overloaded-virtual` with `-Wsign-conversion` added later by the sign-conversion task, and
`-Werror` appended after the probes.

## Verification

`make image` builds under gcc `-Werror` before and after `make format`; `gate-build.sh` reported 0
warnings under **gcc and clang**; `tools/smoke_headless.sh` passed (game ran to the 30 s deadline, ps
audit clean). No gameplay change; the smoke run hard-kills before `Exit_Game`, so the quit path was
not exercised.

## Harvested to durable docs

The reusable lessons (third-party `-isystem` before counting; `-Werror` after autotools probes; green
under both compilers; `run-clang-tidy -fix` header corruption; `-Woverloaded-virtual` don't-blanket-
`using`; sign-conversion `static_cast` is behaviour-preserving) were written into
`cpp-build-modernization.md` and `cpp-coding-standard.md` (runClaude/runCrush, 2026-10-18).
