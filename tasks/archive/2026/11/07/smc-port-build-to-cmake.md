# Ported the game's build from autotools to CMake

**Done 2026-11-07** (authorized batch; branch `std-swaps-managers-cmake`). Decisions by William Emerison
Six <billsix@gmail.com>, 2026-11-07: CMake rather than Meson; remove autotools once CMake is at parity.

## What was done

- **`smc/CMakeLists.txt`** builds the `smc` executable from the 77 sources `src/Makefile.am` listed, with
  dependencies from pkg-config / CMake packages: `PkgConfig::SDL2` (sdl2, SDL2_image, SDL2_mixer,
  SDL2_ttf), `PkgConfig::CEGUI` (CEGUI-OPENGL ≥ 0.7.2, the vendored 0.7.9 found through
  `PKG_CONFIG_PATH`) + `CEGUINullRenderer`, `OpenGL::GL`/`GLU`/`GLX`, `X11::X11`, `Intl::Intl`. Imported
  targets reach the compiler as `-isystem`, so third-party warnings don't count.
- The warning gate is a target option list plus `-Werror` (option `SMC_WERROR`, default ON). Options
  replace the old configure switches: `SMC_DEBUG` (was `--enable-debug`, `-D_DEBUG=1`) and
  `SMC_RUN_IN_PLACE` (was `--enable-runinplace`, no `DATA_DIR`). Installed builds get
  `DATA_DIR="<datadir>/smc"`; data installs to `share/smc` with the old 755/644 modes.
- **Build type:** defaults to RelWithDebInfo with `-DNDEBUG` stripped, i.e. `-O2 -g` with `assert()`
  live (3 asserts in the source), which is what autotools built.
- **`entrypoint/build.sh game`** runs `cmake -S $SMC/smc -B /opt/smc-build -G Ninja`, build, install. The
  build tree is a fixed path outside the bind-mounted repo (the overlay's "keep the build tree outside
  the mount" gotcha), so a `make shell` rebuild is incremental against the mounted sources and never
  meets a host build's files. `cmake` and `ninja-build` joined `entrypoint/01-install-base.sh`.
- **Removed:** `smc/configure.ac`, `smc/Makefile.am`, `smc/src/Makefile.am`, `smc/data/Makefile.am`,
  `smc/autogen.sh`. The generated autotools files were never tracked; `.gitignore`/`.dockerignore` still
  list them so a host tree's leftovers stay out of git and the image. CEGUI 0.7.9 still builds with its
  own autotools (`build.sh cegui`); `smc/docs/` still describes the upstream build (historical).
- Docs updated: `CLAUDE.md` (layout line + the gate paragraph), `tasks/reference/building-2010-era-smc-today.md`,
  the `tasks/reference/smc/{audio,gui,video}.md` citations of `configure.ac` line numbers.

## Meson vs CMake (why CMake)

Both could drive the vendored autotools CEGUI the same way (build it in `build.sh`, consume
`CEGUI-OPENGL.pc`). CMake won because the follow-ups assume it: vcpkg (`tasks/smc-windows-msvc-vcpkg.md`)
is CMake-first, and TSC, SMC's successor fork, already builds with CMake.

## Verification (nested podman, 2026-11-07)

- **Parity with the autotools build**, measured against a baseline captured from the last
  autotools-built image: the installed `share/smc` tree is identical (all 2,661 paths with their
  permission modes), and `ldd /usr/local/bin/smc` lists the identical library set. Compile flags match:
  `-O2 -g -std=c++23` + the full gate set + `-Werror`, `DATA_DIR="/usr/local/share/smc"` (CMake also
  passes the defines SDL2_image's pkg-config cflags carry, harmless).
- Full image rebuild (by hand with `podman build --network=host`, the nested gotcha) builds clean; the
  headless smoke passes (exit 137 at the deadline, clean `ps` audit).
- `tasks/adhoc/smc-port-build-to-cmake/compiler_gate.sh` (a throwaway out-of-tree CMake build of the
  mounted sources, `CXX=` selects the compiler): g++ and clang++ both `-Werror` clean, 0 warnings.
  It stays in `tasks/adhoc/` after archive because the manager refactor uses it as its gate.

## Follow-ups (unblocked by this)

`tasks/smc-windows-msvc-vcpkg.md`, `tasks/smc-linux-appimage-make-target.md`, `tasks/smc-github-cicd.md`,
and `tasks/smc-cegui-0.8-port.md` (parked) all build on the CMake build.
