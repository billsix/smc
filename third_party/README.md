# third_party — vendored dependencies

- `cegui-0.7.9/` — Crazy Eddie's GUI System 0.7.9, the GUI toolkit the game was written against
  (`CEGUI-OPENGL >= 0.7.2`). Source of github.com/cegui/cegui at tag `v0-7-9` (MIT licence:
  `cegui-0.7.9/COPYING`), vendored 2026-09-23 because no current distribution packages the 0.7
  API (Fedora ships the incompatible 0.8). Everything as tagged (`Samples/` is referenced by
  `configure.ac`, so it stays even though the samples are not built). Built by
  `entrypoint/build.sh cegui` into `/usr/local` at image-build time. **It is not pristine:** for
  Fedora 45, which removed PCRE1, its regex matcher was reimplemented with `std::regex`
  (`cegui/src/CEGUIPCRERegexMatcher.cpp`, `cegui/include/CEGUIPCRERegexMatcher.h`, and the
  `CEGUI_CHECK_PCRE` macro in `acinclude.m4`). `tasks/add-dep-update-target-cegui.md` turns that
  into a re-applicable patch series.
