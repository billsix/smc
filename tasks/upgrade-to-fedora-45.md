# Upgrade the container base from Fedora 44 to Fedora 45 (done)

Bumped the base image to `fedora:45`. The blocker was that F45 removed PCRE1
(`pcre`/`pcre-devel`; only pcre2 10.47 remains), and the vendored **CEGUI 0.7.9**
used the PCRE1 C API for `CEGUIPCRERegexMatcher` — the regex engine behind editbox
input validation, which the SMC level editor uses (`moving_platform.cpp`,
`gee.cpp`, `flyon.cpp`, … all call `setValidationString`).

## Fix (decided by William Emerison Six <billsix@gmail.com>, 2026-11-06)
Backport CEGUI 0.8's approach: reimplement the matcher with **C++ `std::regex`**,
dropping the external PCRE dependency entirely (no pcre1, no pcre2). The patterns
in use (`^[+]?\d*$`, `[+]?[0-9]*\.?[0-9]*`) are plain ECMAScript, so std::regex is
a drop-in. Options weighed and rejected: port the matcher to PCRE2 (adds a dep this
avoids); vendor PCRE1 8.45 from source (keeps a dead library alive); upgrade the
whole vendored CEGUI to 0.8.7 (a major API port — deferred). Plain `--disable-pcre`
was not viable: `Editbox::setValidationString` then throws `InvalidRequestException`
and crashes the editor dialogs.

## Changes
- `Dockerfile`: `fedora:44` → `fedora:45`.
- `third_party/cegui-0.7.9/cegui/src/CEGUIPCRERegexMatcher.cpp` +
  `.../include/CEGUIPCRERegexMatcher.h`: body now uses `std::regex` /
  `std::regex_match` (full-string match = same semantics as the old PCRE length
  check). Class name/filename kept, so `CEGUIEditbox.cpp` and the matcher's
  `Makefile.am` entry are untouched.
- `third_party/cegui-0.7.9/acinclude.m4` `CEGUI_CHECK_PCRE`: no longer probes
  `libpcre`; defines `CEGUI_HAS_PCRE_REGEX`, forces `CEGUI_BUILD_PCRE_REGEX`
  true, empty `pcre_CFLAGS`/`pcre_LIBS`. The vendored tree ships no committed
  `configure`; `build.sh` re-bootstraps via autoconf/automake/libtool (all in the
  base image), so this takes effect.
- `entrypoint/01-install-base.sh`: dropped `pcre-devel`; updated the CEGUI-deps
  comment.

## Verification
`make image` nested: **EXIT=0** — CEGUI 0.7.9 compiled under GCC 16 with the
std::regex matcher and the game passed its `-Werror` gate, with **no** further
GCC-16 diagnostic fixes needed (the "will 2011-era CEGUI compile" unknown resolved
clean). Headless smoke (`tools/smoke_headless.sh`): **pass** (`game exit=137` = still
running at the 30 s deadline; `ps audit: clean`) — CEGUI initializes and the game
runs.

> The smoke exercises startup, not the editor dialogs, so the std::regex matcher's
> runtime behaviour (editbox validation) is confirmed to *link and initialize* but
> not yet play-tested interactively. A maintainer play-test of a level-editor
> object dialog (e.g. a moving platform's numeric fields) would exercise it end to
> end — same on-display caveat as other GUI work here.

## Follow-up (deferred until after this branch merges)
`tasks/add-dep-update-target-cegui.md` — add Craft-style `make update-cegui`
re-vendor machinery carrying this change as a `patches/cegui/` series, and the
future options for moving to a newer CEGUI (0.8's native `StdRegexMatcher` would
retire this patch, at the cost of a major API port).
