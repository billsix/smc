#!/usr/bin/env bash
# Interactive dev shell (`make shell`) and the batch twin (`make shell-exec SCRIPT=…|CMD=…`).
# The image built CEGUI and the game once at image-build time, into /usr/local (bin/smc on
# PATH); `make shell` bind-mounts the host repo over /smc, so rebuild after editing with:
#
#     build.sh            # CEGUI (skipped when already installed) + the game: configure, make, install
#     build.sh game       # just the game (incremental; reinstalls over /usr/local in this container)
#
# set -e guards only the setup; the exec'd bash is a fresh process. "$@" carries the shell-exec
# payload (-c '…'); with no args this is the interactive shell.
set -e
cd /smc
[ "$#" -eq 0 ] && echo "smc dev shell -- build.sh rebuilds the game; run.sh runs it (needs a display)."
exec bash "$@"
