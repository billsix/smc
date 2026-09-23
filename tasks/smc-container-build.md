# Build and run SMC in the standard container (Dockerfile + Makefile), with minimal source changes

**Status:** COMPLETE 2026-09-23 — verified on the maintainer's host (William Emerison Six
<billsix@gmail.com>: "so that's working now"; then playing the tutorial with an Xbox controller:
"it's working"). **Archive owed:** move to `tasks/archive/2026/09/23/` in its own commit after
the work commit, and `git rm` the one-shot `tasks/adhoc/container-build/patch_sdl2.py` and the
`data/smoke-*.log` worklogs there (the screenshot stays — the reference doc cites it).
**Priority:** 2 · **Difficulty:** 6
**Started:** 2026-09-23 (maintainer: "use the standard build system I use with dockerfile and
makefile to get an environment to build this project, smc, and do the minimal changes necessary
to get it to run"; "it's perfectly fine to vendor source into this repo"; "sdl 1.2 doesn't need to
be vendored, just make small changes for sdl 2, like I did for gltron")

## BLUF

The repo has the container-per-project template (`Dockerfile`, `Makefile`, `entrypoint/`):
`make image` builds a Fedora 44 image that compiles the vendored CEGUI 0.7.9 and the game into
`/usr/local`, `make run` runs it on the host display with gamepads passed through, `make shell`
bind-mounts the checkout for edit–`build.sh`–`run.sh` cycles. The game's source changed only for
the SDL 1.2 → SDL 2 port (16 files, one script) and one Boost include. What is true now, and
why each change was needed, is `tasks/reference/building-2010-era-smc-today.md`; this file is
the work record.

## Context (as it was at the start)

- Precedents: gltron (SDL 1 game, image-time build, `USE_GRAPHICS` groups) and smalltalk (legacy
  C, `PODMAN_RUN_FLAGS`, display passthrough, `image-export`/`import`), plus the personal
  overlay's container-per-project template.
- The game needed autotools, gettext, SDL 1.2 + image/mixer/ttf, libpng, GL/GLU/X11, Boost
  filesystem/thread/system and CEGUI-OPENGL ≥ 0.7.2 (plus, found later, CEGUI's Null renderer).
  Fedora 44 has everything except CEGUI 0.7 (its `cegui` is the incompatible 0.8) → vendored.

## What was done (in order)

1. **Plumbing.** `entrypoint/01-install-base.sh` (toolchain + every dnf dependency, host-runnable),
   `02-install-graphics.sh` (`USE_GRAPHICS`; later widened to gltron's X set + the Wayland client
   libraries/libdecor), `shell.sh` (`set -e`, `exec bash "$@"`), `build.sh` (CEGUI then the game;
   image-build time and interactive), `run.sh`, `format.sh` (opt-in only — upstream's hand style
   is kept, never formatted on exit), `Dockerfile`, `Makefile` (`image`/`shell`/`shell-exec`/
   `build`/`run`/`format`/`image-export`/`image-import`/`help`; `SHELL_RUN_FLAGS` shared by
   `shell` + `shell-exec`; `PODMAN_RUN_FLAGS`), `.dockerignore`, `.gitignore` additions.
2. **CEGUI 0.7.9 vendored** (`third_party/cegui-0.7.9/`, github.com/cegui/cegui tag `v0-7-9`,
   provenance in `third_party/README.md`). `Samples/` stays because `configure.ac` names their
   Makefiles (`./bootstrap` fails without them; `--disable-samples` keeps them unbuilt). It
   compiled unpatched under GCC 16 (0 errors, 231 warnings) — no `PATCHES.md` needed.
3. **SDL 1.2 → SDL 2 port**, `tasks/adhoc/container-build/patch_sdl2.py` (exact-match pairs,
   idempotent: a second run leaves `git diff` unchanged). 16 files, ~258 lines each way. The
   mapping table is in the reference doc. Pre-flighted in the sandbox before the image existed
   (`g++ -fsyntax-only` over all 77 `.cpp` → 0 errors), which also caught: `SDL_GetError` is
   `const char *`; `SDL_JoystickName(index)` → `SDL_JoystickNameForIndex`; the wheel (buttons 4/5
   → `SDL_MOUSEWHEEL`, mapped back so the button handlers keep their wheel cases); Boost ≥ 1.86
   dropping `filesystem/convenience.hpp`. The patch helper had an ordering bug (it inserted the
   `Text_Input` declaration four times when re-run) — fixed by checking "new already present"
   before "old present".
4. **The game's configure links `CEGUINullRenderer`** → `--enable-null-renderer` added to the
   CEGUI flags (found by the sandbox build: "CEGUINullRenderer library not found").
5. **`make image` green nested** (third attempt — the first two stalled in dnf at a 32 KiB/s
   mirror; killed and restarted at the maintainer's word; the dnf cache mount kept what was
   fetched). Also built, installed and run in the sandbox first against a scratch CEGUI prefix.
6. **Install layout → `/usr/local`** (maintainer: "the same thing as gltron, but in
   /usr/local/bin"): CEGUI and the game both install there (`SMC_PREFIX` overrides), `build.sh`
   registers `/usr/local/lib` via `ld.so.conf.d` + `ldconfig`, `run.sh` runs
   `/usr/local/bin/smc`; nothing lands under the repo mount. gltron itself still installs to
   `/bldInstall` — filed there: `gltron/tasks/install-into-usr-local.md`.
7. **Headless smoke run** (sandbox and in-image): Xvfb, `timeout -s KILL`, EXIT trap, `ps` audit;
   the game stays alive to the deadline and writes `~/.smc/…`; the sandbox run's 12 s
   screenshot is the full main menu (`data/main-menu-sdl2-xvfb-sandbox.png`). Promoted to
   `tools/smoke_headless.sh` (`make shell-exec SCRIPT=tools/smoke_headless.sh`).
8. **Display on the host.** First `make shell` + `smc` failed with "Authorization required, but no
   authorization protocol specified … x11 not available": no project of the maintainer's passed
   the X auth cookie. The `Makefile` now mounts `$XAUTHORITY` (or `~/.Xauthority`) read-only at
   `/root/.Xauthority`. **Tried and reverted:** `SDL_VIDEODRIVER=wayland` — SDL opened a window
   but CEGUI died at "OpenGLRenderer failed to initialise the GLEW library" (GLEW needs GLX), so
   the game is X11/XWayland-only (Makefile comment + reference doc). Maintainer-verified.
9. **Gamepad passthrough** (maintainer: "like runClaudeInContainer passes the joysticks/xbox
   controllers in"): the same `USE_CONTROLLER` block (`-v /dev/input:/dev/input --group-add
   keep-groups`) on `run`, `shell`, `shell-exec`. Maintainer-verified with an Xbox controller.
10. **"Can't move at the overworld start"** — investigated, not a port bug: world 1 starts on
    level 1's waypoint with the next one locked; Return/Space (joystick button 2) enters the
    level. Reproduced headless with `xdotool`; recorded in the reference doc.
11. **Docs:** `CLAUDE.md` build section, `README.md` commands-forward section, the reference doc.

## Decisions

- Port to SDL 2 rather than run on sdl12-compat (maintainer). Vendoring allowed (maintainer).
- No formatter on shell exit; wholesale style/modernization work is its own task:
  `tasks/smc-modernize-cpp.md` (proposed; C++23, one application object — decided).
- The `Makefile`'s `shell-exec` guard breaks on a `CMD=` containing double quotes (the standard
  guard, unchanged); use `SCRIPT=` for anything non-trivial.

## Gotchas met (for the next legacy port)

- Headless harness: `wait <game-pid>`, not bare `wait` (it also waits for Xvfb, so the EXIT trap
  never runs — a 9-minute orphan on the host); `unset WAYLAND_DISPLAY` + `SDL_VIDEODRIVER=x11`,
  because the sandbox mounts the host's `$XDG_RUNTIME_DIR` and an SDL3-era game will otherwise
  open a window on the maintainer's real desktop.
- libtool bakes the configure `--prefix` into `.la` files: no `make install prefix=` afterwards,
  and `make clean` before reconfiguring with a new prefix.
- The game's main menu follows the mouse pointer; an `xdotool` probe must park the pointer first.

## Open questions

None.
