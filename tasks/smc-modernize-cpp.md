# Modernize the game's C++ to the newest standard, replacing the old patterns with the features made for them

**Status:** IN PROGRESS (umbrella) — started 2026-09-24 on branch `modernize-cpp` (off
`pre-modernization-fixes`), William Emerison Six <billsix@gmail.com>: "start on the C++ upgrades,
make a branch, work on your own with the same workflow ... commit yourself, archive a task, repeat".
This is now the **umbrella**; each Plan step below is its own step-task (`smc-modernize-step-N-*.md`),
done → committed → archived in turn. Decisions settled: **C++23**, and **one application object**
for the singletons (step 5).

## Step-task index (archive each on completion; archive this umbrella when the last lands)

- [x] **Step 0** — pin C++23 (`smc-modernize-step-0-pin-cpp23.md`, DONE 2026-09-24): `-std=c++23
  -Wall -Wextra` in `configure.ac`; compiles clean (0 errors). Warning baseline: 4662
  (`-Wdeprecated-copy` 2036, `-Wunused-parameter` 1604, `-Wdeprecated-declarations` 304, …) — the
  metric later steps reduce.
- [x] **Step 1** — `override` sweep (`smc-modernize-step-1-override-sweep.md`, DONE 2026-09-24):
  clang-tidy `modernize-use-override -fix` (codemod kept at `tasks/adhoc/modernize/tidy_override.sh`)
  added **611** `override` across 59 files (redundant `virtual` dropped); reformatted; compiles
  clean; warnings 4662 → 4496. No signature-mismatch surprises (clang-tidy only annotates real
  overrides).
- [~] **Step 2** — `nullptr`/`using`/bool-literals (`smc-modernize-step-2-nullptr-using-bool.md`,
  DONE 2026-09-24): clang-tidy `modernize-use-nullptr,use-using,use-bool-literals -fix`
  (`tasks/adhoc/modernize/tidy_step2.sh`); NULL 487→87 (400→`nullptr`; the 87 are macro/ambiguous
  contexts clang-tidy leaves), all `typedef`→`using`, bool literals fixed. 141 files, compiles
  clean. **`#define`→`constexpr` deferred** (clang-tidy can't do it safely; a later manual pass).
- [x] **Step 3** — drop Boost (`smc-modernize-step-3-drop-boost.md`, DONE 2026-09-24):
  `boost::filesystem` → `std::filesystem` in `filesystem.cpp` (namespace + file_type enum names);
  removed the never-enabled threaded renderer (`boost::thread` member/include, `Render_From_Thread`,
  `Make_GL_Context_*`, the `threaded` branch of `Render`, the `SMC_RENDER_THREAD_TEST` ifdef in
  main) — this also clears the render-thread item the dead-code task deferred; removed the three
  Boost `AC_CHECK_LIB`s + the mac `-lboost_system` from `configure.ac`, and `boost-devel` from
  `entrypoint/01-install-base.sh`. Compiles + **links with no Boost**, and the image builds with
  boost-devel absent. (Left: `Render_Finish` is now an empty no-op with dead call sites; the
  `pRenderer_current` double-buffer is now unused — both minor cleanups for later.)
- [x] **Step 4** — safe clang-tidy modernize sweep
  (`tasks/archive/2026/09/24/smc-modernize-step-4-safe-clang-tidy-sweep.md`, DONE 2026-09-24):
  `modernize-use-emplace` (247) + `modernize-use-equals-default` (37, dtor/default-ctor only);
  34 files, +354/−385; compiles+links clean, image builds, smoke green. `use-equals-default`'s 4
  copy/assign conversions (botched `GL_point` ctor; 3 hand-written `operator=`) reverted by the
  codemod → reviewed pass. `loop-convert` excluded (names range-for locals after the container →
  `m_`-prefixed locals) → moved to step 8. Warnings unchanged (4496). Last purely-mechanical step.

**--- WORKFLOW (maintainer, 2026-09-24): distinct single-purpose commits; the maintainer
play-tests + bisects the whole branch at the END, not per commit. So autonomous batching IS the
method; each commit must build + link + pass the headless smoke on its own. Descriptive-variable
names (`tasks/smc-descriptive-variable-names.md`) come AFTER all cpp modernization. ---**

- [~] Step 5 — ownership → `unique_ptr` / value (`smc-modernize-step-5-ownership.md`). Branches:
  5.1+8+5.2a+5.2/5.3 were on `modernize-cpp-2` (MERGED to master); 5.4+ on `modernize-cpp-3`.
  - [x] **5.1** render queue owns requests via `unique_ptr`.
  - [x] **5.2a** `cLayer` un-derived (non-owning; removed the double-ownership blocker).
  - [x] **5.2/5.3** `cObject_Manager<T>` → `vector<unique_ptr<T>>` across all 12 managers +
    `cSprite_Manager` semantics + collision lambdas/move-transfers (a runtime double-free was
    caught by smoke+gdb and fixed).
  - [x] **5.4** `Copy()` returns `std::unique_ptr<cSprite>` (34 methods).
  - [ ] **5.5** savegame graph (`cSave_*` owning lists + `m_spawned_objects`) → `unique_ptr`. NOTE:
    the save structures' raw ownership is CORRECT today (destructors free them) — this is
    cleanliness, not a bug fix; has parser→struct ownership transfers (use `release`/move).
  - [ ] **5.6** remaining owning `new`/`delete` in touched areas → `make_unique` (incl. the
    `make_unique` polish inside the `Copy()` bodies).
- [x] Step 8 (loops) — ~252 iterator loops → range-for (MERGED). Remaining under step 8:
  `std::format` for `int_to_string`/`ostringstream`/`sprintf`, and the once-per-frame `PlayerInput`
  struct (behavioral).
- [ ] Step 6 — one `cApplication` owning the 24 former singletons (fixes destruction-order hazards
  + `pFramerate` leak by member-declaration order). Pragmatic form: own them as `unique_ptr`
  members in dependency order and keep the `extern cX* pX` globals as borrowed aliases set once, so
  the thousands of `pX->` sites are untouched.
- [ ] Step 7 — `enum class` + virtual dispatch replacing the type-tag switches. **Entangled with
  the level/save FILE FORMAT** (the enum ints are serialized) — keep a serialization boundary; do
  the non-serialized enums first.
- [ ] Step 9 — CEGUI event lambdas + enemy dedup (`cWalkingEnemy` base).
- [ ] Step 10 — the `refactoring-to-modern-cpp.md` reference doc (write last, after the steps land).
**Priority:** 4 · **Difficulty:** 8 (77 `.cpp` / 85 `.h`, a 6-deep sprite hierarchy dispatched by
type tags, 35 global raw pointers; each step must keep the game playable)
**Depends on:** the container build (done: `tasks/archive/2026/09/23/smc-container-build.md`); `make
image` + `tools/smoke_headless.sh` are the regression check for every step here. **Grounded in**
the verified reference set `tasks/reference/smc/` (built by `tasks/archive/2026/09/23/smc-source-analysis-reference-set.md`,
done 2026-09-23) — the Context below is reconciled to it; read the set before starting.
**Recommended first:** the standalone bug/dead-code fixes indexed by
`tasks/smc-pre-modernization-fixes.md` — landing them on the current tree keeps this rewrite from
carrying known bugs and dead code. Step 3 (Boost drop) and `smc-remove-dead-code.md` both touch the
dead `SMC_RENDER_THREAD_TEST` block — coordinate so it goes once.

## BLUF

Take the game from "C++98 that happens to compile under GCC's default" to C++23 as written: a
pinned `-std`, ownership in `unique_ptr`, the twelve hand-rolled manager classes collapsed onto
standard containers and one owning application object, Boost dropped for `<filesystem>`/
`<thread>`, and the type-tag dispatch turned into virtual functions. Done = every step is its own
small commit that builds green in the container and passes the headless smoke run; the final
tree has no raw owning `new`/`delete`, no `NULL`, no Boost link dependency, `override` on every
overrider, and a reference doc that reads as a worked example of "refactoring to modern C++".

## Context

- Read first: the reference set `tasks/reference/smc/README.md` and its cross-cutting docs
  [[ownership-and-lifetime]], [[object-model]], [[game-loop-and-modes]] — they hold the verified
  detail this plan compresses. Then `tasks/reference/building-2010-era-smc-today.md` (build + SDL 2 port).
- **The standard today is accidental.** `smc/configure.ac` sets only `-Wall`; no `-std=` anywhere,
  so the code is C++98/03 compiled under GCC's default (gnu++17 on GCC 11+). Step 0 pins it.
- **The managers** (verified 2026-09-23): one template base `cObject_Manager<T>`
  (`core/obj_manager.h`, an owning public `vector<T*>` read directly at ~63 sites, with two real
  bugs both dead-by-nonuse — `Delete(size_t)` erases-then-deletes the shifted slot;
  `Switch_Array_Num` assigns a `T*` to an iterator under `// todo: does that work ?`, no caller).
  Its **13 subclasses**: `cSprite_Manager` (+ `cWorld_Sprite_Manager`), `cCampaign_Manager`,
  `cObjectCollisionType`, `cLevel_Manager`, `cBackground_Manager`, `cHud_Manager`, `cSound_Manager`,
  `cImage_Manager`, `cAnimation_Manager`, `cOverworld_Manager`, `cLayer`. (`cResource_Manager` and
  `cFont_Manager` are *not* `cObject_Manager`s — they hold a string / three fonts.) The container
  managers collapse to `std::vector<std::unique_ptr<T>>` members on their real owner.
- **24 global singletons**, `new`ed in one order in `main.cpp:Init_Game`, `delete`d in another in
  `Exit_Game`; the order disagrees in ≥5 places and `pFramerate` is never deleted (a leak). Plus
  the non-owning "active" aliases (`pActive_Camera/Player/Level/Overworld/Animation_Manager`).
  The fix is one `cApplication` owning them as `unique_ptr` members in dependency order.
- **Old-pattern counts** (verified, `rg`, 2026-09-23): raw `new` ~621 / `delete` ~195; `NULL` ~487;
  `typedef` ~40 (mostly `vector<cX*> XList`); explicit-iterator loops ~217; `virtual` ~740 vs
  **`override` 0** / `final` 0; unscoped `enum` ~36 (`enum class` 0); `SpriteType` = 58
  `TYPE_`-prefixed values, dispatched by ~20 `switch/if( m_type )` sites and ~90 tag-guarded
  `static_cast<cX*>`; `#define` constants ~37; int-as-bool (`bool x = 0`, `return 1;`) ~350; ~40
  `virtual cX *Copy() const` clones; 3 `sprintf` into `char[]` (`gui/hud.cpp`); `int_to_string`/
  `ostringstream` ~277; **CEGUI `subscribeEvent` ~216, all fire-and-forget** (no `Event::Connection`
  stored or disconnected). No smart pointers anywhere; no `auto_ptr`, no dynamic exception specs.
- **Boost — corrected from the first survey:** it is *not* 9 live call sites. The five
  `// todo : use boost::algorithm` comments in `property_helper` are **never acted on** (Boost.StringAlgo
  is unused). Real use is `boost::filesystem` in `core/filesystem/filesystem.cpp` only, and
  `boost::thread` in `video/video.{h,cpp}` behind the **disabled** `SMC_RENDER_THREAD_TEST`
  (dead). So `<filesystem>` alone replaces the one live Boost user; dropping the dead render
  experiment removes `boost::thread`; then all three `configure.ac` Boost link deps go.
- **Object model:** `cCollidingSprite → cSprite → cMovingSprite → cAnimated_Sprite → cEnemy →`
  concrete (depth 5–6). Dispatch is on `m_type`/`m_sprite_array`, not virtual. Biggest single dedup:
  a `cWalkingEnemy` base (the 13 enemies share a method shape; `cTurtleBoss` duplicates 22 methods
  of `cTurtle`). See [[object-model]], [[enemies]].
- The SDL 2 port (commit `d438165`; `git show d438165:tasks/adhoc/container-build/patch_sdl2.py`)
  is applied and stays underneath; this task starts from that tree.
- **Real bugs the analysis found** (fix opportunistically, note each; some deserve their own task):
  `Draw_Animation` CAPE→SMALL condition that can never fire; `Load_Images` fall-in-jump-slot for 4
  costumes; the `Finished_Sound` mixer-thread data race; `Fadeout_Sounds(filename)` fading all
  channels; `Toggle_Sounds` silencing music; `Apply_Max_Texture_Size` aspect ratio; `Joystick_Name_Click`
  wrong window name → uncaught throw; `main.cpp` arg-parse `option = i + i`. Full list: the ledger in
  `tasks/reference/smc/README.md`.

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
- [ ] **3. Drop Boost:** delete the dead `SMC_RENDER_THREAD_TEST` render-thread code (removes
      `boost::thread`); port `core/filesystem/filesystem.cpp` + `cResource_Manager`'s path
      concatenation to `std::filesystem` (the one live Boost user; also retires the 9 `#ifdef _WIN32`
      twin bodies and the `Trim_Filename`/`File_Exists` string surgery); then remove the three
      `AC_CHECK_LIB`s and `boost-devel` from `entrypoint/01-install-base.sh` (and from the two
      sandbox base lists only if nothing else there needs it).
- [ ] **4. Ownership:** `cObject_Manager<T>` → `std::vector<std::unique_ptr<T>>` (fixes both
      verified bugs by construction; the ~63 `->objects` loops become range-for); the ~40 `Copy()`
      clones → `std::unique_ptr<cSprite> Clone() const override`; the delete-inside loops disappear;
      remaining owning `new`/`delete` → `make_unique`. Also the render queue's per-frame `new
      *_Request` (the hottest allocation site) and the savegame pointer graph → values/`unique_ptr`.
- [ ] **5. The singletons and globals:** one `cApplication` owning the 24 former singletons as
      `unique_ptr` members in dependency (declaration) order — teardown then reverses automatically,
      fixing the ≥5 order hazards and the `pFramerate` leak. The container managers become plain
      members of their owner; the `extern cX *pX` globals become accessors, then owned members.
- [ ] **6. Enums and dispatch:** `SpriteType`/`ArrayType`/`Maryo_type`/`BackgroundType`/… → `enum
      class`; then the closed-set `m_type` switches and tag-guarded downcasts → virtual methods on
      `cSprite` (a `virtual Col_Valid_Type Validate_Player_Collision(...)` removes the level player's
      downcasts), guided by the author's own todos (`objects/box.cpp:453`, `enemies/spika.cpp:404`).
      Keep a serialization map where the raw int is the file format (`BackgroundType`).
- [ ] **7. Loops, strings, input:** iterator loops → range-for/`<ranges>`; `int_to_string`/
      `ostringstream`/`sprintf` → `std::to_string`/`std::format` (survives translator-reordered args
      in `hud.cpp`); sample input once per frame into a `PlayerInput` struct passed to
      `cLevel_Player::Update` (removes the 26 poll sites and the `Validate_Collision` input read).
- [ ] **8. CEGUI events + enemy dedup:** lambdas with stored `Event::Connection`s disconnected in
      the destructor (~216 sites; removes ~40 identical preambles); a `cWalkingEnemy` base folding
      `cTurtleBoss` onto `cTurtle` and the shared enemy method shape.
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
