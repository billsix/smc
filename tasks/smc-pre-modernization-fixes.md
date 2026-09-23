# Pre-modernization fixes — the index (do these before the C++ upgrade)

**Status:** proposed — needs go-ahead (William Emerison Six <billsix@gmail.com>, 2026-09-23: "make a
task for each of those things you found that you recommend fixing, before we do the C++ upgrade work")
**Priority:** 4 · **Difficulty:** — (umbrella; the work is in the child tasks)

## BLUF

The source analysis (`tasks/reference/smc/`) turned up real bugs and dead code that are worth fixing
on the current tree, before the big C++ modernization rewrites everything. This is the index of those
fix tasks. Each is independent and small (one-bug-per-commit), so they can land in any order and
give the eventual `git log` clean "fixed X" commits. The RAII / ownership / manager-collapse work is
NOT here — that *is* the modernization (`tasks/smc-modernize-cpp.md`); this index is only the
standalone fixes that make sense first.

## The fix tasks (child tasks)

**Real bugs** (crash / wrong behaviour), one task each:

| Task | Priority | What |
|---|---|---|
| `smc-bug-joystick-name-click-crash.md` | 3 | clicking the Options "Joystick" label → uncaught CEGUI exception → terminate (wrong window name) |
| `smc-bug-audio-finished-callback-race.md` | 3 | SDL_mixer finished-channel callback mutates a `std::vector` from the audio thread (data race) |
| `smc-bug-fadeout-sounds-fades-all-channels.md` | 4 | `Fadeout_Sounds(filename)` hits channel −1 (= all channels) via idle pool entries |
| `smc-bug-toggle-sounds-silences-music.md` | 4 | toggling sound effects off/on kills the music until the next `Play_Music` |
| `smc-bug-cape-to-small-downgrade-size.md` | 5 | CAPE→SMALL downgrade skips `Change_Size` (a condition that can never be true) |
| `smc-bug-jump-sprite-wrong-for-four-costumes.md` | 5 | fire/ice/flying/ghost Maryo shows the fall sprite while jumping |
| `smc-bug-texture-downscale-aspect-ratio.md` | 6 | textures over `GL_MAX_TEXTURE_SIZE` are squashed (integer scale) |
| `smc-bug-command-line-argument-parsing.md` | 6 | `main()` arg loop `option = i + i` and `arguments[1]` typos |

**Cleanup / latent:**

| Task | Priority | What |
|---|---|---|
| `smc-remove-dead-code.md` | 6 | delete the verified no-caller functions/members/globals (the README ledger) |
| `smc-fix-latent-correctness-smells.md` | 6 | ten small one-line correctness bugs not visible today (string_trim, off-by-ones, null-derefs, …) |

## Notes / decisions

- Every child task is grounded in `tasks/reference/smc/` and re-verifies its finding against current
  source before touching it (a 2026-09-23 lead is not proof at fix time).
- Ordering: pick by priority number, easy wins first (per the triage convention). The two crash/race
  tasks (P3) are the ones actually worth doing promptly; the rest are polish.
- Coordinate `smc-remove-dead-code.md` with `smc-modernize-cpp.md` step 3 (both touch the dead
  `SMC_RENDER_THREAD_TEST` / `boost::thread` block) so it is removed once.
- Archive each child on its own completion; archive this umbrella when the last child lands.

## Open questions

1. Do you want these fixed as a batch before starting the modernization, or interleaved? (The index
   assumes "before"; either works since each is independent.)
