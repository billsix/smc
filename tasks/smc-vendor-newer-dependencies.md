# Decided: stay on vendored CEGUI 0.7.9 (newer-dependencies survey)

**Done 2026-11-07.** A decision task: could smc move to a newer CEGUI or other newer dependencies?

## Findings (researched 2026-11-07)

- CEGUI 0.7.9 (`third_party/cegui-0.7.9/`, github.com/cegui/cegui tag `v0-7-9`, plus the
  `std::regex` patch from the F45 upgrade) was the only dependency smc did not take from Fedora.
  SDL2 + image/mixer/ttf, libpng, GL/GLU/GLEW, freetype, FreeImage, expat, and gettext all come
  from Fedora 45, so they update with the base image.
- The newest CEGUI release was **0.8.7 (2016-04-28)**. Upstream had no tag after `v0-8-7`, and its
  `v0`/`default` branches held unreleased work. Fedora still packaged 0.8.7.
- 0.8 broke the 0.7 API (ImageManager, `CEGUI/` headers, renderer bootstrap, Window/Font/XML
  renames). Given how much smc calls CEGUI, moving was a port, not a re-vendor. Precedents existed:
  RPM Fusion's 2014 `smc-1.9-cegui-0.8.patch` and TSC, the CMake-built successor fork on 0.8.

## Options weighed

(a) stay on vendored 0.7.9; (b) port to Fedora's 0.8.7; (c) port to unreleased upstream `v0`;
(d) defer to the UI/engine-replacement question (`tasks/smc-port-to-modern-engine-or-gl33.md`).

## Decision (William Emerison Six <billsix@gmail.com>, 2026-11-07)

**(a) stay on 0.7.9.** The reproducible re-vendor stayed in `tasks/add-dep-update-target-cegui.md`.
The port to 0.8.7 became the parked follow-up `tasks/smc-cegui-0.8-port.md`, which also holds the
XML decision (move the game's file I/O to pugixml/tinyxml2 as the port's first step).
`third_party/README.md`'s stale "compiles unpatched" claim was corrected in the same change.
