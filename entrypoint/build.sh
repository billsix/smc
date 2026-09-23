#!/usr/bin/env bash
#
# build.sh -- build the vendored CEGUI 0.7.9 into a prefix, then the game (autotools) against it,
# installing the game into a prefix beside the source. Runs at image-build time (Dockerfile) and
# interactively (`make shell`, then `build.sh` after an edit). All paths are under the repo root
# ($SMC, default /smc -- the bind-mount path), so a host run works too.
#
#   build.sh            everything (CEGUI is skipped when its prefix already has the library)
#   build.sh cegui      only CEGUI
#   build.sh game       only the game (incremental; re-runs configure when needed)
#
# Why CEGUI is built here at all: CEGUI 0.7 is not packaged anywhere current, so the image
# compiles the vendored copy into /usr/local (a committed image layer -- the exported image needs
# no network years from now); the game's configure finds it through PKG_CONFIG_PATH.
set -euo pipefail

SMC="${SMC:-/smc}"
# Both CEGUI and the game install into the system prefix (maintainer, 2026-09-23: "the same thing
# as gltron, but in /usr/local/bin"): /usr/local/bin/smc on PATH, data in /usr/local/share/smc,
# CEGUI's libraries in /usr/local/lib. Nothing lands under $SMC, so a `make shell` bind-mount of
# the checkout over /smc hides no build product, and a pulled image runs standalone.
PREFIX="${SMC_PREFIX:-/usr/local}"
JOBS="${JOBS:-$(nproc)}"
what="${1:-all}"

build_cegui() {
    if [ "$what" != cegui ] && [ -f "$PREFIX/lib/pkgconfig/CEGUI-OPENGL.pc" ]; then
        echo "== CEGUI already built in $PREFIX (build.sh cegui to redo)"; return
    fi
    echo "== CEGUI 0.7.9 -> $PREFIX"
    cd "$SMC/third_party/cegui-0.7.9"
    [ -x configure ] || ./bootstrap
    # Only what the game uses: the OpenGL renderer, the Null renderer (smc/configure.ac links
    # CEGUINullRenderer too), the expat XML parser, the FreeImage codec. Everything else (samples,
    # Lua/Python, Ogre/Irrlicht/DirectFB renderers) off.
    ./configure --prefix="$PREFIX" \
        --disable-samples --disable-lua-module --disable-python-module \
        --disable-ogre-renderer --disable-irrlicht-renderer --disable-directfb-renderer \
        --enable-opengl-renderer --enable-null-renderer --enable-external-glew \
        --enable-expat --disable-xerces-c --disable-libxml --disable-tinyxml --disable-rapidxml \
        --enable-freeimage --disable-silly --disable-devil --disable-tga --disable-corona --disable-stb \
        --disable-bidirectional-text --disable-minizip-resource-provider
    make -j"$JOBS"
    make install
    # Fedora's ld.so does not search /usr/local/lib by default; register it once so the game
    # finds libCEGUI*.so without LD_LIBRARY_PATH (skipped where we cannot write system config).
    if [ -w /etc/ld.so.conf.d ] && [ ! -f /etc/ld.so.conf.d/usr-local.conf ]; then
        echo "$PREFIX/lib" > /etc/ld.so.conf.d/usr-local.conf
    fi
    command -v ldconfig >/dev/null && ldconfig || true
}

build_game() {
    echo "== the game -> $PREFIX"
    cd "$SMC/smc"
    export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:$PREFIX/lib64/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
    [ -x configure ] || ./autogen.sh
    if [ ! -f Makefile ] || [ configure -nt Makefile ]; then
        ./configure --prefix="$PREFIX"
    fi
    make -j"$JOBS"
    make install
    echo "== built: $PREFIX/bin/smc (data: $PREFIX/share/smc)"
}

case "$what" in
    all)   build_cegui; build_game ;;
    cegui) build_cegui ;;
    game)  build_game ;;
    *) echo "usage: build.sh [all|cegui|game]" >&2; exit 2 ;;
esac
