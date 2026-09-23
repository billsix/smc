#!/usr/bin/env bash
# Per-version SourceForge release-folder listing (name | date | size) from Wayback snapshots.
set -u
S=${SMC_WORK:?set SMC_WORK to a scratch dir holding smc-tarballs/}
: > "$S/sf_versions_files.txt"
for v in 0.99.1 0.99.2 0.99.3 0.99.4 0.99.5 0.99.6 0.99.7 1.0 1.1 1.2 1.3 1.4 1.5 1.6 1.7 1.8 1.9; do
    snap=$(timeout 40 curl -s "http://archive.org/wayback/available?url=sourceforge.net/projects/smclone/files/Secret%20Maryo%20Chronicles/$v/" \
        | python3 -c 'import json,sys; d=json.load(sys.stdin); print(d.get("archived_snapshots",{}).get("closest",{}).get("url",""))')
    if [ -z "$snap" ]; then echo "$v | (no snapshot)" | tee -a "$S/sf_versions_files.txt"; continue; fi
    timeout 60 curl -sL "$snap" | python3 - "$v" <<'EOF' | tee -a "$S/sf_versions_files.txt"
import re, sys, html
v = sys.argv[1]; t = sys.stdin.read()
rows = []
for m in re.finditer(r'<tr[^>]*title="([^"]+)"[^>]*>(.*?)</tr>', t, flags=re.S):
    name = html.unescape(m.group(1)); row = m.group(2)
    if "{{" in name or name == "Parent folder":
        continue
    d = re.search(r'<abbr[^>]*title="([^"]+)"', row)
    sz = re.search(r'files_size_h"[^>]*>\s*([^<]+)<', row)
    rows.append(f"{v} | {name} | {(d.group(1) if d else '?')[:10]} | {(sz.group(1).strip() if sz else '?')}")
print("\n".join(rows) if rows else f"{v} | (snapshot parsed 0 rows)")
EOF
done
