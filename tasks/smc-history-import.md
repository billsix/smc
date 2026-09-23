# Import Secret Maryo Chronicles' history from SourceForge and GitHub into this repository

**Status:** first pass DONE 2026-09-23 — `master` holds 223 commits (12 SourceForge release
commits 2006–2009 + the 211-commit GitHub history 2010–2014 grafted on top), verified; **open:**
the six missing SourceForge releases and the unreachable CVS (below). Nothing pushed anywhere.
**Priority:** 3 · **Difficulty:** 3 (the remaining work is fetching files SourceForge only serves
to a browser, then re-running the import)
**Started:** 2026-09-23 (William Emerison Six <billsix@gmail.com>: "port all of the history from
the project … preserving all the history"; then "if sourceforge has a bunch of pre 2010 source
releases, version control those first, and then merge the github history in")

## BLUF

Rebuild SMC's history as one git repository: every obtainable SourceForge source release as a
dated commit, the author's GitHub history grafted on top, the pristine GitHub SHAs kept reachable.
Done = `master` is linear 2006→2014 and verified (tree of `master` == tree of FluXy/SMC `master`;
each grafted commit's tree == its original); the provenance is written down
(`tasks/reference/history-provenance.md`); and the gaps are either filled or explicitly recorded.

## Context

- Read first: `tasks/reference/history-provenance.md` (what is on `master` and why).
- Scripts (one-shot, relative paths, need `SMC_WORK=<scratch dir>` holding `smc-tarballs/`):
  `tasks/adhoc/history-import/fetch_tarballs.sh` (Debian snapshot orig tarballs by hash + the
  Wayback-held SourceForge originals), `sf_versions.sh` (SF release-folder listings via Wayback),
  `import_smc.sh` (release commits, then the graft), `import_smc_graft.sh` (the graft step alone,
  after the first run hit `branch -f` on the checked-out branch).
- Verification that was run: Debian tarball sha1s vs snapshot.debian.org; `bzip2 -t`/`unzip -t` on
  the Wayback archives; Debian 1.9 vs SourceForge 1.9 trees (`diff -rq`: only `data/music/*`
  missing from SF's); after the graft: 223 commits, 8 merges preserved, `git diff --quiet
  import/github-fluxy-master master` empty, rewritten GitHub root tree == original root tree.
- Remotes: `github-fluxy` = https://github.com/FluXy/SMC (read-only source). `master` has no
  upstream set on purpose; the maintainer adds `origin`/`github` per his convention.
- Commits on `master` were made unsigned (`-c commit.gpgsign=false` per command; no repo config
  written): they are the imported history, authored `Florian Richter <fluxey@gmail.com>` with the
  release dates. Everything the maintainer adds from here is his, signed as usual.

## Plan

- [x] Establish what survives (CVS unreachable; SVN host gone; GitHub intact; SF releases: 17
      folders, 12 archives obtainable — 11 via Debian, 1 via Wayback).
- [x] Release commits + graft + verification (2026-09-23).
- [ ] **Maintainer, in a browser:** download the six missing source archives from
      `https://sourceforge.net/projects/smclone/files/Secret%20Maryo%20Chronicles/<v>/` for
      `0.99.1 0.99.2 0.99.3 0.99.4 0.99.5 1.5` (the source zip/tarball, not the win32 installer) into
      `$SMC_WORK/smc-tarballs/`; then the agent adds them to `ORDER`/`DATE` in `import_smc.sh` and
      re-runs the import (rewrites `master`; fine before any push).
- [x] Decide on the CVS — **accepted as a gap** (William Emerison Six <billsix@gmail.com>,
      2026-09-23: "a moot point now, we have a ton of history"); the provenance doc records it. This
      task closes once the six releases are in.
- [ ] Maintainer: `git remote add origin …`, sign-off on the first push.

## Notes / decisions

- 2026-09-23: the CVS is not going to be pursued (maintainer's call); the release-granularity
  reconstruction plus the GitHub history is the history.

- Debian's music-bundled tarballs kept as the base (not SF's music-less ones) — the git era tracks
  the music; see the provenance doc.
- `v0.98-beta1` is dated by its archive's newest file (2006-03-30); SourceForge's listing for the
  old-style releases (0.97, 0.98) no longer exists.
- SourceForge answers 403 to non-browser HTTP clients and refuses rsync/pserver outright; every
  listing above came from the Wayback Machine (folder pages, CDX index) and snapshot.debian.org.

## Open questions

None — the CVS question was answered 2026-09-23 (gap accepted).
