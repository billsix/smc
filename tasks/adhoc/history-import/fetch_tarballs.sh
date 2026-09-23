#!/usr/bin/env bash
# Fetch every SMC source release we can reach: Debian's orig tarballs (11 upstream versions) and the
# one original SourceForge tarball the Wayback Machine holds bytes for (smc-1.9.tar.bz2), plus the
# two early zips it holds, for inspection.
set -u
S=${SMC_WORK:?set SMC_WORK to a scratch dir holding smc-tarballs/}
D="$S/smc-tarballs"; mkdir -p "$D"; cd "$D"
: > manifest.txt
for v in 0.99.6-1 0.99.7-1 1.0-1 1.1-1 1.2-1 1.3-1 1.4-1 1.6-1 1.7-1 1.8-1 1.9-1; do
    read -r name hash size < <(timeout 60 curl -s "https://snapshot.debian.org/mr/package/smc/$v/srcfiles?fileinfo=1" | python3 -c '
import json,sys
d=json.load(sys.stdin)
for h,infos in d.get("fileinfo",{}).items():
    for i in infos:
        if "orig" in i["name"]:
            print(i["name"], h, i["size"]); raise SystemExit')
    [ -n "${name:-}" ] || { echo "!! no orig for $v"; continue; }
    if [ ! -s "$name" ] || [ "$(stat -c %s "$name")" != "$size" ]; then
        timeout 900 curl -sL -o "$name" "https://snapshot.debian.org/file/$hash" || echo "!! download failed: $name"
    fi
    echo "$v $name $(stat -c %s "$name") sha1=$(sha1sum "$name" | cut -c1-40) expected=$hash" | tee -a manifest.txt
done
# original SourceForge tarball(s) via the Wayback Machine (id_ = raw bytes)
for u in "https://kumisystems.dl.sourceforge.net/project/smclone/Secret%20Maryo%20Chronicles/1.9/smc-1.9.tar.bz2" \
         "http://easynews.dl.sourceforge.net:80/sourceforge/smclone/SMC_0.98_Beta_1.zip" \
         "http://jaist.dl.sourceforge.net:80/sourceforge/smclone/SMC_0.97_game.zip"; do
    f=$(basename "$u")
    [ -s "$f" ] || timeout 900 curl -sL -o "$f" "http://web.archive.org/web/2id_/$u" || echo "!! wayback download failed: $f"
    echo "wayback $f $(stat -c %s "$f" 2>/dev/null) $(file -b "$f" | cut -c1-40)" | tee -a manifest.txt
done
echo "FETCH DONE"; ls -la "$D" | awk '{print $5, $9}'
