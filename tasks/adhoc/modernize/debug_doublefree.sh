#!/usr/bin/env bash
# Headless gdb run to locate the double-free the ownership flip introduced. Runs the installed
# game under Xvfb; glibc's double-free aborts (SIGABRT), gdb catches it and prints the backtrace.
set -u
disp=":78"
cleanup() { pkill -KILL -x smc 2>/dev/null; pkill -KILL -f "Xvfb $disp" 2>/dev/null; }
trap cleanup EXIT
Xvfb "$disp" -screen 0 1024x768x24 >/dev/null 2>&1 &
sleep 2
export DISPLAY="$disp" SDL_VIDEODRIVER=x11 SDL_AUDIODRIVER=dummy LIBGL_ALWAYS_SOFTWARE=1
unset WAYLAND_DISPLAY
export MALLOC_CHECK_=3
timeout -s KILL 60 gdb -batch \
    -ex 'run' -ex 'bt' -ex 'info registers' \
    /usr/local/bin/smc 2>&1 | grep -vE "^\[|no debugging symbols|Missing sep" | tail -45
