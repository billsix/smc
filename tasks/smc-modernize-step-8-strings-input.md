# Modernize step 8 (residue) — string formatting + per-frame input struct

**Status:** deferred / parked (William Emerison Six <billsix@gmail.com>, 2026-09-24). The LOOPS
part of step 8 (range-for) and the `sprintf`→`snprintf` HUD safety fix already landed; this task is
the remaining, lower-value/behavioral residue.
**Priority:** 7 · **Difficulty:** 5
**Part of:** the SMC C++ modernization — umbrella archived at
`tasks/archive/2026/09/24/smc-modernize-cpp.md`; overview in
`tasks/reference/refactoring-to-modern-cpp.md`.

## BLUF

Two leftovers from the plan's step 8, both deliberately not done in the main run: **(a) string
formatting** — `int_to_string`/`ostringstream` → `std::to_string`/`std::format`; and **(b) the
once-per-frame `PlayerInput` struct** — sample input once per frame into a struct passed to
`cLevel_Player::Update`, removing the ~26 scattered poll sites. Done = builds + smoke green, and the
game still plays (both are gettext/behavioral-sensitive — see below).

## Why deferred (not a clean win)

- **`int_to_string`/`ostringstream` → `std::to_string`/`std::format`:** ~277 sites of low-value
  churn (`int_to_string` in `core/property_helper` works fine). Worse, the printf-style format
  strings that WOULD suit `std::format` come from gettext `_()` (e.g. `"Points %08d"` — already
  converted to `snprintf`), so `std::format`'s `{}` syntax would break the translation catalogs. A
  safe conversion is mostly `int_to_string(x)` → `std::to_string(x)` (trivial but voluminous); the
  `ostringstream` and `%`-format cases need per-site judgment.
- **The `PlayerInput` struct** is behavioral: it changes when/where input is read (the reference
  notes `Validate_Collision` even reads input mid-collision). Sampling once per frame could change
  input timing/feel, so it needs the maintainer's play-test, done in small commits.

## Plan (if picked up)

- [ ] `int_to_string` → `std::to_string` where it's a plain int→string (codemod, word-boundary);
      leave the gettext `%`-format and `ostringstream` cases or handle per-site.
- [ ] `PlayerInput` struct: sample once per frame, pass to `cLevel_Player::Update`, remove the ~26
      poll sites and the `Validate_Collision` input read. Small, play-tested commits.

## Open questions

1. Is the `int_to_string`→`std::to_string` churn (~277 sites) worth it, given the helper works and
   the gettext cases can't use `std::format`? (My lean: skip unless you want it.)
2. The `PlayerInput` refactor changes input timing — do it (with play-test) or leave the current
   per-site polling?
