# Replace third-party / hand-rolled code with standard C++ where the standard has it

**Status:** proposed — needs go-ahead (William Emerison Six <billsix@gmail.com>, 2026-11-07)
**Priority:** 5
**Difficulty:** 5

## BLUF

The F45 upgrade replaced CEGUI 0.7.9's PCRE1 regex matcher with `std::regex` and dropped a library
dependency. This task surveys what else can go the same way. The survey (2026-11-07) found four real
candidates: `rand()` → `<random>`, the hand-rolled `string_to_*` / `*_to_string` helpers →
`std::from_chars` / `std::to_chars`, the directory/file helpers → `<filesystem>`, and the direct
libpng screenshot writer → `IMG_SavePNG` (SDL2_image, which is already linked). **The CEGUI XML
layer (asked about specifically) has no standard-library replacement.** ISO C++ has no XML parser,
so that change means another third-party library (pugixml, tinyxml2) or a newer CEGUI, not a
`std::` swap (see "XML" below). Done = each accepted candidate converted in its own commit, `-Werror`
gate green under gcc and clang, headless smoke green, and the regex-style record added to
`tasks/reference/refactoring-to-modern-cpp.md`.

## Context

- Read first: `tasks/reference/refactoring-to-modern-cpp.md` (what was already modernized, and the
  "Deliberately NOT done" list. Two of the candidates below were parked there, so the reasons must be
  re-checked rather than ignored). The regex precedent: the archived F45 done-record
  `tasks/archive/2026/11/06/upgrade-to-fedora-45.md` and `tasks/add-dep-update-target-cegui.md`
  ("The vendored tree is NOT pristine upstream").
- The game links (`smc/configure.ac`): SDL2 + SDL2_image/mixer/ttf, libpng, GL/GLU/X11, CEGUI-OPENGL
  0.7.9 + CEGUINullRenderer, gettext. CEGUI is built (`entrypoint/build.sh`) with expat as its XML
  parser and FreeImage as its image codec.
- Gate: `make image` (nested) + `make shell-exec SCRIPT=tools/smoke_headless.sh`; the build is
  `-Werror` (CLAUDE.md).

## Survey (2026-11-07, counts from `grep` over `smc/src`)

| Candidate | Today | Standard replacement | Size | Notes |
|---|---|---|---|---|
| Random numbers | `rand()`/`srand()` | `<random>` (`std::mt19937` + distributions, one engine owned by `cApplication`) | 42 lines / 14 files | `rand() % n` has modulo bias and global hidden state. **Changes the random sequence**, so particle and enemy randomness will differ (no saved state depends on it; check). |
| Number↔string | `int_to_string`, `float_to_string`, `string_to_int/long/float` in `core/property_helper.cpp` | `std::to_chars` / `std::from_chars` inside the existing helpers | 460 call-site lines / 41 files, but **only ~5 function bodies** | Change the helper *bodies* and leave the 460 call sites alone, so the "~277 sites of churn" objection in the reference doc does not apply. `from_chars` is locale-independent, which is correct for level/save files. Gettext's printf-style format strings stay as they are. |
| Filesystem helpers | `File_Exists`, `Dir_Exists`, `Delete_File`, `Get_Directory_Files` (`core/filesystem/filesystem.{h,cpp}`) | `<filesystem>` (already used inside since Boost was dropped) | 15 lines / 8 files | Check whether any of these still hand-roll what `std::filesystem::exists/remove/directory_iterator` does. Probably a body-only change. |
| PNG screenshot | libpng `png_create_write_struct`… (`video/video.cpp` ~:1648, 21 lines) | `IMG_SavePNG` (SDL2_image, already a dependency) | 1 function | **Not `std`**, but it removes a direct link dependency (`AC_CHECK_LIB([png]…)`, `libpng-devel`) in the same spirit. |
| Timing | `SDL_GetTicks`/`SDL_Delay` | `<chrono>` + `std::this_thread::sleep_for` | 14 lines | Low value: SDL is still required for events/audio. Optional. |
| `ostringstream` | 7 lines / 2 files | `std::format` | small | The reference doc rejected `std::format` because of gettext printf strings. Only convert sites that do **not** feed `_()`. |

Not candidates: SDL2 (windowing/audio/input, no std equivalent), OpenGL, gettext (no std i18n),
FreeType/SDL2_ttf.

### XML (the CEGUI XML question)

The game uses CEGUI's XML layer as its general XML library: `CEGUI::XMLAttributes` (210 uses),
`XMLSerializer` (78), and `XMLParser`/`XMLHandler` subclasses for levels, worlds, savegames,
preferences, campaigns, and powerups (`gui/generic.cpp`, `overworld/*`, `level/level.cpp`,
`user/savegame.*`, `user/preferences.*`, `core/campaign_manager.*`, `objects/powerup.*`). CEGUI parses
with expat underneath.

- **There is no `std::` XML.** The regex trick worked only because `std::regex` exists.
- The real options are: (a) leave it (it works, and expat is maintained); (b) move the game's
  own file I/O off CEGUI onto **pugixml** or **tinyxml2** (small, MIT/zlib, single-file, easy to
  vendor) so level/save parsing no longer depends on the GUI library. This helps any later CEGUI
  upgrade or replacement (see `tasks/smc-vendor-newer-dependencies.md`), because the
  0.7→0.8 API break also hits these XML classes; (c) switch CEGUI's own parser module to RapidXML
  (already shipped in the vendored tree, `cegui/src/XMLParserModules/RapidXMLParser`, header-only).
  That drops the `expat-devel` build dependency but changes nothing in the game code.
- Recommendation: (b) if a CEGUI upgrade or replacement is going ahead, otherwise (a). It is a
  medium-sized change (about 20 files), and it is file-format I/O, so it needs save/load round-trip
  checks that the smoke run does not perform.

## Plan

1. Re-read the reference doc's "Deliberately NOT done" entries that touch these candidates; record
   which objections still hold.
2. One commit each, gate after each: `string`↔number helper bodies → `from_chars`/`to_chars`;
   filesystem helper bodies → `<filesystem>`; screenshot → `IMG_SavePNG` + drop libpng from
   `configure.ac` and `01-install-base.sh`; `rand` → `<random>`.
3. For each, a maintainer play-test item where the smoke can't see it (screenshot key, random
   effects, level load with float attributes).
4. Add a section to `tasks/reference/refactoring-to-modern-cpp.md`.

## Open questions

1. Which candidates to do? *Recommend: the number↔string helper bodies, filesystem, and
   `IMG_SavePNG` (cheap, body-only), then `<random>`. Skip chrono.*
2. XML: (a) leave it on CEGUI/expat, (b) move the game's file I/O to pugixml/tinyxml2, or (c) only
   swap CEGUI's parser module to RapidXML? *Recommend (a) for now, and (b) only as the first step of a
   CEGUI upgrade.*
