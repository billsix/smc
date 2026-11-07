# Port smc from vendored CEGUI 0.7.9 to CEGUI 0.8.7 (parked follow-up)

**Status:** parked — the maintainer chose to stay on 0.7.9 for now (William Emerison Six
<billsix@gmail.com>, 2026-11-07). Revisit when he wants smc off the 2011 GUI library.
**Priority:** 9
**Difficulty:** 8

## BLUF

If smc ever leaves the vendored CEGUI 0.7.9, the only released target is **CEGUI 0.8.7**
(2016-04-28, the last upstream release; Fedora packages it as `cegui-devel`). 0.8 breaks the 0.7
API, so this is a port: headers, renderer bootstrap, imagesets, widgets, the editor, and the XML
classes the game uses for its own files. Done = smc builds against Fedora's `cegui-devel` 0.8.7,
`third_party/cegui-0.7.9/` and its `std::regex` patch are deleted (0.8 ships a native
`StdRegexMatcher`), menus and editor are play-tested, and the smoke run is green.

## Context

- Decision record: `tasks/archive/2026/11/07/smc-vendor-newer-dependencies.md` (the options and
  research). The 0.7.9 path being kept: `tasks/add-dep-update-target-cegui.md` (reproducible
  re-vendor + patch series).
- Scale: about 350 `CEGUI::Editbox`, 200 `Combobox`, 176 `WindowManager`, 210 `XMLAttributes`,
  and 78 `XMLSerializer` references in `smc/src`. `tasks/reference/smc/editors.md` lists 0.7-only
  calls.
- Precedents to mine: RPM Fusion's `smc-1.9-cegui-0.8.patch` (2014) and **TSC** (The Secret
  Chronicles of Dr. M, SMC's GPLv3 successor), which runs on CEGUI 0.8 with a CMake build. The
  CMake port (`tasks/smc-port-build-to-cmake.md`, decided 2026-11-07) should land first, so this
  port changes only one build system.

## Step 1 — the game's XML I/O (decided here, 2026-11-07)

The game reads and writes its own files (levels, worlds, savegames, preferences, campaigns,
powerups) through **CEGUI's XML classes** (`XMLParser`, `XMLHandler`, `XMLAttributes`,
`XMLSerializer`), which also change in 0.8. ISO C++ has no XML parser, so no `std::` swap is
possible. The options weighed (originally in `tasks/smc-replace-libraries-with-std.md`):

- (a) leave it on CEGUI's XML layer (expat underneath) and port it along with everything else;
- (b) **move the game's file I/O off CEGUI onto pugixml or tinyxml2** (small, permissively
  licensed, easy to vendor), so level/save parsing stops depending on the GUI library;
- (c) only switch CEGUI's own parser module to the bundled RapidXML (removes the `expat-devel`
  build dependency; changes no game code).

**Decision (William Emerison Six <billsix@gmail.com>, 2026-11-07): hold off. When this port goes
ahead, do (b) as its first step.** That separates the file-format work, which needs save/load
round-trip checks, from the widget port. It also shrinks the 0.8 diff to GUI code only.

## Plan (when un-parked)

1. XML I/O → pugixml (or tinyxml2; pick by API fit), with a save/load round-trip test over the
   shipped levels and worlds.
2. Build a 0.7→0.8 API map from TSC's history + the RPM Fusion patch.
3. Scaffold step-tasks: headers/bootstrap → imagesets/ImageManager → widgets/menus → editor →
   delete `third_party/` and the regex patch, and switch the build to `cegui-devel`.
