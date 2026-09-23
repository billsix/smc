Secret Maryo Chronicles
=======================

NOTE
----

SMC is no longer under active development. However, there is now a
new game based on SMC:

https://github.com/secretchronicles/TSC

Building and running (2026, containerized)
------------------------------------------

This checkout builds and runs on a current Fedora via Podman; the game is
ported to SDL 2 and the CEGUI 0.7.9 it needs is vendored under `third_party/`.

    [HOST]  make image      # Fedora 44 image; compiles CEGUI + the game into /usr/local
    [HOST]  make run        # runs /usr/local/bin/smc on your display (XWayland on Wayland);
                            # gamepads pass through -- plug in first (USE_CONTROLLER=0 to skip)
    [HOST]  make shell      # dev shell, repo mounted at /smc; then in it:
    [CONTAINER]  build.sh game && run.sh
    [HOST]  make help       # every target

> Needs `podman`. The first `make image` downloads ~400 MiB of packages and
> compiles CEGUI; later builds reuse the layers. Why each source change was
> needed: `tasks/reference/building-2010-era-smc-today.md`.

What is it?
-----------

From the official website at http://www.secretmaryo.org:

> Secret Maryo Chronicles is an Open Source two-dimensional platform
> game with a design similar to classic computer games. SMC has
> computer support to a great degree by using an accelerated [Open
> Graphics Library](http://opengl.org) for the best possible graphic
> design and stock performance.

It features a wealth of levels, powerups, great music and much more —
see http://www.secretmaryo.org/index.php?page=about for a more
comprehensive list. And for those still not getting enough, it
provides a great in-game level editor.

License
-------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
