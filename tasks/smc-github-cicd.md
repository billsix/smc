# Build on GitHub CI/CD, Makefile-driven

**Status:** proposed — needs go-ahead (William Emerison Six <billsix@gmail.com>, 2026-09-24)
**Priority:** 5 · **Difficulty:** 4
**Depends on:** `tasks/smc-port-build-to-cmake.md`; consumes `tasks/smc-linux-appimage-make-target.md`
and `tasks/smc-windows-msvc-vcpkg.md` as they land.

## BLUF

Add GitHub Actions CI that **drives most of the build through the project's Makefile** — the same
approach as `github.com/billsix/modelviewprojection`, where the workflow calls `make` targets rather
than duplicating build logic in YAML. Done = a `.github/workflows/*.yml` that, on push/PR, builds
SMC and runs the headless smoke (Linux via the container `make image` + `make shell-exec
SCRIPT=tools/smoke_headless.sh`), and — as the follow-ups land — builds the AppImage (`make
appimage`) and the Windows/MSVC+vcpkg build, publishing artifacts on tags.

## Context

- Model: `github.com/billsix/modelviewprojection` — its GitHub workflow invokes the repo's `make`
  targets so the build logic lives in the Makefile, not the YAML (read that repo's
  `.github/workflows/` before starting, and its README CI notes).
- SMC's build is containerized (`Dockerfile` + `Makefile`); the Linux CI job should build the image
  and run the smoke inside it, matching how a developer builds locally (`make image`, then
  `make shell-exec SCRIPT=tools/smoke_headless.sh`). Nested podman is not needed on the CI runner
  (it can run podman/docker directly), but the Makefile targets should Just Work.
- The headless smoke (`tools/smoke_headless.sh`) is Xvfb-based and returns a clear pass/fail — it is
  the natural CI gate.
- Reference the project by its GitHub URL in any committed CI docs (per the cross-project convention).

## Goal

Makefile-driven GitHub Actions: build + headless-smoke on Linux for every push/PR; AppImage +
Windows artifacts on tags — thin YAML, logic in the Makefile, mirroring modelviewprojection.

## Open questions

1. Linux runner strategy: build the project's Fedora image in CI (slow, faithful) vs. install the
   toolchain directly on `ubuntu-latest` (faster, but diverges from the container). (Lean: build the
   image so CI matches local exactly.)
2. Which events publish artifacts — tags only, or every main-branch build?
