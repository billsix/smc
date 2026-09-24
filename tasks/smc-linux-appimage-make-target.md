# Linux AppImage as a make target

**Status:** proposed — needs go-ahead (William Emerison Six <billsix@gmail.com>, 2026-09-24)
**Priority:** 5 · **Difficulty:** 5
**Depends on:** `tasks/smc-port-build-to-cmake.md` (build the binary), and pairs with
`tasks/smc-github-cicd.md` (CI can produce the AppImage as a release artifact).

## BLUF

Add a `make appimage` target that packages SMC as a self-contained Linux **AppImage** — the same
approach the maintainer's imps N64 game ports use. Done = `make appimage` (runnable in the nested
container) produces a single `SMC-x86_64.AppImage` that bundles the binary, the `data/` tree, and
the non-glibc shared libraries (SDL2 family, freetype, freeimage, glew, the vendored CEGUI), and
runs on a clean Linux host without installing dependencies.

## Context

- Model: the imps N64-games project builds an AppImage as a make target — mirror its
  `linuxdeploy`/`appimagetool` recipe and `AppDir` layout. (Confirm imps's exact AppImage recipe
  from that repo before starting.)
- SMC installs to a prefix and loads `data/` (see `smc/data/Makefile.am` install + the `DATA_DIR`
  handling); the AppDir must lay the data out so the bundled binary finds it (the `runinplace` /
  relative-path story matters here).
- CEGUI 0.7.9 is vendored + dynamically linked — its `.so` (and its data: schemes/imagesets/fonts)
  must be bundled too.
- Container-per-project convention: the target is a `.PHONY`, `##`-documented Makefile target that
  runs in the image (uses `$(CONTAINER_CMD)`, threads `PODMAN_RUN_FLAGS`), and gitignores the
  produced `*.AppImage`.

## Goal

A `make appimage` target producing a portable, dependency-free `SMC-x86_64.AppImage`, matching the
imps pattern, gitignored, ready for CI to publish as a release asset.

## Open questions

1. Bundle strategy: `linuxdeploy` + the SDL2 plugin, or a hand-rolled `AppDir` + `appimagetool`?
   (Match whatever imps uses.)
2. Data layout inside the AppDir so the bundled binary finds `data/` without an install step.
