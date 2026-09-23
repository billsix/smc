# Reference: where this repository's history comes from

> Written 2026-09-23 when the repository was assembled (task: `tasks/smc-history-import.md`).
> States what is TRUE about the commits on `master`; the how-to is in the task and the scripts under
> `tasks/adhoc/history-import/`.

Secret Maryo Chronicles (SMC) was developed by Florian Richter ("FluXy") and contributors from
~2003 to 2014. Its version control moved three times, and only the last hop survives as a
repository; `master` here is one linear history stitched from what is still obtainable:

| era | where it lived | what survives | how it appears on `master` |
|---|---|---|---|
| ~2003 – 2006 | SourceForge project `smclone`, **CVS** | the CVS repository is still on SourceForge but unreachable: SF made CVS read-only in 2017 and its host (`a.cvs.sourceforge.net`, 216.105.38.20) refuses rsync (873) and pserver (2401) from every network tried (this sandbox and the maintainer's host, 2026-09-23); the documented remaining route is SF's interactive shell (needs an SF account) | **not present** — see "Gaps" |
| 2006 – 2010-03 | external **SVN** at `opensvn.csie.org/SMC/SMC/` (the project's `svnexternalurl` link), r1 … r1641 | the host is gone; nothing archived | **release granularity only**: one commit per SourceForge source release (below) |
| 2010-03-13 – 2014-12-02 | **git**, github.com/FluXy/SMC, 211 commits, 8 merges, root commit "initial smc commit with the changes from last SVN (1641)" | intact | grafted on top of the 1.9 release commit; SHAs rewritten by the graft, the pristine originals are tag `import/github-fluxy-master` (remote `github-fluxy`) |

## The release commits (12, 2006-03-30 → 2009-09-02)

One commit per release, author `Florian Richter <fluxey@gmail.com>` (the author identity the
GitHub history uses), author/committer date = the release's SourceForge upload date (from the
Wayback Machine's 2026-06-19 snapshot of the SF files listing), tagged `v<version>`. Each commit
message names its source archive and sha1.

| tag | date | source archive | provenance |
|---|---|---|---|
| `v0.98-beta1` | 2006-03-30 (archive's newest file; no SF folder date survives) | `SMC_0.98_Beta_1.zip` | SourceForge original, bytes from the Wayback Machine; a combined source + Windows-binary release, imported as released |
| `v0.99.6` … `v1.4`, `v1.6` … `v1.9` (11) | SF release dates | `smc_<v>.orig.tar.gz` | **Debian's orig tarballs from snapshot.debian.org**, hashes verified against the snapshot index |

**Why Debian's tarballs are the right base, not a compromise:** for 1.9 the SourceForge original
(`smc-1.9.tar.bz2`, also recovered from the Wayback Machine) was diffed against Debian's orig:
zero differing files; the only difference is that the SF tarball *lacks* `data/music/{game,land,
overworld,story}` — SourceForge shipped the music as a separate "Addon - Music" download and Debian
bundled it back in (then split it into its `smc-music` package). The author's own repository tracks
the music (`smc/data/music/`, 44 files at the GitHub root commit), so the Debian trees are the ones
that line up with the git era. The other ten Debian tarballs are assumed to follow the same
convention (Debian packaged them the same way); their SF originals are not obtainable to check.

## Layout

Every release's tarball top directory is imported as `smc/`, because that is the layout of the
GitHub repository (`smc/` = the game; `drafts/` and `README.md` appear only in the git era).

## What the SourceForge 1.9 → GitHub-root delta is

`git diff v1.9 693d6ec --shortstat` → 404 files changed (+100 665 / −138 241): the SVN work between
the 1.9 release (2009-09) and r1641 (2010-03) — the next release, 2.0, was never made. That delta is
the only trace of those six months.

## Gaps (what is NOT here and where it could still come from)

- **SourceForge source releases missing between 0.98 and 1.9:** `0.99.1`, `0.99.2`, `0.99.3`,
  `0.99.4`, `0.99.5`, `1.5` (SF folders exist; Debian never packaged them; the Wayback Machine holds
  the folder pages but not the files, except the folder dates). They can be downloaded in a browser
  from `https://sourceforge.net/projects/smclone/files/Secret%20Maryo%20Chronicles/<version>/`
  (SF's mirrors answer 403 to non-browser clients). Once obtained, re-run the import
  (`tasks/adhoc/history-import/import_smc.sh`) — it rewrites `master` (nothing was pushed) — with
  the new tarballs in `ORDER`/`DATE`.
- **Everything before 0.98 and every intermediate CVS/SVN commit:** only via SourceForge's CVS
  (needs an SF account: `ssh -t USER@shell.sourceforge.net create`, then `rsync -aiv
  /home/classic/cvs/smclone/ ~/cvs/`) and then `cvs-fast-export` / `cvs2git` — the tooling is ready
  (`cvs`, `cvsps`, `git-svn`, `git-filter-repo` are in the sandbox base image; `cvs-fast-export`
  builds from gitlab.com/esr/cvs-fast-export). If the CVS ever arrives, it would become the
  history *under* `v0.98-beta1` (or replace the release commits it covers) — another rewrite of
  `master`, so do it before publishing the repository widely.
- The **fork lineage after 2014** (The Secret Chronicles of Dr. M., github.com/Secretchronicles/TSC,
  forked 2012) is a different project and deliberately not merged.
