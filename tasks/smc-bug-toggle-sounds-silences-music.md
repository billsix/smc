# Bug: toggling sound effects off/on silences the music

**Status:** proposed — needs go-ahead (found in the source analysis, 2026-09-23)
**Priority:** 4 · **Difficulty:** 2
**Do before:** `tasks/smc-modernize-cpp.md`.

## BLUF

`cAudio::Toggle_Sounds` calls `Init`, which unconditionally `Close`s the device (freeing the current
music), but unlike `Toggle_Music` it never replays `m_music_filename` afterward — so music stays
silent until the next `Play_Music`. Restart the music after a sound toggle (or make `Init` not tear
down music it does not need to).

## Context

- Verified in `tasks/reference/smc/audio.md` (§6 #3).
- `smc/src/audio/audio.cpp`: `Toggle_Sounds` (~:694-704) → `Init` → unconditional `Close()` (~:175)
  frees `m_music` (~:337). After re-init `m_music` is NULL and nothing replays it. `Toggle_Music`
  (~:688-691) *does* replay `m_music_filename`; `Toggle_Sounds` does not.
- Callers: F10 (`input/keyboard.cpp:~265`) and the options menu (`gui/menu_data.cpp:~2068,2688`).

## Fix

After the `Init` in `Toggle_Sounds`, if music is enabled and `m_music_filename` is set, replay it
(mirror `Toggle_Music`'s restart, with a fade-in). Alternatively, make `cAudio::Init` avoid the
`Close` when only the sound-enable flag changed and the device spec is unchanged — but the targeted
replay is the smaller, safer fix.

## Verification

Container build green; with music playing, toggle sound effects off then on (F10 or the options
menu) and confirm the music keeps playing / resumes.

## Open questions

None.
