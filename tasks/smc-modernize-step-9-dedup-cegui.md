# Modernize step 9 — enemy dedup + CEGUI event lambdas

**Status:** assessed, deferred as its own task (William Emerison Six <billsix@gmail.com>,
2026-09-24). Two distinct jobs; both are large and behavioral. Resume deliberately.
**Priority:** 5 · **Difficulty:** 7
**Part of:** the SMC C++ modernization (umbrella archived at `tasks/archive/2026/09/24/smc-modernize-cpp.md`; overview in `tasks/reference/refactoring-to-modern-cpp.md`)

## BLUF

**(9a) Enemy dedup** — a `cWalkingEnemy` base (or folding `cTurtleBoss` onto `cTurtle`) to remove
duplication. **(9b) CEGUI event lambdas** — the ~216 `subscribeEvent` fire-and-forget callbacks →
lambdas with stored `Event::Connection`s disconnected in the destructor. Both are behavioral and
smoke-invisible (gameplay / GUI events), so they need careful, individually play-testable commits,
not autonomous batching at the tail of a long run — hence this is parked with the assessment below.

## Assessment (2026-09-24)

### 9a — enemy dedup: a real refactor, NOT a clean mechanical fold
`cTurtle` and `cTurtleBoss` BOTH derive from `cEnemy` directly (turtle.h, turtle_boss.h) and share
method *names* (`Init`, `Set_Direction`, `Turn_Around`, `DownGrade`, `Update_Dying`,
`Set_Turtle_Moving_State`, `Update`, `Stand_Up`, …), but the bodies differ: the boss adds
hit-counting (`Set_Max_Hits`), downgrade counts, shell time, `Set_Level_Ends_If_Killed`, and uses a
DIFFERENT state enum (`TurtleBoss_state` vs `Turtle_state`). So the "~22 duplicated methods" are
shaped alike but not identical — a correct dedup must extract the genuinely-common logic into a base
while preserving the boss's extra behavior, verifying each method by diff. Get it wrong and enemy
behavior changes (only caught in play-test). The reference (`tasks/reference/smc/object-model.md`,
[[enemies]]) frames the broader `cWalkingEnemy` base (wall/edge turn, standard downgrade, standard
dying) across the 13 enemies as "the single largest dedup opportunity" — bigger still.
**Recommended approach:** small, per-method, individually play-testable commits; start by
mechanically diffing `cTurtle` vs `cTurtleBoss` method bodies to find the truly-identical ones.

**Evidence the fold is NOT mechanical (diffed 2026-09-24):** e.g. `Stand_Up` differs by state enum
(`TURTLE_SHELL_STAND` vs `TURTLEBOSS_SHELL_STAND`), sound file (`stand_up.wav` vs `power_up.ogg`),
and target state; `Update_Velocity_Max` is entirely different (the boss scales speed by
`m_hits`/`m_downgrade_count`). So a shared base must UNIFY the state enums and PARAMETERIZE the
sounds/speeds — a real behavioral refactor, not extraction of identical code. Given the game is
play-test-verified working and this dedup is cleanliness-only, it is NOT worth the regression risk
without deliberate per-method work + play-testing. Left parked.

### 9b — CEGUI event lambdas: high-churn / modest-value (like the enums)
~216 `subscribeEvent` sites, all fire-and-forget (no `Event::Connection` stored or disconnected —
`tasks/reference/smc/gui.md`). Converting to lambdas + stored connections disconnected in the
destructor is cleanliness, not a known bug (nothing today dangles). Same profile as the deferred
enum-class work: large mechanical/behavioral churn for modest value. **Recommendation:** defer
unless the maintainer wants it; if done, it is a mechanical pass but touches every GUI callback.

## Open questions

1. 9a enemy dedup: do the contained `cTurtleBoss`→`cTurtle` fold, the full `cWalkingEnemy` base, or
   skip? (Behavioral; needs per-method diffing + play-test.)
2. 9b CEGUI lambdas: do the ~216-site conversion, or defer like the enums? (My lean: defer.)
