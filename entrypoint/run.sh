#!/usr/bin/env bash
# run.sh -- launch the installed game (`make run`, or from `make shell`). Needs a display: the
# Makefile passes the host's X11/Wayland through; headless smoke tests use Xvfb (see the task doc).
# Saves and settings go to $HOME/.smc inside the container (ephemeral unless you mount one).
set -eu
BIN="${SMC_PREFIX:-/usr/local}/bin/smc"
[ -x "$BIN" ] || { echo "run.sh: $BIN not built -- run build.sh first" >&2; exit 1; }
exec "$BIN" "$@"
