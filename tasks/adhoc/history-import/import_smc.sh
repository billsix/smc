#!/usr/bin/env bash
# Build /foo/opt/smc: one commit per SourceForge source release (from the tarballs fetched into
# smc-tarballs/), then the FluXy/SMC GitHub history grafted on top of the last release, so `master`
# is one linear history 2007 -> 2014 and the pristine GitHub commits stay reachable under a tag.
set -u
S=${SMC_WORK:?set SMC_WORK to a scratch dir holding smc-tarballs/}
T="$S/smc-tarballs"; R=$(git -C "$(dirname "$0")" rev-parse --show-toplevel); W=$(mktemp -d)
AUTHOR="Florian Richter <fluxey@gmail.com>"                  # SMC's author (the GitHub history's author identity)
NOSIGN=(-c commit.gpgsign=false)                              # per command; nothing is written to the repo config
# version -> SourceForge release-folder date (Wayback snapshot of the files listing, 2026-06-19)
declare -A DATE=([0.99.6]="2007-04-20 17:00:37 +0000" [0.99.7]="2007-06-15 14:24:00 +0000" [1.0]="2007-07-28 09:03:31 +0000"
  [1.1]="2007-09-21 14:19:28 +0000" [1.2]="2007-10-18 17:04:04 +0000" [1.3]="2007-12-10 20:23:09 +0000" [1.4]="2008-01-10 09:30:28 +0000"
  [1.6]="2008-11-04 02:08:51 +0000" [1.7]="2009-01-04 00:27:00 +0000" [1.8]="2009-04-10 15:20:45 +0000" [1.9]="2009-09-02 16:22:37 +0000")
DATE[0.98-beta1]="2006-03-30 00:00:00 +0000"   # no SF folder date survives; the archive's newest file timestamp
ORDER=(0.98-beta1 0.99.6 0.99.7 1.0 1.1 1.2 1.3 1.4 1.6 1.7 1.8 1.9)

echo "== 0. sanity: Debian's 1.9 orig vs the original SourceForge smc-1.9.tar.bz2"
mkdir -p "$W/deb19" "$W/sf19"
tar -xzf "$T/smc_1.9.orig.tar.gz" -C "$W/deb19" && tar -xjf "$T/smc-1.9.tar.bz2" -C "$W/sf19"
d1=$(ls "$W/deb19"); d2=$(ls "$W/sf19"); echo "  top dirs: debian=$d1 sf=$d2"
if diff -rq "$W/deb19/$d1" "$W/sf19/$d2" > "$W/diff19.txt"; then echo "  IDENTICAL trees (Debian orig == SourceForge original)"; SAME19=yes
else echo "  DIFFER: $(wc -l < "$W/diff19.txt") entries (first 10):"; head -10 "$W/diff19.txt"; SAME19=no; fi

echo "== 1. repository"
[ -e "$R/.git" ] && { echo "$R is already a git repo; refusing" >&2; exit 2; }
[ -z "$(ls -A "$R" 2>/dev/null)" ] || { echo "$R is not empty; refusing" >&2; exit 2; }
git init -q -b master "$R"
cd "$R"
for v in "${ORDER[@]}"; do
    src="$T/smc_$v.orig.tar.gz"; kind="Debian snapshot.debian.org orig tarball (smc_$v.orig.tar.gz)"
    if [ "$v" = 1.9 ] && [ "$SAME19" = yes ]; then src="$T/smc-1.9.tar.bz2"; kind="SourceForge original smc-1.9.tar.bz2 (via the Wayback Machine; byte-identical tree to Debian's orig)"; fi
    rm -rf "$W/x"; mkdir -p "$W/x"
    if [ "$v" = 0.98-beta1 ]; then
        src="$T/SMC_0.98_Beta_1.zip"; kind="SourceForge original SMC_0.98_Beta_1.zip (via the Wayback Machine; a combined source + Windows-binary release, imported as released; the date is the archive's newest file timestamp, no release-folder date survives)"
        unzip -q "$src" -d "$W/x"; top="(zip root)"
        rm -rf smc; mkdir smc; cp -a "$W/x/." smc/
    else
    case "$src" in *.bz2) tar -xjf "$src" -C "$W/x";; *) tar -xzf "$src" -C "$W/x";; esac
    top=$(ls "$W/x"); [ "$(ls "$W/x" | wc -l)" = 1 ] || { echo "!! $v: tarball has $(ls "$W/x" | wc -l) top entries"; exit 2; }
    rm -rf smc; mkdir smc; cp -a "$W/x/$top/." smc/
    fi
    git add -A
    GIT_AUTHOR_NAME="Florian Richter" GIT_AUTHOR_EMAIL="fluxey@gmail.com" GIT_AUTHOR_DATE="${DATE[$v]}" GIT_COMMITTER_DATE="${DATE[$v]}" \
      git "${NOSIGN[@]}" commit -q -m "Secret Maryo Chronicles $v (SourceForge release, ${DATE[$v]%% *})

Imported from the release tarball, not from version control: the project's
SourceForge CVS is no longer reachable (rsync and pserver refused since 2017's
read-only conversion) and its later SVN host (opensvn.csie.org) is gone, so the
pre-2010 history is reconstructed at release granularity, one commit per
SourceForge source release, dated by the release folder's SourceForge date.

Source: $kind
sha1: $(sha1sum "$src" | cut -c1-40)
Tarball top directory '$top' imported as 'smc/' to match the layout of the
author's later git repository (github.com/FluXy/SMC), which continues this
history." || { echo "!! commit $v failed"; exit 2; }
    git tag -a "v$v" -m "Secret Maryo Chronicles $v" $(git rev-parse HEAD) 2>/dev/null || git tag "v$v"
    echo "  $v: $(git rev-parse --short HEAD) $(git ls-files | wc -l) files"
done
LAST=$(git rev-parse HEAD)

echo "== 2. GitHub history on top"
git remote add github-fluxy https://github.com/FluXy/SMC
git fetch -q github-fluxy
ROOT=$(git rev-list --max-parents=0 github-fluxy/master); echo "  github root: $ROOT ($(git log -1 --format=%s $ROOT | cut -c1-60))"
git tag import/github-fluxy-master github-fluxy/master            # the pristine GitHub SHAs stay reachable here
echo "  delta SourceForge 1.9 -> GitHub root (the lost SVN r1642..1641 era):"; git diff --shortstat "$LAST" "$ROOT"
git replace --graft "$ROOT" "$LAST"
git branch -f master github-fluxy/master
git filter-repo --force --refs master --quiet 2>&1 | tail -2
git replace -d "$ROOT" 2>/dev/null || true
git remote add github-fluxy https://github.com/FluXy/SMC 2>/dev/null || true
echo "== 3. verification"
NEWROOT=$(git rev-list --max-parents=0 master); echo "  master root now: $NEWROOT (should be the first release)"
echo "  master commits: $(git rev-list --count master) (expect 211 + ${#ORDER[@]})"
echo "  tree(master) == tree(pristine GitHub master)? $(git diff --quiet import/github-fluxy-master master && echo YES || echo NO)"
echo "  GitHub root now sits on v1.9? $(git log --format=%h -1 "$(git rev-list master | tail -n +$((211)) | head -1)")"
git log --format="%h %ad %an | %s" --date=short master | tail -14
git status --short | head -3; echo "IMPORT DONE"
