# Full source analysis of the game — an in-depth reference set, then feed the modernization task

**Status:** COMPLETE 2026-09-23 (William Emerison Six <billsix@gmail.com>: "go ahead and do the
study of the source"). The reference set is `tasks/reference/smc/` (17 docs: the map `README.md`,
6 cross-cutting — object-model, ownership-and-lifetime, game-loop-and-modes, data-formats,
input-flow, editors — and 10 subsystem docs). `tasks/smc-modernize-cpp.md` rewritten from the
findings. **Archive owed:** move this task to `tasks/archive/2026/09/23/` in its own commit after
the work commit; the reference set stays (never archived).
**Started / finished:** 2026-09-23
**Method note:** ten subsystem readers ran in parallel; six (`core`, `level`, `video`, `gui`,
`user`, `audio`) finished on Fable before a rate-limit; the other four (`objects`, `enemies`,
`overworld`, `input`) and all synthesis were done directly on Opus 4.8. Each reader's report became
its subsystem doc (promoted into the set, not kept as a separate worklog); the two `cObject_Manager`
bugs were re-verified independently against the source.
**Priority:** 3 · **Difficulty:** 5 (done)
**Next:** `tasks/smc-modernize-cpp.md` (proposed) — its Context and Plan were rewritten from this
analysis (step 6 below).

## BLUF

Read the whole game — **SMC's own source only**, `smc/src` (77 `.cpp` / 85 `.h`, 73,459 lines);
not `third_party/` (CEGUI) nor SDL/Boost, which appear only as the seams the game calls
(maintainer, 2026-09-23: "the task should be on smc itself, not third party code like cegui") —
subsystem by subsystem, and write a layered reference set under `tasks/reference/smc/` — a map (`README.md`, one L0 capsule
per topic + status board), one L1 mental model per subsystem where it is non-obvious, and L2
mechanism docs anchored to stable names — so that the C++ modernization (and any later change)
starts from written, verified knowledge instead of a re-read. Done = every subsystem below has
its doc, every claim a later task would act on (ownership, lifetime, dispatch, dead code) has
been verified against the code rather than inferred, and `tasks/smc-modernize-cpp.md` has been
rewritten from the findings.

## Context

- Read first: `tasks/reference/building-2010-era-smc-today.md` (how the tree builds and runs
  today; the SDL 2 port), then `tasks/smc-modernize-cpp.md` (the survey numbers that motivated
  this: 12 manager classes on one owning-vector template, 35 global raw pointers, 621 raw
  `new`, 0 `override`, type-tag dispatch over `m_type`, Boost at 9 call sites). That survey was
  one read-only agent pass — a lead, not proof; this task is where each of its claims gets
  verified or corrected.
- Conventions to follow: the personal overlay's "Reference documents" and "Layered reference
  documents (LoD)" sections — L0 → L1 → L2 → L3 (source, cited by **stable named anchor**, never
  a line number), generate deepest-first then compress upward, compare each doc to a baseline the
  reader knows; and "Authoring a reference set for a codebase you don't know": one reader per
  subsystem in parallel returning `file:symbol`-anchored reports, the author synthesizes and
  **verifies** every claim a durable doc will be trusted on (especially "X is dead", "the seam is
  here"), distinguishes live from dead code explicitly, uses git history for why/when/who and the
  current code for what-is-true-now. Worked precedent: the imps mario64 set
  (`github.com/billsix/imps`, `tasks/reference/mario64/README.md` — a map with L0s + status
  board, per-topic docs).
- Git history is usable for "why": `master` carries the 12 release commits (2006–2009) and the
  211 GitHub commits (2010–2014), so `git log -S<symbol>` / `git blame` reach back to the
  author's own commit messages. Upstream is dead; the successor project is
  github.com/secretchronicles/TSC, whose refactorings of the same code are a second baseline
  worth comparing against (what they changed first, and why).
- The subsystems, by directory (`smc/src`, sizes 2026-09-23):

  | Directory | Files | Lines | What (first look) |
  |---|---|---|---|
  | `core/` (+ `filesystem/`, `math/`) | 13 cpp / 22 h | 9,698 | `main.cpp` (init/teardown of every singleton, the event loop), `game_core` (modes, `Game_Action`), `editor`, `obj_manager.h` (the owning-vector template), `sprite_manager`, `camera`, `campaign_manager`, property/XML helpers, math |
  | `objects/` | 16 / 16 | 13,984 | the sprite hierarchy (`cSprite` → `cMovingSprite` → `cAnimated_Sprite` …), boxes, items, platforms, paths, text box, ball, goldpiece, … |
  | `enemies/` (+ `bosses/`) | 13 / 13 | 11,069 | every enemy class; `cEnemy` base |
  | `level/` | 6 / 6 | 9,592 | `cLevel` (load/save XML, update/draw), `cLevel_Player` (the largest state machine), `level_manager`, `level_settings`, `level_background` |
  | `video/` | 7 / 8 | 8,512 | `cVideo` (window/GL context — the SDL 2 port lives here), `cGL_Surface`, `cImage_Manager`, `cFont_Manager`, `cAnimation_Manager` (particles), renderer queue, image settings |
  | `gui/` | 5 / 5 | 7,683 | CEGUI menus (`menu`, `menu_data` — 58 event subscriptions), HUD, generic dialogs |
  | `overworld/` | 7 / 7 | 4,762 | `cOverworld`, waypoints/layer lines, `cOverworld_Player`, world editor, world manager |
  | `input/` | 3 / 3 | 3,456 | keyboard (key state adapter, CEGUI injection), mouse (also the editor's object mover), joystick |
  | `user/` | 2 / 2 | 2,475 | preferences (XML), savegames (XML, per-level state) |
  | `audio/` | 3 / 3 | 2,228 | SDL_mixer sounds/music, `cSound_Manager` |

## Goal

A reference set that answers, per subsystem, "what owns what, who calls whom, where is the
seam, what is dead" — verified — and, on top of it, a sharper modernization plan: which
manager classes really collapse, which globals have a single owner, which `m_type` switches are
closed sets, where the CEGUI subscriptions dangle, what the sprite `Copy()` clones are for, and
in what order the rewrite is safe.

## Plan

- [x] **0. Scaffold** `tasks/reference/smc/README.md` (the map: provenance = commit `d438165`
      + the SDL 2 port; one L0 per topic below; a status board). Keep the existing
      `building-2010-era-smc-today.md` and `history-provenance.md` where they are and link them.
- [x] **1. Fan-out reading** (one reader per row of the table above, in parallel, read-only),
      each returning a structured report anchored to `file:symbol`: public surface, owned data
      and its lifetime (who `new`s/`delete`s), globals touched, dispatch points (`m_type` /
      `m_sprite_array` switches, virtuals overridden), CEGUI subscriptions and their owners,
      XML formats read/written, dead or vestigial code with the evidence, and the top three
      modern-C++ hot spots with counts.
- [x] **2. Cross-cutting topics** (the author, from the reports + code): the object model
      (hierarchy diagram, which virtuals each level adds, `Copy()`/clone semantics, collision
      dispatch); ownership and lifetime (the singleton construction/teardown order in
      `main.cpp`, who owns sprites — `cSprite_Manager` vs level vs editor — and the
      `cObject_Manager<T>` template); the game loop and modes (`Game_Mode`, `Game_Action`,
      mode transitions, the update/draw order, the renderer queue); data formats (level XML,
      world XML, savegame XML, preferences; the `property_helper` layer); input flow (SDL
      event → `cKeyboard`/`cJoystick` → CEGUI injection → mode handler → player state
      machine); the editor (level + world editors, object menus, the settings screens).
- [x] **3. Verify** before writing anything durable: every "dead code" claim by build + grep +
      `git log -S`; every ownership claim by reading both ends (allocation and deallocation);
      the two `cObject_Manager<T>` bugs from the survey (`Delete(size_t)` use-after-erase,
      `Switch_Array_Num`) by a minimal reproduction or a written proof from the code; the
      "0 `override`" and "construction vs teardown order disagree" claims by listing them.
- [x] **4. Write the set deepest-first**: L2 per subsystem (mechanism, anchored), then L1 where
      the model is non-obvious (object model, ownership, game loop, editor), then compress to
      L0s in the map. Each doc compares to a baseline the maintainer knows (gltron's C, the
      imps ports' patterns, TSC's later refactor of the same code).
- [x] **5. Live vs dead ledger**: one table in the map — every file/class judged live, vestigial,
      or dead, with the evidence — because the modernization deletes before it rewrites.
- [x] **6. Rewrite `tasks/smc-modernize-cpp.md`** from the findings: corrected counts, the
      concrete manager-collapse plan (which of the 12 become members of what), the global-pointer
      ownership map, the closed-vs-open `m_type` dispatch sites, the CEGUI subscription owners,
      a step order justified by the dependency graph, and the risks the reading found. Link
      every claim to the reference doc that verifies it.
- [x] **7.** Archive this task (its own commit after the work commit); the set stays.

## Notes / decisions

- **Scope is the game's own code.** `third_party/cegui-0.7.9/` is a vendored dependency and is
  not analysed or documented here beyond the API surface the game uses (`CEGUI::System`,
  window/event injection, the OpenGL renderer entry points) — those belong in the `gui/` and
  `input/` docs as "what the game asks of CEGUI". Same for SDL 2, SDL_mixer/image/ttf and Boost:
  documented as seams, not as libraries.
- Reference docs state what is TRUE, not what to DO: the modernization plan goes into the
  modernization task (step 6), the facts it rests on into `tasks/reference/smc/`.
- No source changes in this task. A bug found in reading is recorded in the relevant doc and,
  if it changes what the modernization does, in that task; a bug worth fixing on its own gets a
  `proposed` task of its own.
- Cite by stable anchor (`file` + class/function name), never a line number — the modernization
  will move every line.

## Open questions

None.
