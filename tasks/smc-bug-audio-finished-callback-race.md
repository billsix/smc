# Bug: the SDL_mixer finished-channel callback mutates game state from the audio thread

**Status:** proposed — needs go-ahead (found in the source analysis, 2026-09-23)
**Priority:** 3 · **Difficulty:** 4 (a real data race; the fix is small but needs care)
**Do before:** `tasks/smc-modernize-cpp.md` (the modernization's audio step should build on the fix,
not inherit the race).

## BLUF

`Finished_Sound`, registered with `Mix_ChannelFinished`, walks the `std::vector m_active_sounds` and
writes to it with no lock, while the main thread `push_back`s/`erase`s/`delete`s the same vector.
SDL_mixer may call it from the audio thread. Replace the shared-vector walk with a lock-free
per-channel signal drained on the main thread, and register the callback once.

## Context

- Verified in `tasks/reference/smc/audio.md` (§6 #1).
- `smc/src/audio/audio.cpp`: `Finished_Sound` (~:29) iterates `pAudio->m_active_sounds` and sets
  `obj->m_channel = -1`. SDL2_mixer's contract (`SDL_mixer.h`): the callback "may be called from the
  mixer's audio callback." Meanwhile `Create_Sound_Channel` (~:653) `push_back`s (reallocation),
  `Close`/`Stop` `erase`/`delete` the same vector. `Mix_ChannelFinished( &Finished_Sound )` is
  **re-registered on every `cAudio_Sound::Play`** (~:114) instead of once.

## Fix

Register `Mix_ChannelFinished` once in `cAudio::Init`. Have the callback set only an atomic flag —
e.g. `std::array<std::atomic<bool>, N> m_channel_finished` (N = `m_max_sounds`) — or push the
channel number onto an SDL-mutex-protected queue. Drain it in `cAudio::Update` (main thread) to mark
pool entries free. Do not touch `m_active_sounds` from the callback.

## Verification

Container build green; headless smoke run stays clean (no crash); play many overlapping sounds
(a level with several enemies) and confirm channels recycle correctly and no use-after-free under a
sanitizer build if one is stood up. Note: the race is hard to trigger deterministically — the fix is
justified by construction (SDL_mixer's documented threading), not by reproducing a crash.

## Open questions

None.
