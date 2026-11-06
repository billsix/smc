# Upgrade the container base from Fedora 44 to Fedora 45

**Status:** in progress (code done; nested `make image` verification pending)
**Priority:** 4
**Difficulty:** 7

## BLUF
Base bumped to `fedora:45`. The blocker was that F45 removed PCRE1
(`pcre`/`pcre-devel`; only pcre2 10.47 remains), and the vendored **CEGUI 0.7.9**
used the PCRE1 C API for `CEGUIPCRERegexMatcher` (editbox input validation, which
the SMC level editor uses). **Decision (William Emerison Six <billsix@gmail.com>,
2026-11-06): option A — backport CEGUI 0.8's std::regex matcher into the vendored
0.7.9 tree**, dropping the external PCRE dependency entirely (no pcre1, no pcre2).
"Done" = `make image` builds nested on F45 (CEGUI + game compile under GCC 16) and
the headless smoke run passes.

## Why option A
PCRE1 is used in exactly one place — the regex matcher backing editbox
`setValidationString` (used by the editor's numeric fields in
`moving_platform.cpp`, `gee.cpp`, `flyon.cpp`, …). The patterns in use
(`^[+]?\d*$`, `[+]?[0-9]*\.?[0-9]*`) are plain ECMAScript, so C++ `std::regex`
is a drop-in. Upstream CEGUI 0.8 made the same move (`StdRegexMatcher`). Options
weighed and rejected: port the matcher to PCRE2 (adds an external dep A avoids);
vendor PCRE1 8.45 from source (keeps a dead library alive); upgrade the whole
vendored CEGUI to 0.8.7 (a major API port — deferred, see below). Plain
`--disable-pcre` is NOT viable: `Editbox::setValidationString` then throws
`InvalidRequestException`, crashing the editor dialogs.

## Changes made
- `Dockerfile`: `fedora:44` → `fedora:45`.
- `third_party/cegui-0.7.9/cegui/src/CEGUIPCRERegexMatcher.cpp` +
  `.../include/CEGUIPCRERegexMatcher.h`: reimplemented with `std::regex` /
  `std::regex_match` (full-string match = same semantics as the old PCRE length
  check). Class name/filename kept, so `CEGUIEditbox.cpp` and the matcher's
  `Makefile.am` entry are untouched.
- `third_party/cegui-0.7.9/acinclude.m4` `CEGUI_CHECK_PCRE`: no longer probes
  `libpcre`; defines `CEGUI_HAS_PCRE_REGEX`, forces `CEGUI_BUILD_PCRE_REGEX`
  true, empty `pcre_CFLAGS`/`pcre_LIBS`. (The vendored tree ships no committed
  `configure`; `build.sh` re-bootstraps via autoconf/automake/libtool, all in
  the base image, so this takes effect.)
- `entrypoint/01-install-base.sh`: dropped `pcre-devel`; updated the CEGUI-deps
  comment.

## Second unknown (being verified in the build)
CEGUI 0.7.9 is 2011-era C++ and had never compiled under GCC 16 / clang 23 here
(the build previously died at the dnf step). Expect `-Werror`-class diagnostics on
the CEGUI compile; fix at the source as they appear and record each here.

## Follow-up (deferred by the maintainer until after merge)
`tasks/add-dep-update-target-cegui.md` — add Craft-style `make update-cegui`
re-vendor machinery, carrying the above as a `patches/cegui/` series. It also
records the future options for moving to a newer CEGUI (0.8's native
`StdRegexMatcher` would retire our patch, but it's a major API port).
