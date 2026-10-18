# Subagent brief — smc warning-gate fixes (Wave 1), one directory each

You are fixing compiler warnings in **one** directory of smc's first-party C++ game
(`smc/src/<DIR>/`). smc is Secret Maryo Chronicles, a 2006-era C++ SDL2 game. The goal is a
behaviour-preserving pass that drives a raised warning gate to zero. Your edits are reviewed and
built centrally by the coordinator — you do NOT build.

## Absolute rules
- **Edit ONLY files under `smc/src/<YOUR-DIR>/`.** Never touch other directories, never
  `smc/src/core/obj_manager.h` / `objects/movingsprite.h` / `objects/sprite.h` unless that IS your
  dir, never `third_party/`, never `configure.ac`.
- **NO git, NO build/make/configure, NO clang/gcc, NO clang-format, NO shell mutations.** You only
  Read / Grep / Edit source files. (If you have those tools, do not use them.)
- **Behaviour-preserving only.** Renames and `using`-declarations and casts that change no runtime
  behaviour. If a fix would change behaviour, skip it and note it in your report.
- Do **not** touch `-Wsign-conversion` / integer-conversion issues — a separate task owns those.

## Where the warnings are
The full gcc warning log is at `tasks/adhoc/smc-const-sweep-and-warning-gate/data/gate-g.log`
(repo-root-relative). Grep it for your directory, e.g.
`grep -nE '/(smc/src/)?<DIR>/[a-z_]+\.(cpp|h):' tasks/adhoc/smc-const-sweep-and-warning-gate/data/gate-g.log`
(paths in the log carry `../` noise — match on the final `<DIR>/<file>` segment). Each warning line
gives `file:line:col: warning: … [-Wclass]`; `-Wshadow` is followed by a `note: shadowed declaration
is here` pointing at what is shadowed.

## What to fix (only the classes below; ignore others)

1. **`-Wshadow`** (the bulk): a local variable or parameter shadows an outer local, a member, or a
   global. **Rename the INNER one** (the local/parameter the warning points at) to a clear,
   non-shadowing name that fits smc's style (the game uses `cFoo` types, `m_` members, lower_snake or
   lowerCamel locals — match the file). Update every use of that inner name **within its scope only**.
   Never rename the member/outer/global. If the shadow is a parameter, rename the parameter and its
   uses in that function body. Keep it readable (not `x2`); e.g. a local `cSprite* sprite` shadowing a
   member `sprite` becomes `cSprite* p_sprite` or a more specific name.

2. **`-Woverloaded-virtual`**: a derived class declares a function that hides a base `virtual` of the
   same name but different signature. Fix by adding a `using <Base>::<name>;` in the derived class's
   `public:` section (un-hides the base overload; behaviour-preserving). The exact declarations to add
   for files in YOUR dir are given in your launch prompt. Add nothing if none are listed for your dir.

3. **Small classes** (only if the grep shows them in your dir):
   - `-Wsign-compare`: a signed/unsigned comparison — cast the signed side at the comparison, e.g.
     `static_cast<size_t>(i) < v.size()`, choosing the cast that preserves the current result. Do NOT
     retype the variable.
   - `-Wunused-variable`: if the variable is genuinely dead, remove it; if it is used only in a
     `assert`/debug build, mark it `[[maybe_unused]]`. Prefer removal only when clearly dead.
   - `-Wdeprecated-enum-enum-conversion`: an arithmetic op mixing two enum types — `static_cast<int>`
     each operand, preserving the value.
   - `-Warray-bounds`: inspect — it may be a real bug; if so, note it loudly in your report and make
     the minimal correct fix; do not paper over it.
   - `-Wformat-nonliteral`: a `printf`-family call with a non-literal format — if the format is a
     controlled internal string, you may wrap with a `#pragma GCC diagnostic ignored
     "-Wformat-nonliteral"` around just that call WITH a one-line reason; prefer passing a literal if
     trivially possible.

## Report back
List every file you changed and, per warning class, how many sites you fixed and anything you
skipped (with why). Flag any `-Warray-bounds` as a possible real bug. Keep it concise.
