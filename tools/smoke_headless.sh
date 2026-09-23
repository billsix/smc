#!/usr/bin/env bash
# Headless smoke run of the installed game inside the smc image: start Xvfb, run
# /usr/local/bin/smc for a bounded time, screenshot the root window at ~12 s, hard-kill, audit.
# Passes when the game is still alive at the deadline (exit 137) and no process is left behind.
# The screenshot is a bonus: the image ships no ImageMagick, so in-image runs report NONE; the
# sandbox-built frame is tasks/adhoc/container-build/data/main-menu-sdl2-xvfb-sandbox.png.
#
#   [HOST]  make shell-exec SCRIPT=tools/smoke_headless.sh
#
# Output: tasks/adhoc/container-build/data/smoke-<date>.log (+ .png when ImageMagick exists) in the mounted
# repo. Paths are relative to this script; nothing container-absolute except the install prefix.
set -u
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
out="$here/../tasks/adhoc/container-build/data"   # the worklog dir (gitignored logs are fine to keep)
mkdir -p "$out"
stamp="$(date +%Y-%m-%d_%H-%M-%S)"
BIN="${SMC_PREFIX:-/usr/local}/bin/smc"
[ -x "$BIN" ] || { echo "no $BIN -- build.sh first"; exit 2; }
command -v Xvfb >/dev/null || { echo "no Xvfb"; exit 2; }
disp=":77"
cleanup() { pkill -KILL -x smc 2>/dev/null; pkill -KILL -f "Xvfb $disp" 2>/dev/null; }
trap cleanup EXIT
Xvfb "$disp" -screen 0 1024x768x24 > /dev/null 2>&1 &
sleep 2
# SDL_VIDEODRIVER=x11: the Makefile sets it to wayland on a Wayland host session; this run is
# on our own Xvfb, so force X11 (and never touch the host's Wayland socket from a smoke test).
export DISPLAY="$disp" SDL_VIDEODRIVER=x11 SDL_AUDIODRIVER=dummy LIBGL_ALWAYS_SOFTWARE=1
unset WAYLAND_DISPLAY
timeout -s KILL 30 "$BIN" > "$out/smoke-$stamp.log" 2>&1 &
game=$!
sleep 12
shot=""
if command -v import >/dev/null; then
    import -display "$disp" -window root "$out/smoke-$stamp.png" && shot="$out/smoke-$stamp.png"
fi
wait "$game"; rc=$?
cleanup; sleep 1
leftover="$(ps -eo args | grep -E "^$BIN|^Xvfb $disp" || true)"
echo "game exit=$rc (137 = still running at the 30 s deadline = pass), screenshot=${shot:-NONE}"
echo "log:"; cut -c1-160 "$out/smoke-$stamp.log"
[ -z "$leftover" ] && echo "ps audit: clean" || { echo "ps audit: LEFTOVER: $leftover"; exit 1; }
[ "$rc" = 137 ]
