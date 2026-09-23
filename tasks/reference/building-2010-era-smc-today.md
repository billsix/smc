# Building 2010-era SMC on a 2026 toolchain — what had to change, and why

**Status:** reference (durable). Written 2026-09-23 by the agent for the maintainer (William
Emerison Six <billsix@gmail.com>) from the work in `tasks/archive/2026/09/23/smc-container-build.md`. States what is
TRUE of the build after that task; the task doc holds the chronology.

## In one paragraph

The game at HEAD (`smc/`, autotools, last upstream commit 2014) targets SDL 1.2, CEGUI 0.7 and a
Boost of its day. Getting it to build on Fedora 44 (GCC 16, Boost 1.90, SDL 2.32 via
`sdl2-compat`) took three things: **vendoring CEGUI 0.7.9** (no distribution packages the 0.7
API any more; Fedora's `cegui` is the incompatible 0.8) and building it into the image;
**porting the game from SDL 1.2 to SDL 2** (16 files, ~260 lines each way, applied by one
exact-match script so the port is reproducible and reviewable); and **one Boost include** (the
umbrella header replaces the removed `filesystem/convenience.hpp`). The game's own C++ compiled
under GCC 16 with no other change. CEGUI 0.7.9 compiled unpatched.

## The dependencies, and where each comes from

| Need (from `smc/configure.ac`) | Fedora 44 package | Note |
|---|---|---|
| `sdl2 >= 2.0.0` (was `sdl >= 1.2.10`) | `SDL2-devel` (provided by `sdl2-compat-devel` 2.32) | the port, below |
| SDL2_image / SDL2_mixer / SDL2_ttf | `SDL2_image-devel`, `SDL2_mixer-devel`, `SDL2_ttf-devel` | same call names as the 1.2 libs |
| libpng, GL/GLU, X11 | `libpng-devel`, `mesa-libGL-devel`, `mesa-libGLU-devel` | |
| Boost filesystem/thread/system | `boost-devel` (1.90) | one include renamed, below |
| `CEGUI-OPENGL >= 0.7.2` **and** `CEGUINullRenderer` | **vendored** `third_party/cegui-0.7.9/` | built at image-build into `/usr/local` |
| CEGUI's own deps | `freetype-devel`, `pcre-devel` (pcre 8), `expat-devel`, `freeimage-devel`, `glew-devel` | only the modules the game uses are built |
| gettext (`AM_GNU_GETTEXT`) | `gettext-devel` | |

The full list, host-runnable, is `entrypoint/01-install-base.sh`; X11/Mesa client libraries for
on-screen runs are `entrypoint/02-install-graphics.sh` (`USE_GRAPHICS`, default on from `make`).

## CEGUI 0.7.9 — vendored, built once per image

- Source: github.com/cegui/cegui at tag `v0-7-9`, MIT (`third_party/cegui-0.7.9/COPYING`),
  provenance in `third_party/README.md`. No local patches were needed: it compiles under GCC 16
  as tagged (231 warnings, 0 errors, 2026-09-23).
- `Samples/` must stay in the tree even though nothing builds it: `configure.ac` names the
  sample Makefiles, so `./bootstrap` fails without them (`--disable-samples` keeps them unbuilt).
- Configured by `entrypoint/build.sh cegui` with exactly the modules the game needs: the OpenGL
  renderer (external GLEW), the **Null renderer** (the game's `configure.ac` links
  `CEGUINullRenderer` too — without `--enable-null-renderer` the game's configure stops with
  "CEGUINullRenderer library not found"), the expat XML parser, the FreeImage codec. Lua/Python,
  Ogre/Irrlicht/DirectFB, bidi text and minizip are off.
- Gotcha for anyone rebuilding by hand: libtool bakes the `--prefix` into the `.la` files, so a
  `make install prefix=<other>` after a prefix-less configure is refused ("cannot install … to a
  directory not ending in /usr/local/lib"), and reconfiguring with a new prefix needs a `make
  clean` first or stale `.la` files keep the old one. `build.sh` always configures with the
  final prefix, so the image never hits this.

## The SDL 1.2 → SDL 2 port

Applied by one exact-match, idempotent script, `patch_sdl2.py`, removed once the port was
committed (read it with `git show d438165:tasks/adhoc/container-build/patch_sdl2.py`; the port
itself is the `smc/` diff of that commit). The mapping, so a reader of the diff knows why each
hunk exists:

| SDL 1.2 | SDL 2 in this port |
|---|---|
| `SDL_SetVideoMode` / `SDL_WM_SetCaption` / `SDL_WM_SetIcon` / `SDL_GL_SwapBuffers` | one `SDL_Window *screen` + `SDL_GLContext` created once in `cVideo::Init_Video`, then `SDL_SetWindowSize`/`SDL_SetWindowFullscreen`; `SDL_SetWindowIcon`; `SDL_GL_SwapWindow( screen )` |
| `SDL_ListModes` / `SDL_VideoModeOK` | `SDL_GetNumDisplayModes` / `SDL_GetDisplayMode`; `Test_Video` reports the bpp it was asked for |
| `SDL_GL_SWAP_CONTROL` attribute | `SDL_GL_SetSwapInterval` / `SDL_GL_GetSwapInterval` (a context property, not a GL attribute) |
| `SDL_GetKeyState()` indexed by `SDLKey` | `SDL_GetKeyboardState()` is scancode-indexed; `cKey_State` in `keyboard.h` keeps the keysym-indexed interface by translating with `SDL_GetScancodeFromKey`, so the ~30 key checks stay as written |
| `SDL_EnableUNICODE` + `keysym.unicode` | `SDL_StartTextInput`; `SDL_TEXTINPUT` events → `cKeyboard::Text_Input` (UTF-8 → `CEGUI::utf32` → `injectChar`) |
| `SDL_ACTIVEEVENT`, `SDL_VIDEORESIZE` | `SDL_WINDOWEVENT` (`SIZE_CHANGED` → `notifyDisplaySizeChanged`; `FOCUS_LOST` → the old pause-music-and-wait path) |
| wheel as buttons 4/5 | `SDL_MOUSEWHEEL`, mapped back to the two button constants in `cMouseCursor::Handle_Event` so every `Mouse_Down` handler keeps its wheel cases |
| `SDL_WarpMouse`, `SDL_PeepEvents(…, SDL_EVENTMASK(...))` | `SDL_WarpMouseInWindow( screen, …)`, `SDL_PeepEvents(…, SDL_MOUSEMOTION, SDL_MOUSEMOTION)` |
| `SDL_JoystickName( index )` | `SDL_JoystickNameForIndex( index )` (the SDL 2 `SDL_JoystickName` takes an opened joystick) |
| `SDL_GetWMInfo` clipboard code (X11 selection by hand) | `SDL_GetClipboardText` / `SDL_SetClipboardText` |
| `SDL_SetAlpha( surface, 0, 0 )` | `SDL_SetSurfaceBlendMode( surface, SDL_BLENDMODE_NONE )` |
| `SDLK_KP0..9`, `SDLK_PRINT`, `SDLK_LSUPER/RSUPER`, `SDLK_SCROLLOCK`, `SDLK_NUMLOCK`; `val <= SDLK_LAST` | `SDLK_KP_0..9`, `SDLK_PRINTSCREEN`, `SDLK_LGUI/RGUI`, `SDLK_SCROLLLOCK`, `SDLK_NUMLOCKCLEAR`; the upper-bound checks dropped (`SDL_Keycode` is sparse); `typedef SDL_Keycode SDLKey` in `global_basic.h` keeps the game's spelling |
| `char *e = SDL_GetError()` | `const char *` |

`configure.ac` asks for `sdl2` and the `SDL2_*` libraries; SDL2_image/mixer/ttf kept their 1.2
function names, so no call sites changed for them.

## Boost 1.90

`boost/filesystem/convenience.hpp` was removed (Boost ≥ 1.86); `filesystem.cpp` includes the
umbrella `boost/filesystem.hpp` instead. `boost::filesystem` v3 path semantics, `boost::thread`:
no change needed.

## Verifying a change

`make image` (nested or on the host) is the build gate; `make shell-exec
SCRIPT=tools/smoke_headless.sh` is the run gate — Xvfb, the installed `/usr/local/bin/smc` for
30 s, hard-kill, `ps` audit; pass = still alive at the deadline, nothing left behind. The game's
reference frame (main menu, sandbox build, 2026-09-23):
`tasks/adhoc/container-build/data/main-menu-sdl2-xvfb-sandbox.png`.

## Run-time notes

- CEGUI and the game both install into `/usr/local` at image-build time (maintainer's choice,
  2026-09-23: the same layout as gltron, binary on `PATH`): `/usr/local/bin/smc`,
  `/usr/local/share/smc` (`DATA_DIR`, compiled in by `configure --prefix`), `/usr/local/lib/
  libCEGUI*.so` (registered via `/etc/ld.so.conf.d/usr-local.conf` + `ldconfig` in `build.sh`,
  because Fedora's ld.so does not search `/usr/local/lib`). Nothing is installed under the repo
  path, so the `make shell` bind-mount hides no build product; a rebuild from the mounted tree
  (`build.sh game`) reinstalls over `/usr/local` in that container only. `entrypoint/run.sh`
  runs `/usr/local/bin/smc`. Saves, config and cache go to `~/.smc`.
- **Display from the container — X11 only.** SDL here is `sdl2-compat` over SDL3 and prefers
  X11; X11 needs the session's auth cookie, or Xlib says "Authorization required, but no
  authorization protocol specified" and SDL reports "x11 not available". The `Makefile` therefore
  mounts `$XAUTHORITY` (or `~/.Xauthority`) read-only at `/root/.Xauthority`. **Do not switch
  SDL to Wayland** (`SDL_VIDEODRIVER=wayland`): SDL then opens a window, but CEGUI 0.7.9's
  OpenGL renderer calls `glewInit()`, which needs a GLX display, and under Wayland/EGL the game
  dies at "OpenGLRenderer failed to initialise the GLEW library" (proven 2026-09-23 from the
  sandbox). On a Wayland desktop the game runs on XWayland. Fallback if X11 still refuses:
  `xhost +SI:localuser:root` on the host.
- **Gamepads:** `make run`/`make shell` bind-mount `/dev/input` with `--group-add keep-groups`
  (the runClaudeInContainer recipe), so SDL sees the host's evdev/js nodes; the game's
  `cJoystick` enumerates at startup, so plug in before launching. `USE_CONTROLLER=0` skips it.
- **"Can't move at the start of the overworld" is the game, not the port** (checked 2026-09-23):
  in world 1 Maryo starts standing on level 1's waypoint and the next waypoint is locked until
  that level is beaten, so the arrows do nothing; Return/Space (joystick button 2) enters the
  level. Overworld arrows are hard-wired to the cursor keys (`cOverworld::Key_Down`), the joystick
  synthesises them from the axes/hat; Esc/Backspace (joystick button 4) exits to the menu.
- Headless (Xvfb) the only complaint is SDL's "Time out elapsed after mode switch on display …
  no window becoming fullscreen; reverting" — Xvfb has no mode switching; the game carries on
  windowed. `SDL_AUDIODRIVER=dummy` and `LIBGL_ALWAYS_SOFTWARE=1` make a display-less container
  run possible for smoke tests.
