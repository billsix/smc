# Subagent brief — smc -Wsign-conversion fixes, one directory each

You are silencing `-Wsign-conversion` in **one** directory of smc's first-party C++ game
(`smc/src/<DIR>/`), behavior-preservingly. Reviewed and built centrally by the coordinator — you do
NOT build.

## Absolute rules
- **Edit ONLY files whose path is under `smc/src/<YOUR-DIR>/`.** Never other dirs, never
  `third_party/`, never `configure.ac`.
- **NO git, NO build/make/configure, NO clang-format, NO shell mutation.** Read / Grep / Edit only.
- **Behaviour-preserving, guaranteed.** The fix makes an *implicit* integer conversion the compiler
  is ALREADY doing into an *explicit* `static_cast` — the runtime result is identical; you are only
  documenting it so the warning goes away. If a fix would change a value, you are doing it wrong.

## The fix
The gcc log is `tasks/adhoc/smc-sign-conversion-sweep/data/signconv-g.log` (repo-root-relative). Grep
it for your directory, e.g.
`grep -nE '/<DIR>/[a-z_]+\.(cpp|h):' tasks/adhoc/smc-sign-conversion-sweep/data/signconv-g.log`
(paths carry `../` noise; match the final `<DIR>/<file>:line`). Each hit reads like:
`…/<DIR>/foo.cpp:123:45: warning: conversion to 'unsigned int' from 'int' may change the sign […] [-Wsign-conversion]`
with a caret line under the offending expression.

At each site, wrap the **operand being converted** in `static_cast<DEST>( … )`, where DEST is the
"conversion to 'DEST'" type named in the warning:
- `unsigned int n = some_int;`            → `unsigned int n = static_cast<unsigned int>(some_int);`
- `int i = some_unsigned;`                → `int i = static_cast<int>(some_unsigned);`
- `int x = some_Uint32;`                  → `int x = static_cast<int>(some_Uint32);`
- `vec[signed_idx]` (warns converting the index to size_t/unsigned) → `vec[static_cast<size_t>(signed_idx)]`
- in an expression `a + b` where one operand converts → wrap just that operand, parenthesized, so
  operator precedence is unchanged: `foo(static_cast<unsigned int>(a) + b)`.

Guidance:
- **Cast at the boundary; do NOT retype the variable or change a declaration** (don't turn an `int`
  member/local into `unsigned` — that is a different, riskier change). Wrap the expression only.
- Wrap the **minimal** sub-expression the caret points at, and keep existing parentheses valid.
- Use `static_cast<T>(…)`, never a C-style `(T)` cast.
- Match the exact DEST spelling from the warning (`unsigned int`, `int`, `size_t`, `Uint32`, etc.);
  `std::size_t`/`size_t` are fine as written in the file's style.
- If a site is genuinely unclear or wrapping would change behaviour, SKIP it and note it — don't guess.

## Report back
Per file: how many sites you cast, and any you skipped (with why). Keep it concise.
