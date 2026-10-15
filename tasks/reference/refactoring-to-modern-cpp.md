# Refactoring Secret Maryo Chronicles to modern C++ — a worked example

A durable, per-pattern record of how the 2010-era SMC codebase (C++98/03 compiled under GCC's
default) was modernized to C++23, one old pattern at a time, each replaced with the language or
library feature designed to replace it. Read it as a tutorial: each section is *the old pattern →
the modern replacement → what it caught or fixed → how it was verified*. It also records, honestly,
what was deliberately NOT done and why.

Grounded in the verified reference set `tasks/reference/smc/` (object model, ownership, game loop).
The cross-project method behind all of this — "flip the definition, let the compiler enumerate the
work" and the raw→`unique_ptr` migration idioms — is in the sandbox reference doc
`cpp-ownership-migration.md`.

> **A second pass followed this one (2026-10-15, `tasks/smc-apply-cpp-coding-standard.md`):** it
> applied the shared `cpp-coding-standard.md` and refined the ownership shapes recorded below —
> always-present non-polymorphic subobjects moved from `unique_ptr` to **value** members, and the
> singletons moved from plain `unique_ptr` members to a `cOwned<T>` wrapper destroyed in reverse
> declaration order. Where a section below shows a `unique_ptr` member that is a plain value today,
> that pass is why; the method is `cpp-ownership-migration.md` ("The second pass").

## The setup that made it safe

- **Pinned standard + warnings first.** `configure.ac` set only `-Wall` and no `-std=`, so the code
  was C++98/03 built as gnu++17 by accident. Step 0 pinned `-std=c++23 -Wall -Wextra` and recorded
  the warning count (4662) as the progress metric. Nothing else can be trusted modern until the
  standard is pinned.
- **A regression gate per change.** `make image` (a Fedora-44 + vendored-CEGUI container build) plus
  `tools/smoke_headless.sh` (a 30 s Xvfb launch, hard-killed, with a process audit) ran after every
  step. The gate proves *builds + links + starts + runs 30 s without crashing*; it does NOT prove
  gameplay or teardown (the smoke SIGKILLs), so behavioral changes are gated by the maintainer's
  play-test. Distinct, single-purpose commits make a regression bisectable.

## The mechanical sweeps (compiler-enforced, behavior-preserving)

These are the safe bulk — done with `clang-tidy -fix` (per-file flags, `-header-filter` scoped to
`smc/src` so vendored code is never touched) and verified by rebuild:

| Old pattern | Modern replacement | Notes |
|---|---|---|
| `virtual` overrides with no marker | `override` on every overrider (`modernize-use-override`) | +611 across 59 files; no signature-mismatch surprises |
| `NULL` | `nullptr` (`modernize-use-nullptr`) | 400 converted; 87 macro/ambiguous left |
| `typedef` | `using` (`modernize-use-using`) | all |
| `return 1;` / `bool x = 0` | `true`/`false` (`modernize-use-bool-literals`) | all |
| `push_back(std::string("x"))` | `emplace_back("x")` (`modernize-use-emplace`) | 247 |
| empty `~cFoo() {}` / `cFoo() {}` | `= default` (`modernize-use-equals-default`) | 37 — but see the wart below |
| explicit iterator `for` loops | range-`for` (`modernize-loop-convert`) | 252 |

**clang-tidy warts on old code (each verified, not trusted blindly):**
- `modernize-use-equals-default` *botched* a copy constructor with a member-init list and rewrote
  three hand-written `operator=` to `= default` (a copy/assign semantics change) — those four were
  reverted; only the destructor/default-ctor conversions were kept.
- `modernize-loop-convert` names each range-`for` variable after its container, so a member
  container `m_foos` yielded a local `m_foo` (breaks the `m_` = member convention); a scoped rename
  codemod fixed the 32 such locals.

## Boost dropped for the standard library

- `boost::filesystem` → `<filesystem>` in `core/filesystem/filesystem.cpp` (namespace alias +
  `file_type` enum names). The one live Boost user.
- `boost::thread` and the never-enabled `SMC_RENDER_THREAD_TEST` render-thread experiment: deleted
  entirely (the member, the include, `Render_From_Thread`, the threaded branch of `Render`).
- The three `AC_CHECK_LIB([boost_*])` and `boost-devel` from the image: removed. The image now
  builds and links with no Boost present.

## The ownership migration — raw owning pointers → `unique_ptr` (the centerpiece)

The pre-C++11 codebase owned everything through raw `T*` / `vector<T*>` and hand `delete`. This was
migrated to `unique_ptr` ownership, in distinct commits, each building + smoking green:

- **The render queue** (`cRenderQueue`): `vector<cRender_Request*>` → `vector<unique_ptr<…>>`. `Add`
  adopts the incoming raw pointer, so the 61 `Add(new c*_Request)` call sites are untouched; `Clear`
  frees by erase (`std::erase_if`), no hand-`delete`.
- **`cObject_Manager<T>`** (the base of 12 manager subclasses): the public `vector<T*> objects` →
  `vector<unique_ptr<T>>`. This is atomic (all subclasses share the one typed member). It fixes two
  long-dead bugs *by construction* and turns ~63 `->objects` loops into range-`for`. Method idioms:
  `Add` adopts via `emplace_back(raw)`; `Delete` erases (frees) or `release()`s when
  `delete_data=false`; `Get_Pointer` returns `.get()`; `find` → `find_if` over the `unique_ptr`.
- **`cSprite_Manager`** semantics preserved: auto-destroy slot reuse via `reset()`;
  `Move_To_Front/Back` move ownership (move-out / erase / reinsert) instead of erasing a raw copy;
  the player (`m_disallow_managed_delete`) is `release()`d before erase, not freed.
- **`cLayer` un-derived** from `cObject_Manager`: it only *borrows* the overworld line points (the
  sprite manager owns them), so it became a plain non-owning `vector<cLayer_Line_Point_Start*>`,
  removing a double-ownership hazard before the base flip.
- **`cObjectCollisionType` predicates** → lambdas over `unique_ptr` (retiring
  `std::binary_function`/`std::bind2nd`, both removed in C++17); collision-list transfers use
  `std::make_move_iterator`.
- **`Copy()`** → returns `std::unique_ptr<cSprite>` across 34 sprite classes (was a covariant raw
  `T*`), so a clone's ownership is explicit at the call boundary.
- **The savegame graph** (`cSave_*` owning lists + spawned-sprite lists) → `unique_ptr`, with
  parser→struct handoffs via `swap`/`release`.

**The lesson the smoke + gdb caught (a double-free that compiled fine):** `Add_Collisions` *moved*
collisions from a temporary list into a raw-owning list and then `clear()`ed the source — a move
that worked only because clearing a raw-pointer vector frees nothing. Once the source became
`vector<unique_ptr>`, `.clear()` freed them too, so each was owned twice and freed twice. Fixed with
`release()`. **A compile-clean ownership migration is not a correct one** — for an ownership change
the compiler proves the types line up, not that nothing is freed twice; the runtime smoke (and gdb
on its crash) is the second oracle.

## One owning application object

The 24 process-global singletons were `new`ed in one hand-chosen order in `Init_Game` and `delete`d
in another in `Exit_Game` (the order disagreed in several places and `pFramerate` was never freed —
a leak). A `cApplication` now owns all 24 as `unique_ptr` members; `Init_Game` creates each into its
member and sets the `extern cX* pX` global as a *borrowed alias* (so the thousands of `pX->` call
sites are untouched); `Exit_Game` `reset()`s them (the `pFramerate` leak is gone). The teardown
*order* was preserved from the original for safety — switching to automatic reverse-declaration
teardown is a follow-up, because teardown is smoke-invisible (the smoke SIGKILLs).

## Smaller safety wins

- The three `sprintf` into fixed `char` buffers in the HUD → `snprintf` with `sizeof`
  (buffer-overflow safety). Note: `std::format` was *rejected* here — the format strings come from
  gettext `_()` as printf-style (`"Points %08d"`), so `std::format`'s `{}` syntax would break the
  translation catalogs.

## Deliberately NOT done (and why) — the honest part

Modernization is also knowing which "obvious" changes cost more than they return:

- **`enum class`** for the 36 unscoped enums: ~860 enumerator-qualification edits for modest safety
  value (the enums work; `enum class` mainly prevents accidental cross-enum compares, not a bug
  here), and ~17 enums have their int values *serialized* in the level/save file formats, so they'd
  need a serialization boundary. Parked — `tasks/smc-modernize-step-7-enums-dispatch.md`.
- **Type-tag `switch` → virtual dispatch**: high value (the real "modern OO" showcase) but a
  high-risk gameplay rewrite that is smoke-invisible; wants small, individually play-tested commits.
- **Enemy dedup** (`cWalkingEnemy` / folding `cTurtleBoss` onto `cTurtle`): the "shared" methods
  differ by state enum, sound file, and logic (the boss scales speed by hits), so it is a real
  behavioral refactor, not a mechanical fold — not worth regressing verified-working gameplay for
  cleanliness. Parked — `tasks/smc-modernize-step-9-dedup-cegui.md`.
- **CEGUI event lambdas** (~216 fire-and-forget `subscribeEvent`): high churn, modest value (nothing
  dangles today). Parked with the dedup.
- **`int_to_string`/`ostringstream` → `std::to_string`/`std::format`**: ~277 sites of low-value
  churn, plus the gettext printf issue above.
- **`#define` constants → `constexpr`**: most are `GAME_*_DIR`/`DATA_DIR` string macros used in
  compile-time literal concatenation (`DATA_DIR "/" GAME_LEVEL_DIR`), which only works with macros —
  they must stay. Only a few int constants are safely convertible; low value.

## Verification status

Everything above builds and links under C++23 and passes the headless smoke. The ownership and
singleton work is play-test-verified through the first levels (maintainer, 2026-09-24). Teardown
order, save/load round-trips, the overworld, and the editor are covered by the maintainer's
continued play-test, not the smoke.
