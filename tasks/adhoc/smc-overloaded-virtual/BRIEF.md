# Subagent brief — smc -Woverloaded-virtual, one directory each

Re-enabling `-Woverloaded-virtual` on smc. The maintainer decided (2026-10-18): un-hide the
no-ambiguity cases with `using`, and for the intentional/load-bearing hides (base overload has a
default arg overlapping the derived) **suppress narrowly with a reason**. You apply the exact edits
named in your launch prompt. You do NOT build.

## Absolute rules
- **Edit ONLY files under `smc/src/<YOUR-DIR>/`.** Never other dirs, never `third_party/`, never
  `configure.ac`. NO git/build/configure/clang-format. Read / Edit only. Behaviour-preserving.

## Two edit kinds

**(A) `using` — un-hide a base overload (Group 1, no ambiguity).** Add the exact line named in your
prompt to the class's `public:` section, e.g.:
```
  using cObject_Manager<cSound>::Get_Pointer;
```

**(B) suppress — accept an intentional hide (Group 2).** Wrap *only the hiding method declaration* in
a GCC-diagnostic pragma with a one-line reason. The `#pragma` lines go at column 0 (preprocessor);
keep the method's own indentation. Template (adapt the method decl + the base signature in the reason):
```
  // Intentionally hides <BASE SIGNATURE> — this is the convenience form; the base's defaulted
  // argument makes an un-hiding `using` ambiguous, so the hide is deliberate (maintainer, 2026-10-18).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
  <EXACT EXISTING METHOD DECLARATION LINE(S), unchanged>
#pragma GCC diagnostic pop
```
Copy the existing declaration verbatim between the push and pop — do not change its signature. If the
declaration spans two lines, include both. `#pragma GCC diagnostic` is understood by both gcc and clang.

## Report back
Per file: which edit kind you applied to which class/method, and the base signature you named in the
reason. Concise.
