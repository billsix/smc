#!/usr/bin/env bash
#
# 02-install-graphics.sh -- X11 / Mesa libraries for running the game on the host display
# (`make run`, `make shell` with display passthrough). Corresponds to the Dockerfile's
# USE_GRAPHICS flag; the Dockerfile runs this only when USE_GRAPHICS=1. No options -- see
# 01-install-base.sh for the design. Single dnf call, so its exit status is the script's.
set -uo pipefail

if ! command -v dnf >/dev/null 2>&1; then
    echo "02-install-graphics.sh: needs 'dnf', not found." >&2
    exit 1
fi

# The X set matches gltron's (SDL3's X11 driver dlopens Xcursor/Xfixes/Xi/Xrandr/Xrender/...);
# the Wayland client libraries + libdecor are what SDL3's Wayland driver dlopens (they arrive
# as SDL3 dependencies anyway -- listed so a bare-host install is explicit).
dnf install -y \
    libdecor \
    libglvnd-gles \
    libwayland-client \
    libwayland-cursor \
    libwayland-egl \
    libxkbcommon \
    libXcomposite \
    libXcursor \
    libXdamage \
    libXfixes \
    libXft \
    libXi \
    libXinerama \
    libXmu \
    libXrandr \
    libXrender \
    libXres \
    libXScrnSaver \
    libXtst \
    libXv \
    libXxf86vm \
    mesa-demos \
    mesa-dri-drivers \
    mesa-libEGL
