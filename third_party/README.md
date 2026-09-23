# third_party — vendored dependencies

- `cegui-0.7.9/` — Crazy Eddie's GUI System 0.7.9, the GUI toolkit the game was written against
  (`CEGUI-OPENGL >= 0.7.2`). Source of github.com/cegui/cegui at tag `v0-7-9` (MIT licence:
  `cegui-0.7.9/COPYING`), vendored 2026-09-23 because no current distribution packages the 0.7
  API (Fedora ships the incompatible 0.8). Everything as tagged (`Samples/` is referenced by
  `configure.ac`, so it stays even though the samples are not built). Built by
  `entrypoint/build.sh cegui` into `/usr/local` at image-build time; it compiles unpatched under
  GCC 16 (no `PATCHES.md` needed so far — add one if that changes).
