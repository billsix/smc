# Upgrade the container base from Fedora 44 to Fedora 45

**Status:** blocked
**Blocked on:** Fedora 45 removed PCRE1 (`pcre`/`pcre-devel`); the vendored
CEGUI 0.7.9 requires the PCRE1 API. Needs a maintainer decision on how to supply
PCRE to CEGUI (options below).
**Recheck:** after picking a PCRE path, `make image` nested must get past the
`01-install-base.sh` dnf step AND compile CEGUI 0.7.9 under GCC 16.
**Priority:** 4
**Difficulty:** 7

## BLUF
Base bumped to `fedora:45` (committed), but `make image` fails at the dnf step:
`No match for argument: pcre-devel`. Fedora 45 ships only **pcre2** (PCRE2
10.47); PCRE1 is gone. smc builds vendored **CEGUI 0.7.9** from source
(`entrypoint/build.sh`), which depends on PCRE1 (for `CEGUIPCRERegexMatcher`) via
`CEGUI_CHECK_PCRE` in its `configure.ac` with **no --disable-pcre option**.

## Fix options (maintainer call; none verifiable without a display)
1. **Vendor PCRE1 (8.45, final release) from source** in `build.sh` before the
   CEGUI configure, installing into $PREFIX so `CEGUI_CHECK_PCRE` finds it, and
   drop `pcre-devel` from `01-install-base.sh`. Smallest conceptual change;
   adds a from-source dep. (The user pre-authorized build-from-source as a last
   resort.)
2. **Patch CEGUI 0.7.9 to make PCRE optional / use std::regex** (configure.ac m4
   + the RegexMatcher source) and drop pcre-devel. More invasive.
3. **Port CEGUI 0.7.9 PCRERegexMatcher to PCRE2** (pcre2-devel is present).
   Most work; PCRE2 API differs.

## SECOND unknown (applies to all three)
Even with PCRE supplied, **CEGUI 0.7.9 (2011-era C++) may not compile under GCC
16 / clang LLVM 23** without patches — the build failed at the dnf step before
CEGUI compiled, so its GCC-16 compatibility is untested. Budget for new C++
diagnostics on the CEGUI compile too.

## What was done
- `Dockerfile`: `fedora:44` -> `fedora:45` (committed). No other changes;
  `pcre-devel` left in `01-install-base.sh` pending the decision.
