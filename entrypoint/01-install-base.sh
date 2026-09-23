#!/usr/bin/env bash
#
# 01-install-base.sh -- the toolchain and every third-party library Secret Maryo Chronicles
# (2010-era C++: SDL 1.2, CEGUI 0.7, Boost) needs to build on Fedora 44, plus the dev tools the
# shell target expects. Host-runnable (a bare Fedora host or guest works the same way); the
# Dockerfile runs it. Which optional groups run is decided in the Dockerfile (ARG if-blocks) --
# this script takes no options. Add packages alphabetically within each group.
#
# Single dnf call after the guard, so its own exit status is this script's exit status.
set -uo pipefail

if ! command -v dnf >/dev/null 2>&1; then
    echo "01-install-base.sh: needs 'dnf' (this installs Fedora packages), not found." >&2
    echo "Run on a Fedora host/guest, or inside the project's Fedora-based image." >&2
    exit 1
fi

# The game was written for SDL 1.2 and is ported to SDL 2 in this repo (small, explained
# changes -- the same route as gltron), so the SDL2 family is what gets installed.
# CEGUI 0.7 is NOT packaged (Fedora's cegui is the incompatible 0.8) -- it is vendored under
# third_party/ and built by build.sh; its own dependencies are the freetype/pcre/expat/
# freeimage/glew lines below.
dnf install -y \
    autoconf \
    automake \
    bison \
    boost-devel \
    ccache \
    clang-tools-extra \
    expat-devel \
    flex \
    freeimage-devel \
    freetype-devel \
    gcc \
    gcc-c++ \
    gdb \
    gettext \
    gettext-devel \
    git \
    glew-devel \
    less \
    libpng-devel \
    libtool \
    make \
    mesa-libGL-devel \
    mesa-libGLU-devel \
    pcre-devel \
    pkgconf-pkg-config \
    procps-ng \
    SDL2-devel \
    SDL2_image-devel \
    SDL2_mixer-devel \
    SDL2_ttf-devel \
    tmux \
    vim-enhanced \
    which \
    xorg-x11-server-Xvfb
