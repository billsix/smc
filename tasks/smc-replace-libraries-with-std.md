# Replace third-party / hand-rolled code with standard C++ where the standard has it

**Status:** proposed — decisions recorded, implementation awaits go-ahead (William Emerison Six <billsix@gmail.com>, 2026-11-07)
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

### XML

ISO C++ has no XML parser, so CEGUI's XML layer has no `std::` replacement. Per the decision
below, the XML question moved to the CEGUI-upgrade follow-up task,
`tasks/smc-cegui-0.8-port.md` (its step 1), and is out of scope here.

## Decisions (William Emerison Six <billsix@gmail.com>, 2026-11-07)

1. **Which swaps:** the maintainer deferred to the agent ("whatever is better and more standard").
   The agent's choice is **all five std-replaceable rows**, in this order (cheapest and most
   contained first):
   number↔string helper bodies → `std::from_chars`/`std::to_chars`; filesystem helper bodies →
   `<filesystem>`; screenshot → `IMG_SavePNG` (drops the direct libpng link); `rand()` →
   `<random>` (one `std::mt19937` owned by `cApplication`); `SDL_GetTicks`/`SDL_Delay` →
   `std::chrono::steady_clock` + `std::this_thread::sleep_for` where they measure game time. The
   `ostringstream` → `std::format` row applies only to sites that do not feed gettext `_()`.
2. **XML:** hold off. It became step 1 of the CEGUI-upgrade follow-up (`tasks/smc-cegui-0.8-port.md`).

## Plan

1. Re-read the reference doc's "Deliberately NOT done" entries that touch these candidates. The
   `int_to_string` objection was about changing ~277 call sites; changing only the helper bodies
   avoids it. Record that in the reference doc.
2. One commit per row above, in that order, with the gate after each.
3. A maintainer play-test item for each where the smoke run can't see the change: screenshot key,
   random particle/enemy effects, level load with float attributes, frame pacing.
4. Add a section to `tasks/reference/refactoring-to-modern-cpp.md`.
