# Build on Windows with MSVC via vcpkg

**Status:** proposed — needs go-ahead (William Emerison Six <billsix@gmail.com>, 2026-09-24)
**Priority:** 5 · **Difficulty:** 7
**Depends on:** `tasks/smc-port-build-to-cmake.md` (a CMake build is the prerequisite).

## BLUF

Make SMC build and run on Windows with MSVC, using **vcpkg** for the dependencies — the same
approach the maintainer's imps N64 game ports use. Done = a documented `vcpkg install` + CMake
(`-DCMAKE_TOOLCHAIN_FILE=…/vcpkg.cmake`) build that produces a running `smc.exe` on Windows, with
the Windows-specific code paths (the `#ifdef _WIN32` / `__WIN32__` branches, path handling, the SDL2
main shim) working.

## Context

- Model: the **imps** N64-games project builds its ports on Windows/MSVC via vcpkg — mirror its
  vcpkg manifest + CMake toolchain wiring and its CI shape. (Confirm imps's exact vcpkg/CMake setup
  from that repo before starting.)
- Dependencies to source from vcpkg: SDL2, sdl2-image, sdl2-mixer, sdl2-ttf, freetype, pcre, expat,
  freeimage, glew, gettext/libintl, and OpenGL. **CEGUI 0.7.9 is vendored** (not in vcpkg — vcpkg
  has 0.8+, incompatible), so it must build as part of the CMake project on MSVC too — expect this
  to be the hard part (a 2011 CEGUI on a modern MSVC).
- The SDL 1.2→2 port is already applied (`tasks/reference/building-2010-era-smc-today.md`); the
  Windows build inherits it.
- Grounded on the CMake build from the prerequisite task.

## Goal

A reproducible Windows/MSVC + vcpkg build (manifest mode) yielding a working `smc.exe`, documented
in the README with `[WINDOWS]`-labelled steps, matching the imps pattern.

## Open questions

1. Which SMC data/assets ship with the Windows build, and where does it look for them (the
   `runinplace` vs installed layout on Windows)?
2. How much work is vendored CEGUI 0.7.9 on modern MSVC — is a patch set needed (like the game's own
   SDL2 patches)?
