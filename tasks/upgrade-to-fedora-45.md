# Upgrade the container base from Fedora 44 to Fedora 45

**Status:** in progress
**Priority:** 4
**Difficulty:** 6

## BLUF
Bump `Dockerfile` to `fedora:45`; `make image` green nested, then the headless
smoke (`tools/smoke_headless.sh`). smc is a C++ SDL2 game (vendored CEGUI 0.7.9,
SDL1.2→2 port); the C++ build under GCC 16 / LLVM 23 is the risk. "Done" = image
builds + headless smoke passes on 45.

## Context
Fleet-wide Fedora 44 → 45 bump (William Emerison Six <billsix@gmail.com>,
2026-11-05). Fedora 45: GCC 16.2 / clang LLVM 23. The vendored CEGUI 0.7.9 (old
C++) and smc's own C++ are the main new-diagnostic risk; also SDL2/GLEW/X client
package availability. Running the game needs a display (verify = build + headless
smoke; on-display play-test is the maintainer's).

## Plan
1. `fedora:44` → `fedora:45`. 2. `make image` nested; then headless smoke.
3. Fix toolchain breakage minimally (discretion; log here). 4. Stage by path;
archive.

## Notes / findings
(appended as the build runs)
