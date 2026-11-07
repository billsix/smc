# Investigate and port the build system to CMake

**Status:** proposed — build system and migration decided 2026-11-07; implementation awaits go-ahead (William Emerison Six <billsix@gmail.com>, 2026-09-24)
**Priority:** 4 · **Difficulty:** 6

## BLUF

SMC currently builds with **autotools** (`smc/configure.ac`, `smc/src/Makefile.am`, `AM_*`,
`PKG_CHECK_MODULES`). Investigate porting it to **CMake**, then do the port: a `CMakeLists.txt` that
finds SDL2/SDL2_image/mixer/ttf, the vendored CEGUI 0.7.9 (`third_party/`), OpenGL/GLU/GLEW,
freetype/pcre/expat/freeimage, gettext, and builds the `smc` binary + installs the `data/` tree to
`/usr/local` (or the configured prefix), matching what autotools does today. Done = `cmake -S . -B
build && cmake --build build` produces a working binary that passes `tools/smoke_headless.sh`, the
container `make image` uses CMake, and the autotools files can be removed. **This is the enabler
task** — the vcpkg/Windows, AppImage, and GitHub-CI follow-ups all assume a CMake build.

## Context

- Read first: `tasks/reference/building-2010-era-smc-today.md` (the SDL 2 port + the current
  container build) and `smc/configure.ac` / `smc/src/Makefile.am` for what the build actually does
  today (the checks, flags, the `--enable-debug`/`--enable-runinplace` options, the CEGUI-OPENGL
  and CEGUINullRenderer link, the `-D_DEBUG`, `-DDATA_DIR`, the `-std=c++23 -Wall -Wextra` from the
  modernization).
- The game bundles CEGUI 0.7.9 under `third_party/` (Fedora's is the incompatible 0.8), built by
  `entrypoint/build.sh` — the CMake build must build/consume that vendored CEGUI, not the system one.
- Container: `Dockerfile` + `Makefile` (`make image`/`make shell`/`build.sh`); the port must keep
  the nested-podman `make image` + smoke gate working.
- The maintainer's other projects use CMake/meson in containers (e.g. spimulator meson); follow the
  container-per-project conventions.

## Meson or CMake? (added 2026-11-07, at the maintainer's "meson or cmake" ask)

Both can do this port. Differences that matter for smc:

- **The vendored CEGUI 0.7.9 is autotools.** Either build system has to drive it as an external
  project (CMake `ExternalProject_Add`, Meson `external_project` module) or keep building it in
  `entrypoint/build.sh` and consume it via pkg-config (`CEGUI-OPENGL.pc`). The pkg-config route is
  the least work and is identical in both. **If smc moves to CEGUI 0.8.7** (see
  `tasks/smc-vendor-newer-dependencies.md`), it comes from Fedora's `cegui-devel` and the question
  disappears.
- **CMake is what the follow-ups assume:** vcpkg (`tasks/smc-windows-msvc-vcpkg.md`) is CMake-first,
  and TSC, smc's successor fork, builds with CMake, so its build is reusable. Meson can consume
  vcpkg only through pkg-config or `cmake` dependencies, with more friction on Windows/MSVC.
- **Meson is shorter and stricter** (no `CMakeCache` path traps; see the overlay's "keep the build
  tree outside the mount" gotcha). The maintainer already uses it in spimulator.
- `-Werror` flag set, `DATA_DIR`, gettext `po/`, and the `--enable-debug`/`--enable-runinplace`
  options translate equally in both (`option()` / `meson_options.txt`).

*Recommendation: CMake, because the Windows/vcpkg and AppImage follow-ups depend on it. Choose
Meson only if Windows is dropped.*

## Goal

Replace autotools with a maintained `CMakeLists.txt` that reproduces the current build exactly
(same libs, flags, data install, options), verified by the headless smoke, so the cross-platform and
CI follow-ups have a modern, portable build to build on.

## Plan

- [ ] Inventory what `configure.ac` + `Makefile.am` do (libs, flags, options, data install).
- [ ] Write `CMakeLists.txt` (+ subdirs); `find_package`/`pkg_check_modules` for the deps; vendored
      CEGUI as a subproject/ExternalProject.
- [ ] Wire the container `build.sh`/`Dockerfile` to CMake; keep `make image` + smoke green.
- [ ] Remove the autotools files once CMake is at parity.

## Followups (created together, each depends on this)

- `tasks/smc-windows-msvc-vcpkg.md` — build on Windows/MSVC via vcpkg.
- `tasks/smc-linux-appimage-make-target.md` — AppImage as a make target.
- `tasks/smc-github-cicd.md` — GitHub CI driven by the Makefile.
- `tasks/smc-port-to-modern-engine-or-gl33.md` — the bigger rendering/engine question.

## Decisions (William Emerison Six <billsix@gmail.com>, 2026-11-07)

1. **Port fully; remove autotools once CMake works** (smoke-verified at parity), rather than
   keeping the two build systems side by side.
2. **CMake, not Meson.** The Windows/vcpkg and AppImage follow-ups depend on CMake, and TSC's CMake
   build is reusable (comparison above).
