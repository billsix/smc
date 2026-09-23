#!/usr/bin/env bash
# Redo the graft step of import_smc.sh (it failed on `branch -f` of the checked-out branch).
set -u
cd "$(git -C "$(dirname "$0")" rev-parse --show-toplevel)"
LAST=$(git rev-parse v1.9^{commit})
echo "== provenance chosen for 1.9:"; git log -1 --format=%B v1.9 | grep -A1 "^Source:" | head -2
git remote get-url github-fluxy >/dev/null 2>&1 || git remote add github-fluxy https://github.com/FluXy/SMC
git fetch -q github-fluxy
ROOT=$(git rev-list --max-parents=0 github-fluxy/master)
git tag -f import/github-fluxy-master github-fluxy/master >/dev/null
git replace --graft "$ROOT" "$LAST"
git checkout -q --detach
git branch -f master github-fluxy/master
git checkout -q master
git filter-repo --force --refs master --quiet 2>&1 | tail -1
git replace -d "$ROOT" >/dev/null 2>&1 || true
git remote get-url github-fluxy >/dev/null 2>&1 || git remote add github-fluxy https://github.com/FluXy/SMC
echo "== verification"
echo "  master commits: $(git rev-list --count master) (expect 211 + 12 = 223)"
echo "  master root: $(git log --format='%h %s' --max-parents=0 master)"
NEWGH=$(git log --format=%H --grep='initial smc commit with the changes from last SVN' master)
echo "  rewritten GitHub root: $NEWGH -> parent $(git rev-parse --short "$NEWGH^") (v1.9 = $(git rev-parse --short "$LAST"))"
echo "  tree(master) == tree(pristine GitHub master)? $(git diff --quiet import/github-fluxy-master master && echo YES || echo NO)"
echo "  tree(rewritten GitHub root) == tree(original GitHub root)? $(git diff --quiet "$ROOT" "$NEWGH" && echo YES || echo NO)"
echo "  merges preserved: $(git rev-list --merges --count master) (expect 8)"
echo "  tags: $(git tag | tr '\n' ' ')"
git log --format='%h %ad %an | %s' --date=short master | sed -n '1,3p;209,214p'
git status --short | head -2; du -sh .git | cut -f1
echo "GRAFT DONE"
