# `src/audio/` — verified source analysis

Scope: the game's own audio code under `smc/src/audio/` (6 files, 2228 lines). SDL_mixer is the
seam the game calls, not analysed. Build pins SDL2 + SDL2_mixer (`configure.ac:52` `sdl2 >= 2.0.0`,
`configure.ac:67` `AC_CHECK_LIB([SDL2_mixer], [Mix_OpenAudio]`; commit `d438165 update to SDL2`).
Every `Mix_*` call in the game lives in this directory (`rg -n 'Mix_[A-Z]' src | grep -v src/audio/`
= 0 hits), so this directory *is* the whole SDL_mixer seam. Paths below are relative to `smc/src/`.

## 1. Files and roles

| File | Lines | Role | Status |
|---|---|---|---|
| `audio/audio.h` | 227 | `enum AudioChannel` (resource ids), `cAudio_Sound` (one per mixer channel), `cAudio` (engine), `extern pAudio` | live |
| `audio/audio.cpp` | 1008 | the above + the free function `Finished_Sound` (channel-finished callback) + `pAudio` definition | live |
| `audio/sound_manager.h` | 98 | `cSound` (a `Mix_Chunk` + filename), `cSound_Manager : cObject_Manager<cSound>` (cache), `extern pSound_Manager` | live; `Delete_Sounds`, `m_load_count`, both `operator[]` dead (section 6) |
| `audio/sound_manager.cpp` | 113 | implementations | live |
| `audio/random_sound.h` | 152 | `cRandom_Sound : cSprite` — a level/world object that plays a positional ambient sound | live: instantiated `level/level.cpp:1837`, `overworld/overworld.cpp:1234`; editor lists it `core/editor.cpp:1275` (`TYPE_SOUND = 60`, `core/global_game.h:276`) |
| `audio/random_sound.cpp` | 630 | implementation incl. XML load/save + 9 CEGUI editor callbacks | live |

Nothing in the directory is vestigial as a whole; dead members are itemised in section 6.

## 2. `cAudio` (`audio/audio.h:86`, `audio/audio.cpp:133-1000`)

**Lifecycle.** `pAudio = new cAudio()` at `core/main.cpp:252`; `pAudio->Init()` at `core/main.cpp:296`
(after video/SDL init), re-`Init()` from `user/preferences.cpp:438` (`cPreferences::Apply_Audio`),
and `Close()`+`Init()` from `gui/menu_data.cpp:2627-2629` on a sample-rate change; `delete pAudio`
at `core/main.cpp:358`. `pSound_Manager` is created *after* `pAudio` (`main.cpp:259`) and deleted after it
(`main.cpp:376`) — fine because `cAudio::Close` never touches the manager.

**Init (`audio.cpp:158`).** Reads `pPreferences->m_audio_sound/m_audio_music/m_audio_hz`. Early-out
if `Mix_QuerySpec` says the device is open and both enable flags and the frequency are unchanged
(`audio.cpp:170`). Otherwise **unconditionally `Close()`** (`audio.cpp:175`), returns success if both
disabled, then opens the device:
`Mix_OpenAudio( pPreferences->m_audio_hz, MIX_DEFAULT_FORMAT, m_audio_channels /*MIX_DEFAULT_CHANNELS*/, m_audio_buffer /*4096*/ )`
(`audio.cpp:199`; constants set in the ctor `audio.cpp:149-150`, comment "below 2048 can be choppy").
It then warns via `printf` on any frequency/format/channel mismatch (`audio.cpp:213-256`) and sets
`m_initialised = 1`. The three flags: `m_initialised` = device open; `m_music_enabled` /
`m_sound_enabled` = per-feature toggles set in `audio.cpp:269-300`. Enabling sound calls
`Set_Max_Sounds()` (default 10, clamped ≥5, `Mix_AllocateChannels(10)`, `audio.cpp:356-391`) — the only
call site (`audio.cpp:290`), so the channel count is effectively a constant 10. Volumes are applied via
`Set_Music_Volume(m_music_volume)` / `Set_Sound_Volume(m_sound_volume)` (`audio.cpp:274,292`);
`m_sound_volume`/`m_music_volume` are public `Uint8` fields seeded from
`cPreferences::m_*_volume_default` (`audio.cpp:141-142`) and written directly by
`user/preferences.cpp:303-304,591,600` and `gui/menu_data.cpp:2669,2701` (the menu sliders also call
`Set_*_Volume`, `menu_data.cpp:2667,2699`). If `SDL_InitSubSystem(SDL_INIT_AUDIO)` failed
(`video/video.cpp:239-242`), preferences force both flags off before calling `Init`
(`user/preferences.cpp:427-431`).

**Close (`audio.cpp:305`).** Halts all channels, `delete`s every `cAudio_Sound`, `Mix_AllocateChannels(0)`,
halts + `Mix_FreeMusic`s `m_music` and `m_music_old`, `Mix_CloseAudio()`, clears all three flags.
`m_music_filename` is deliberately kept so `Toggle_Music` can restart it (`audio.cpp:688-691`).

**Sound playback.** One overload: `Play_Sound( std::string filename, int res_id=-1, int volume=-1, int loops=0 )`
(`audio.h:106`, `audio.cpp:440`). Flow: prepend `DATA_DIR "/" GAME_SOUNDS_DIR "/"` if the bare path
does not exist (`audio.cpp:448-462`, `File_Exists` = a stat, called twice) → `Get_Sound_File`
(`audio.cpp:393`: linear lookup in `pSound_Manager`, else `new cSound` + `Load` + `Add`) →
`Create_Sound_Channel` (`audio.cpp:653`: first `cAudio_Sound` with `m_channel < 0` is recycled via
`Free()`; if none and `size() < m_max_sounds`, `new cAudio_Sound` pushed to `m_active_sounds`; else
returns NULL and the sound is silently dropped, `audio.cpp:476-480`) → `sound->Load(data); sound->Play(res_id, loops)`
→ `Mix_Volume(channel, volume)` with `volume` clamped to `m_sound_volume` when `<0` or `>MIX_MAX_VOLUME`
(`audio.cpp:500-513`). So the per-channel object pool grows lazily to at most 10 and is never
shrunk except by `Close`.
`cAudio_Sound::Play` (`audio.cpp:82`): if `use_res_id >= 0`, every other pool entry with the same
`m_resource_id` is `Stop()`ped (`Mix_HaltChannel`) — that is what the `RID_*` ids in `enum AudioChannel`
(`audio.h:28-45`) mean ("sounds which shouldn't be played multiple times at the same time"; note
`RID_FIREPLANT`/`RID_MUSHROOM*`/`RID_FEATHER` all share id 6 and `RID_1UP_MUSHROOM`/`RID_MOON` share 7).
Then `m_channel = Mix_PlayChannel( -1, chunk, loops )` and **`Mix_ChannelFinished( &Finished_Sound )` is
re-registered on every play** (`audio.cpp:114`). `Finished_Sound` (`audio.cpp:29`) walks
`pAudio->m_active_sounds` and sets `m_channel = -1` on the entry owning that channel — the pool's
"free" marker. The enum's type name `AudioChannel` is never used; ids travel as `int`.

**Music.** `Play_Music( filename, loops=0, force=1, fadein_ms=0 )` (`audio.cpp:519`): prefixes
`DATA_DIR/GAME_MUSIC_DIR`, stores `m_music_filename` *before* the enabled check (`audio.cpp:534-539`, so
the filename survives audio-off), `Resume_Music()`, then:
- forced or nothing playing (`audio.cpp:545`): halt + free `m_music` and `m_music_old`, `Mix_LoadMUS`,
  `Mix_PlayMusic` or `Mix_FadeInMusic`.
- else (queue-after-current, `audio.cpp:590-611`): the playing `Mix_Music*` moves to `m_music_old`
  (or, if a queued one already exists, the queued one is freed) and the new file is loaded into `m_music`
  but not started. `Update()` (`audio.cpp:977`, called from the main loop `core/main.cpp:693` and five
  blocking loops: `level/level_manager.cpp:437`, `objects/level_exit.cpp:328`, `objects/level_entry.cpp:288`,
  `objects/text_box.cpp:238`, `core/camera.cpp:169`) polls `!Mix_PlayingMusic() && m_music` and starts it
  with **`Mix_PlayMusic( m_music, 0 )`** — the caller's `loops` and `fadein_ms` are discarded; looping
  is emulated by the poll restarting the track each time it ends (a frame-length gap).
`Fadeout_Music` (`audio.cpp:802`) refuses to double-fade unless `overwrite_fading`, and if the music is
fading *in* it halts instead (comment `audio.cpp:829`: "Can't stop fade-in with SDL_Mixer and fade-out is
ignored when fading in"). `Pause_Music`/`Resume_Music` wrap `Mix_PauseMusic`/`Mix_ResumeMusic`
(`audio.cpp:706,731`); the main loop calls `Resume_Music()` every frame (`core/main.cpp:692`) and
`Pause_Music()` on window focus loss (`core/main.cpp:547`). There is no `Mix_HookMusicFinished`; "music
finished" is detected only by the `Update()` poll above. `Toggle_Music`/`Toggle_Sounds`
(`audio.cpp:682-704`) flip the preference, call `Init()`, then replay `m_music_filename` with a 2 s
fade-in / play `audio_on.ogg`.

## 3. `cSound_Manager` (`audio/sound_manager.h:57`)

`cSound` (`sound_manager.h:30`) owns one `Mix_Chunk*` (`Mix_LoadWAV` in `Load`, `Mix_FreeChunk` in
`Free`/dtor, `sound_manager.cpp:33-57`). `cSound_Manager` derives from `cObject_Manager<cSound>`
(`core/obj_manager.h:27`), which is a public `vector<T*> objects` with virtual `Add/Delete/Delete_All/
Get_Pointer(size_t)`. The manager adds `Get_Pointer(const std::string &path)` — a linear
`std::string::compare` scan (`sound_manager.cpp:73-88`); declared `virtual` but it is an overload that
**hides** the base `Get_Pointer(size_t)`, which is why `operator[](unsigned)` re-qualifies the base
(`sound_manager.h:71-74`). Ownership: `cAudio::Get_Sound_File` allocates and `Add`s; the manager's dtor
runs `Delete_All()` (`sound_manager.cpp:70`) which `delete`s every chunk. Reload after a sample-rate
change is done by the caller: `pAudio->Close(); pSound_Manager->Delete_All(); pAudio->Init(); Preload_Sounds();`
(`gui/menu_data.cpp:2627-2631`, with the in-code `// todo : add sound manager function to reload sounds`).
`Preload_Sounds` (`core/game_core.cpp:486-610`) warms the cache with 60 hard-coded filenames at startup
(`core/main.cpp:342`). `cAudio_Sound::m_data` is a non-owning pointer into this cache; nothing ever
evicts a single entry, so the pointers stay valid until `Delete_All` — which is only ever called right
after `pAudio->Close()` has deleted every `cAudio_Sound`, so no dangling use occurs today.

## 4. Callers

`rg -o 'pAudio->[A-Za-z_]+' src | cut -d: -f2 | sort | uniq -c`: 91 `Play_Sound`, 11 `Play_Music`,
9 `Fadeout_Music`, 6 `Update`, 3 each `Toggle_Music/Toggle_Sounds/Is_Music_Playing/Init`, 2 each
`Resume_Music/Set_Sound_Volume/Get_Playing_Sound`, 1 each `Close/Stop_Sounds/Pause_Music/Is_Music_Fading/
Get_Sound_File/Set_Music_Volume/Fadeout_Sounds`; plus direct field access: `m_music_enabled` 7,
`m_sound_enabled` 6, `m_sound_volume` 5, `m_music_volume` 5, `m_music_filename` 1 (`level/level.cpp:495`).
Per-file `pAudio->` counts (`rg -n 'pAudio->' src | cut -d: -f1 | sort | uniq -c`): `level/level_player.cpp` 32,
`gui/menu_data.cpp` 22, `enemies/bosses/turtle_boss.cpp` 11, `user/preferences.cpp` 9, `enemies/furball.cpp` 7,
`objects/bonusbox.cpp` 6, `enemies/turtle.cpp` 6, `core/main.cpp` 6, `level/level.cpp` 5, `gui/menu.cpp` 5,
`audio/random_sound.cpp` 5, then 28 more files with 1-4 each (38 files total).
`pSound_Manager` is touched outside `audio/` only by `core/main.cpp` (new/delete) and `gui/menu_data.cpp:2628`.

Play_Sound arguments: **every** sound is a raw filename string literal (e.g. `"player/jump_small.ogg"`,
`level/level_player.cpp:1304`) or a per-enemy `m_kill_sound` string (`enemies/enemy.h:72`, 12 sites, e.g.
`enemies/spika.cpp:445`, `objects/box.cpp:458`); there are no sound-id constants. 26 of the 91 sites pass an
`RID_*` id (23 in `level_player.cpp`, `gui/hud.cpp:529`, `objects/box.cpp:757`); note
`objects/box.cpp:805` plays `"wall_hit.wav"` *without* the id its sibling at `:757` uses. One caller passes
a full path into `Play_Sound` (`level_player.cpp:251`, `DATA_DIR "/" GAME_MUSIC_DIR "/game/lost_1.ogg"` — a
music file played as a sound). `Play_Music` callers: `level/level.cpp:497`, `overworld/overworld.cpp:463`,
`gui/menu.cpp:657-669`, `level_player.cpp:3235-3236` (star: forced `game/star.ogg` then queued level music),
`turtle_boss.cpp:309`, `furball.cpp:326`, and the data-driven `core/game_core.cpp:250` (event attributes
`play_music`/`music_loops`/`music_force`/`music_fadein`). `Preload_Sounds` lists `"stomp_1.ogg"`
(`game_core.cpp:583`) which no code plays (`rg stomp_1 src` = that line only).

## 5. Old-pattern counts (all over `audio/*.{h,cpp}`)

| Pattern | Command | Count |
|---|---|---|
| raw `new` / `delete` | `rg -n '\bnew\s+[A-Za-z_]' ` / `rg -n '\bdelete\s'` (minus comment hits) | 5 / 4 (`audio.cpp:415,673,321,379,432`; `sound_manager.cpp:102`; `random_sound.cpp:89,420,426`) |
| `NULL` | `rg -o '\bNULL\b'` | 23 (audio.cpp 16, sound_manager.cpp 5, random_sound.* 2) |
| `typedef` | `rg -n typedef` | 2 (`AudioSoundList`, `SoundList`) |
| explicit-iterator loops | `rg -n '::(const_)?iterator'` | 9 (8 loops + `last_itr`) |
| `virtual` / `override` | `rg -o '\bvirtual\b'` / `'\boverride\b'` | 13 / 0 |
| unscoped enums | `rg -n '\benum\b'` | 1 (`AudioChannel`, name unused, values used as `int`) |
| `#define` | `rg -n '^\s*#define'` | 3 (include guards only) |
| `bool` as `0`/`1` | `return [01];` in bool fns 36; bool assignments 16; `bool x = 0/1` defaults 4; `Set_Pos(...,1)` 2 | 58 |
| `printf` / `sprintf` / `char buf[]` | `rg -o 'printf\('` etc. | 17 / 0 / 0 (one `const char *format_str`, `audio.cpp:222`) |
| C-style casts / `static_cast` | `rg -n '\((int\|float\|Uint8\|...)\)\s*[a-z(]'` / `rg -o static_cast<` | 0 / 38 |
| `( void )` parameter lists | `rg -o '\( void \)'` | 84 |
| `std::string` by value in API | `audio.h:103,106,108,113,145` | 5 (mutated in-body, so by-value is deliberate) |
| public data members | `audio.h:74-79,190-215`, `sound_manager.h:42-44` | all state public |

## 6. Dead or suspicious code

Dead (zero callers anywhere, `rg -n 'pAudio->X\|X('` per name): `cAudio::Resume_Sound` (`audio.cpp:720`),
`Fadeout_Sounds(ms, channel, overwrite)` overload (`audio.cpp:745`; only the filename overload is used,
`random_sound.cpp:477`), `Set_Music_Position` (`:841`), `Is_Music_Paused` (`:871`), `Halt_Sounds` (`:901`);
`cSound_Manager::Delete_Sounds` (`sound_manager.cpp:96` — and buggy: deletes each chunk but leaves the
dangling pointers in `objects`, the `obj = NULL` at `:103` only nulls a local), `m_load_count`
(incremented `:92`, never read), both `operator[]` (`sound_manager.h:71,76`). `cAudio::m_debug` is never set
true anywhere (`rg 'm_debug' src` → only the ctor and its `if( m_debug )` guards), so 9 of the 17 `printf`s
are unreachable. `RID_MARYO_AU` is referenced only from three commented-out `//pAudio->PlaySound(...)` lines
(old API name; `enemies/eato.cpp:323`, `flyon.cpp:661`, `gee.cpp:645`, each under `// todo : create again`),
matching the commented-out preload at `game_core.cpp:520`. `rg -n -i 'todo|fixme'` inside `audio/` = 0.
`cObject_Manager::Delete(size_t)` (`core/obj_manager.h:40-54`, inherited) erases then indexes the erased
slot — not called for sounds.

Suspicious, verified by reading:
1. **Callback from the mixer thread mutates game state.** SDL2_mixer's contract (`/usr/include/SDL2/SDL_mixer.h:1249`):
   "The callback may be called from the mixer's audio callback or it could be called as a result of
   `Mix_HaltChannel()`, etc." `Finished_Sound` (`audio.cpp:29-41`) iterates `pAudio->m_active_sounds`
   (a `std::vector`) and writes `obj->m_channel = -1` with no lock, while the main thread `push_back`s
   (`audio.cpp:674`), `erase`s (`:381`) and `delete`s (`:321`) the same vector. A natural channel end
   racing a `Create_Sound_Channel` reallocation is iterator invalidation on the audio thread. It also
   fires synchronously from `Mix_HaltChannel` inside `Stop()`/`Close()`, so the vector is walked from
   both threads. No crash is proven here — flagged as a data race by construction.
2. **`Fadeout_Sounds(ms, filename)` can fade every channel.** `audio.cpp:781-798` does not skip idle pool
   entries (`m_channel == -1`); a recycled `cAudio_Sound` whose last chunk had the requested filename
   reaches `Mix_FadeOutChannel( obj->m_channel /* -1 */, ms )`, and channel −1 means *all channels*.
   Reached from `cRandom_Sound::Event_Out_Of_Range` (`random_sound.cpp:477`) whenever a random-sound
   object scrolls out of range after its sound has ended.
3. **Toggling sound silences music.** `Toggle_Sounds` → `Init` → unconditional `Close()` (`audio.cpp:175`)
   frees `m_music` (`:337`); after re-init `m_music` is NULL and nothing replays `m_music_filename`
   (`Toggle_Music` does, `:688-691`; `Toggle_Sounds`, `:694-704`, does not). Callers: F10
   (`input/keyboard.cpp:265`) and the options menu (`gui/menu_data.cpp:2068,2688`). Music stays off until
   the next `Play_Music`.
4. Queued music drops `loops`/`fadein_ms` (`audio.cpp:990`, see section 2) — every `force=0` caller passes
   `-1` and a fade-in that never applies once queued.
5. `cAudio_Sound::Play`'s "skip self" test `obj->m_channel == m_channel` (`audio.cpp:97`) compares against
   −1 at that point, so it skips every idle entry rather than specifically self; harmless but not what the
   comment says.
6. Cost per `Play_Sound`: two `stat()`s (`File_Exists`, `audio.cpp:448,457`) + a string-compare scan of the
   ~60-entry cache + a scan of the 10-entry pool; `cRandom_Sound::Update` in continuous mode repeats
   `Get_Playing_Sound` (path prefix insert + scan) every frame per object (`random_sound.cpp:322`).
7. `cRandom_Sound::Draw` `new`s two `cCircle_Request`s (`random_sound.cpp:420,426`) handed to `pRenderer->Add`;
   the queue deletes them (`video/renderer.cpp:722,774`) — ownership transfer by convention, no type says so.

## 7. Modern-C++ hot spots (top five)

1. **Owning raw pointers → value/`unique_ptr`.** `vector<cAudio_Sound*> m_active_sounds` and
   `vector<cSound*> objects` with hand `delete` loops (`audio.cpp:319-324,374-382`, `obj_manager.h:84-92`)
   → `std::vector<cAudio_Sound>` (pool is ≤10 POD-ish objects) and `std::unordered_map<std::string,
   std::unique_ptr<cSound>>` (kills the linear scan and `Delete_Sounds`); `Mix_Chunk*`/`Mix_Music*` →
   `std::unique_ptr<Mix_Chunk, decltype(&Mix_FreeChunk)>`. Sites: 5 `new`, 4 `delete`, 2 typedefs, 9 loops.
2. **Thread-safe finished-callback.** Replace the vector walk in `Finished_Sound` with an atomic
   per-channel flag array (`std::array<std::atomic<bool>, N>`) or an SDL-locked queue drained in
   `Update()`; register `Mix_ChannelFinished` once in `Init`, not per play (`audio.cpp:114`). Fixes 6.1.
3. **`bool` literals and `NULL`.** 58 `0/1`-as-bool sites and 23 `NULL` → `true/false`, `nullptr`;
   `( void )` ×84 → `()`; `virtual` ×13 with no `override` → `override` on the 9 `cRandom_Sound`
   overrides and `final` where appropriate.
4. **Resource ids → `enum class` + typed API.** `enum AudioChannel` values are passed as `int res_id`
   through `Play_Sound`/`cAudio_Sound::Play` (`audio.h:69,106`); make it `enum class SoundGroup` with an
   explicit `None`, and take `std::string_view`/`std::filesystem::path` for filenames (the 5 by-value
   `std::string` params mutate only to prefix the data dir — do that once into a local).
5. **Diagnostics.** 17 `printf`s (9 unreachable behind the never-set `m_debug`) → one logging function or
   `std::print`/`fmt`; the `switch` on `dev_format` (`audio.cpp:224-247`) → a `constexpr` lookup.
Runners-up: replace the `File_Exists`-twice-then-prefix dance with one `std::filesystem::exists`; make
`cRandom_Sound`'s eight editor callbacks a table of `{label, getter, setter}`.

## 8. Cross-references

Depends on: SDL2_mixer (`Mix_*`, 30+ distinct calls; `sound_manager.h:23` is the only `#include "SDL_mixer.h"`),
`core/global_basic.h` (SDL, `using std::vector`), `core/obj_manager.h` (`cObject_Manager<T>`),
`core/global_game.h` (`DATA_DIR`, `GAME_SOUNDS_DIR`, `GAME_MUSIC_DIR`), `core/filesystem/filesystem.h`
(`File_Exists`), `user/preferences.h` (`pPreferences->m_audio_*`, volume defaults), `core/game_core.h`
(string helpers, `Preload_Sounds` lives there). `audio.cpp` also includes `level/level.h`,
`overworld/overworld.h`, `core/i18n.h` but uses nothing from them (`rg 'pActive_Level|pActive_Overworld|_\(' audio.cpp` = 0)
— removable includes. `random_sound.*` additionally depends on `objects/sprite.h`, `video/renderer.h`,
`input/mouse.h`, `core/framerate.h`, `core/math/utilities.h`, and CEGUI (XML + editor widgets).
Depended on by: 38 files via `pAudio` (section 4); `level/level.h:23` and `overworld/overworld.h:25`
include `random_sound.h` for object construction; `core/main.cpp` owns both singletons' lifetimes;
`user/preferences.cpp` reads/writes `pAudio->m_*_volume` and `m_*_enabled` directly.
