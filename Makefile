.DEFAULT_GOAL := help

# Feature flags: 1 here, 0 in the Dockerfile (bare `podman build` is lean, `make` opts in).
USE_GRAPHICS ?= 1

CONTAINER_CMD = podman
CONTAINER_NAME = smc
REPO_MOUNT = /smc

# Extra flags for every container `run` (never `build`). Auto-set when running nested inside a
# runClaudeInContainer/runCrushInContainer sandbox (which exports NESTED_PODMAN=1); empty -- and
# byte-identical behaviour -- on a normal host. Overridable per invocation.
PODMAN_RUN_FLAGS ?= $(if $(filter 1,$(NESTED_PODMAN)),--cgroups=disabled)

TMUX_FILE := $(HOME)/.tmux.conf
TMUX_REAL_PATH := $(shell readlink -f $(TMUX_FILE))
TMUX_MOUNT := $(shell if [ -f "$(TMUX_REAL_PATH)" ]; then echo "-v $(TMUX_REAL_PATH):/root/.tmux.conf:Z"; fi)

# `make shell` bind-mounts the whole repo over the image's baked copy (same path), so edits
# persist to the host; shell.sh is mounted too, so a change to it needs no image rebuild.
FILES_TO_MOUNT = -v $(CURDIR):$(REPO_MOUNT)/:Z \
                 -v $(CURDIR)/entrypoint/shell.sh:/usr/local/bin/shell.sh:Z \
                 -v $(CURDIR)/entrypoint/build.sh:/usr/local/bin/build.sh:Z \
                 -v $(CURDIR)/entrypoint/run.sh:/usr/local/bin/run.sh:Z \
                 $(TMUX_MOUNT)

# Display passthrough: X11 over the /tmp/.X11-unix socket, Wayland over $XDG_RUNTIME_DIR; each
# block is empty when its variable is unset, so a headless host still gets a working shell.
#
# X11 needs the session's auth cookie too, or Xlib fails with "Authorization required, but no
# authorization protocol specified" (seen 2026-09-23 with the KDE Wayland session's XWayland):
# the file $XAUTHORITY names (or ~/.Xauthority) is mounted read-only at /root/.Xauthority. If
# X11 still refuses, `xhost +SI:localuser:root` on the host is the fallback.
#
# The game must run on X11 (XWayland on a Wayland session), never SDL's Wayland driver: CEGUI
# 0.7.9's OpenGL renderer initialises GLEW, which needs a GLX context -- under Wayland/EGL it
# dies with "OpenGLRenderer failed to initialise the GLEW library" (proven 2026-09-23). SDL here
# is sdl2-compat over SDL3 and already prefers X11; the Wayland block below stays for other
# tools in the shell, so do NOT add SDL_VIDEODRIVER=wayland.
X_AUTH_FILE := $(if $(XAUTHORITY),$(XAUTHORITY),$(HOME)/.Xauthority)
X_AUTH_FLAGS := $(if $(wildcard $(X_AUTH_FILE)),-e XAUTHORITY=/root/.Xauthority \
	-v $(X_AUTH_FILE):/root/.Xauthority:ro)
X_FLAGS_FOR_CONTAINER = $(if $(DISPLAY),-e DISPLAY=$(DISPLAY) \
	-v /tmp/.X11-unix:/tmp/.X11-unix \
	--security-opt label=type:container_runtime_t \
	$(X_AUTH_FLAGS))
WAYLAND_FLAGS_FOR_CONTAINER = $(if $(WAYLAND_DISPLAY),-e WAYLAND_DISPLAY=$(WAYLAND_DISPLAY) \
	-e XDG_RUNTIME_DIR=$(XDG_RUNTIME_DIR) \
	-v $(XDG_RUNTIME_DIR):$(XDG_RUNTIME_DIR))
DISPLAY_FLAGS = $(X_FLAGS_FOR_CONTAINER) $(WAYLAND_FLAGS_FOR_CONTAINER)

# Game controllers / joysticks, the same way runClaudeInContainer passes them: SDL reads gamepads
# from the host's evdev nodes (/dev/input/event*, legacy /dev/input/js*), invisible in the
# container by default. Bind-mount the input tree and keep the host user's supplementary groups
# so the 0660 device nodes are readable (rootless podman maps container-root to the host user,
# so systemd-logind's `uaccess` ACLs apply). Plug the controller in BEFORE launching -- no udev
# hotplug in the container, SDL enumerates only what exists at startup. USE_CONTROLLER=0 skips
# it; if SELinux denies (`ausearch -m avc -ts recent`), add `--security-opt label=disable`.
USE_CONTROLLER ?= 1
ifeq ($(USE_CONTROLLER),1)
  CONTROLLER_FLAGS_FOR_CONTAINER := $(shell if [ -d /dev/input ]; then echo "-v /dev/input:/dev/input --group-add keep-groups"; fi)
else
  CONTROLLER_FLAGS_FOR_CONTAINER :=
endif

# One flag block shared by `shell` and `shell-exec` so the two cannot drift (shell-exec = shell minus -it).
SHELL_RUN_FLAGS = --rm $(PODMAN_RUN_FLAGS) --entrypoint /bin/bash $(FILES_TO_MOUNT) $(DISPLAY_FLAGS) $(CONTROLLER_FLAGS_FOR_CONTAINER) $(CONTAINER_NAME)
SHELL_EXEC_ARGS = -c 'cd $(REPO_MOUNT) && $(if $(CMD),$(CMD),exec bash $(SCRIPT))'

.PHONY: all
all: shell ## Build the image and get a shell in it

.PHONY: image
image: ## Build the podman image (compiles CEGUI 0.7.9 and the game at image-build time)
	$(CONTAINER_CMD) build \
	                 --build-arg USE_GRAPHICS=$(USE_GRAPHICS) \
	                 -t $(CONTAINER_NAME) \
	                 .

.PHONY: shell
shell: image ## Dev shell with the repo mounted (edit + `build.sh` inside; GUI-capable)
	$(CONTAINER_CMD) run -it $(SHELL_RUN_FLAGS) /usr/local/bin/shell.sh

.PHONY: shell-exec
shell-exec: image ## Batch twin of shell: make shell-exec SCRIPT=<repo-relative path> | CMD='...'
	@[ -n "$(SCRIPT)$(CMD)" ] || { echo "usage: make shell-exec SCRIPT=<path> | CMD='...'"; exit 2; }
	$(CONTAINER_CMD) run $(SHELL_RUN_FLAGS) /usr/local/bin/shell.sh $(SHELL_EXEC_ARGS)

.PHONY: build
build: image ## Compile the game from the mounted tree once, in a throwaway container (a smoke check of your edits; `make shell` + build.sh + run.sh to iterate)
	$(CONTAINER_CMD) run $(SHELL_RUN_FLAGS) /usr/local/bin/shell.sh -c 'build.sh game'

.PHONY: run
run: image ## Run the image's /usr/local/bin/smc on the host display (gamepads passed through; USE_CONTROLLER=0 to skip)
	$(CONTAINER_CMD) run -it --rm $(PODMAN_RUN_FLAGS) \
		--entrypoint /bin/bash \
		$(DISPLAY_FLAGS) \
		$(CONTROLLER_FLAGS_FOR_CONTAINER) \
		$(CONTAINER_NAME) \
		/usr/local/bin/run.sh

.PHONY: format
format: image ## clang-format the game's C++ (opt-in; upstream style otherwise untouched)
	$(CONTAINER_CMD) run $(SHELL_RUN_FLAGS) /usr/local/bin/format.sh

.PHONY: image-export
image-export: ## Export the image to a timestamped tar in the repo root (gitignored)
	$(CONTAINER_CMD) save $(CONTAINER_NAME) -o $(CONTAINER_NAME)-$(shell date +%m-%d-%Y_%H-%M-%S).tar

.PHONY: image-import
image-import: ## Load an exported image tar: make image-import FILE=foo.tar
	$(CONTAINER_CMD) load -i $(FILE)

.PHONY: help
help:
	@grep --extended-regexp '^[a-zA-Z0-9_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
