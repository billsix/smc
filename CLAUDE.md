# smc — Secret Maryo Chronicles, history reassembled

This repository is the maintainer's reconstruction of Secret Maryo Chronicles' history: one
linear `master` from the 2006 `0.98 Beta 1` release to the last GitHub commit in 2014. **The
history's provenance is the whole point of the repo — read
`tasks/reference/history-provenance.md` before touching it.** In short: 12 release commits (from
Debian's orig tarballs and two Wayback-recovered SourceForge archives, dated by SourceForge's
release dates, tagged `v<version>`), then the 211 commits of github.com/FluXy/SMC grafted onto
`v1.9` (pristine SHAs at tag `import/github-fluxy-master`, remote `github-fluxy`).

- The game itself is under `smc/` (autotools; `smc/docs/` has the upstream build notes); `drafts/`
  are the author's 2010 design drafts.
- **Building and running:** the container-per-project template (`Dockerfile`, `Makefile`,
  `entrypoint/`). `make image` builds a Fedora 44 image that compiles the vendored CEGUI 0.7.9
  (`third_party/`) and the game into `/usr/local` at image-build time; `make run` runs
  `/usr/local/bin/smc` on the host display; `make shell` bind-mounts the checkout over `/smc`
  (`build.sh game` rebuilds from the mount, `run.sh` runs it). The game is **ported to SDL 2**
  (the 1.2 → 2 mapping and every other change a 2026 toolchain forced:
  `tasks/reference/building-2010-era-smc-today.md` — read it before touching `video/`, `input/`
  or the build). Verification is the nested `make image` plus the headless smoke run
  `tools/smoke_headless.sh` (`make shell-exec SCRIPT=tools/smoke_headless.sh`; hard-kill + `ps`
  audit — never leave a GUI process behind).
- **No formatter runs on shell exit** — the game keeps upstream's hand style; `make format` is
  opt-in and would drown the minimal-change diffs. The modernization task
  (`tasks/smc-modernize-cpp.md`, proposed) is where wholesale style changes belong.
- **Do not rewrite `master` casually** — but note it *will* be rewritten once more if the six
  missing SourceForge releases (or the CVS) turn up: `tasks/smc-history-import.md`.
- Layout follows the maintainer's conventions: `tasks/` (task docs), `tasks/reference/` (durable
  knowledge), `tasks/adhoc/` (the one-shot import scripts, kept as the record of how `master` was
  made — relative paths, driven by `SMC_WORK`).
- Commits are the maintainer's (signed, made on the host); the imported history commits were
  made unsigned by the agent as the reconstruction itself.
