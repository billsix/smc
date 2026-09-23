FROM registry.fedoraproject.org/fedora:44

# Feature flags default to 0 here (a bare `podman build` is lean); the Makefile passes 1.
ARG USE_GRAPHICS=0

# System-package installation lives in per-group scripts (entrypoint/0N-install-*.sh),
# host-runnable with no container runtime; the scripts take no options -- WHICH optional
# groups run is decided here by the ARG if-blocks. The dnf cache mount and keepcache stay
# in the Dockerfile (build plumbing; the scripts must not depend on them).
COPY entrypoint/01-install-base.sh entrypoint/02-install-graphics.sh /usr/local/bin/
RUN --mount=type=cache,target=/var/cache/libdnf5 \
    --mount=type=cache,target=/var/lib/dnf \
    sed -i -e "s@tsflags=nodocs@#tsflags=nodocs@g" /etc/dnf/dnf.conf && \
    echo "keepcache=True" >> /etc/dnf/dnf.conf && \
    dnf upgrade -y && \
    /usr/local/bin/01-install-base.sh && \
    if [ "$USE_GRAPHICS" = "1" ]; then /usr/local/bin/02-install-graphics.sh; fi && \
    echo 'set debuginfod enabled off' > /root/.gdbinit

COPY entrypoint/build.sh entrypoint/run.sh entrypoint/format.sh /usr/local/bin/
COPY entrypoint/dotfiles/.extrabashrc /root/.extrabashrc
RUN echo "source ~/.extrabashrc" >> ~/.bashrc

# The vendored dependency first (its own layer: a game edit does not rebuild CEGUI), then the
# game. The SOURCE is baked at the same path `make shell` bind-mounts the repo at (a pulled/loaded
# image has it; a mounted checkout shadows it with live edits); both BUILDS install into
# /usr/local (bin/smc on PATH, share/smc, lib/libCEGUI*), outside the mount, like gltron.
COPY third_party /smc/third_party/
RUN /usr/local/bin/build.sh cegui
COPY smc /smc/smc/
RUN /usr/local/bin/build.sh game
