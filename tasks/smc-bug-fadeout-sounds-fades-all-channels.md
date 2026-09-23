# Bug: Fadeout_Sounds(filename) can fade every channel via channel −1

**Status:** DONE 2026-09-23 (branch `pre-modernization-fixes`). **Done:** `Fadeout_Sounds(ms,
filename)` now skips pool entries with `m_channel < 0` before the filename check, so an idle entry
no longer reaches `Mix_FadeOutChannel(-1, ms)` (channel -1 = all channels). The guard also prevents
a latent NULL deref, since a freed/idle entry can have a null `m_data`. Compiles green.
**Priority:** 4 · **Difficulty:** 2

## BLUF

`cAudio::Fadeout_Sounds(ms, filename)` does not skip idle pool entries (`m_channel == -1`), so a
recycled `cAudio_Sound` whose last chunk matched the filename reaches `Mix_FadeOutChannel(-1, ms)`,
and channel −1 means *all* channels. Guard the idle entries.

## Context

- Verified in `tasks/reference/smc/audio.md` (§6 #2).
- `smc/src/audio/audio.cpp`, `Fadeout_Sounds(ms, filename)` (~:781-798): iterates `m_active_sounds`
  and calls `Mix_FadeOutChannel( obj->m_channel, ms )` for a filename match without checking
  `obj->m_channel >= 0`. An idle entry has `m_channel == -1` → fades everything.
- Reached from `cRandom_Sound::Event_Out_Of_Range` (`audio/random_sound.cpp:~477`) when a
  random-sound object scrolls out of range after its sound has already ended.

## Fix

Skip entries with `m_channel < 0` before `Mix_FadeOutChannel` (and apply the same guard to the
sibling channel-based overload if it has the flaw). One `if( obj->m_channel < 0 ) continue;`.

## Verification

Container build green; place/scroll past a `cRandom_Sound` after its sound ends while other sounds
play, confirm only the matching channel fades (or nothing), not all audio.

## Open questions

None.
