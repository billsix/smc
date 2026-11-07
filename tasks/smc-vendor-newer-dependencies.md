# Can smc move to a newer CEGUI (or other newer dependencies)?

**Status:** proposed — needs go-ahead (William Emerison Six <billsix@gmail.com>, 2026-11-07)
**Priority:** 6
**Difficulty:** 8

## BLUF

The only dependency smc does not take from Fedora is **CEGUI 0.7.9**, vendored at
`third_party/cegui-0.7.9/` (github.com/cegui/cegui tag `v0-7-9`, carrying our `std::regex` patch).
Everything else (SDL2 + image/mixer/ttf, libpng, GL/GLU/GLEW, freetype, FreeImage, expat, gettext)
is already the current Fedora 45 package, so "newer" for those is automatic with the base image.
**Research (2026-11-07): the newest CEGUI *release* is 0.8.7 (2016-04-28).** The upstream repo has
no tag after `v0-8-7`, and its `v0`/`default` branches carry unreleased 0.9/1.0 work. 0.8 is a major
API break from 0.7 (ImageManager replaces Imageset, `CEGUI/` header paths, a changed
renderer bootstrap, Window/Font/XML API renames). Because smc calls CEGUI everywhere (about 350
`Editbox`, 200 `Combobox`, 176 `WindowManager`, and 210 `XMLAttributes` references), adopting 0.8.7 is
a **port**, not a re-vendor. Done for this task = a decision between the options below and, if one
is chosen, a follow-up implementation task (or step-task tree) scaffolded.

## Context

- Read first: `tasks/add-dep-update-target-cegui.md` (re-vendoring the **same** 0.7.9 + patches;
  it lists what changes with a newer CEGUI), `third_party/README.md`, `entrypoint/build.sh`
  (CEGUI configure flags), `tasks/reference/smc/editors.md` (lists 0.7-only CEGUI calls), and
  `tasks/smc-modernize-step-9-dedup-cegui.md`.
- **Stale doc found while researching:** `third_party/README.md` still says CEGUI "compiles
  unpatched … no `PATCHES.md` needed". Since the F45 upgrade it carries the `std::regex` matcher
  patch (`cegui/src/CEGUIPCRERegexMatcher.cpp`, `acinclude.m4`). Fix it with whichever CEGUI task
  goes first.
- Precedents for the 0.8 port: RPM Fusion carried `smc-1.9-cegui-0.8.patch` (2014), and SMC's
  successor **TSC** (The Secret Chronicles of Dr. M) moved to CEGUI 0.8 with a CMake build (its
  Gentoo ebuild pins `cegui <0.9`). Both are sources to mine for the API mapping, subject to
  licence checks (TSC is GPLv3, like SMC).
- Fedora still packages **cegui 0.8.7**. Moving to 0.8 could therefore **drop the vendored tree
  entirely** (use `cegui-devel`), which also removes our regex patch, since 0.8 has a native
  `StdRegexMatcher`.

## Options

- **(a) Stay on 0.7.9 vendored** and finish `add-dep-update-target-cegui.md` (reproducible
  re-vendor + patch series). Cheapest. The cost is carrying a 2011 GUI library and its patches
  indefinitely.
- **(b) Port to CEGUI 0.8.7 from Fedora's package** (no vendoring), mining TSC and the RPM Fusion
  patch. Large (difficulty ~8). The editor and the menus are the risk, and the smoke run does not
  exercise them.
- **(c) Port to CEGUI's unreleased `v0`/`default` branch, vendored at a SHA.** Newer than 0.8 but
  unreleased and still changing. Not recommended.
- **(d) Replace CEGUI** with a maintained immediate-mode/retained UI (Dear ImGui, RmlUi). This is a
  rewrite of every menu and the editor. It overlaps `tasks/smc-port-to-modern-engine-or-gl33.md`
  and belongs there rather than here.

Coupling: (b) is easier if the game's level/save XML I/O is first moved off CEGUI's XML classes
(`tasks/smc-replace-libraries-with-std.md`, XML option (b)), because those classes also change in
0.8.

## Plan (once an option is chosen)

1. (b): build a 0.7→0.8 API map from TSC's history + the RPM Fusion patch; scaffold a step-task
   tree (headers/bootstrap → imagesets → widgets/menus → editor → drop `third_party/`).
2. (a): do `add-dep-update-target-cegui.md` and close this.

## Open questions

1. Which option: (a) stay on vendored 0.7.9, (b) port to Fedora's CEGUI 0.8.7, (c) port to
   unreleased upstream `v0`, or (d) defer to the engine/UI-replacement task? *Recommend (a) now. If
   the maintainer wants smc off the 2011 library, (b) is the only released target.*
