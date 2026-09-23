# Fix the small latent correctness bugs (not user-visible, but wrong)

**Status:** proposed — needs go-ahead (found in the source analysis, 2026-09-23)
**Priority:** 6 · **Difficulty:** 3
**Do before:** `tasks/smc-modernize-cpp.md` (start the rewrite on correct code).

## BLUF

A cluster of small, individually one-line correctness bugs the analysis found — each real, none
user-visible today (dead paths, masked-by-luck, or edge cases). Grouped into one task because each
is trivial and they share the "fix before the rewrite so it starts clean" rationale. Fix them (or,
for the dead ones, delete per `smc-remove-dead-code.md`); each is one small commit.

## Context and items

All verified in `tasks/reference/smc/` (subsystem docs' "Dead or suspicious code" sections).

1. **`string_trim` trims the wrong end** — `core/property_helper`: `string_trim_from_begin` is
   implemented as an end-trim, and `string_trim_from_end` returns a copy that callers discard, so
   `string_trim` (level.cpp, world_manager.cpp) and `cFile_parser::Parse_Line` never trim leading
   characters. Fix the two helpers (or replace with `std::string_view` trimming). The five
   `// todo : use boost::algorithm` comments are moot — Boost.StringAlgo is unused.
2. **`cObject_Manager::Delete(size_t)` and `Switch_Array_Num`** — `core/obj_manager.h`: both are
   dead but wrong (Delete erases-then-deletes the shifted slot; Switch assigns a `T*` to an
   iterator). If not deleted by `smc-remove-dead-code.md`, fix to `std::iter_swap` / delete-before-erase.
3. **`cLevel_Manager::Load` adds a failed load** — `level/level_manager.cpp:~125`: `level->Load();
   Add(level); return level;` ignores the load result, leaving an unloaded `cLevel` in the list and
   returning non-NULL. Check the return; do not `Add` on failure.
4. **`cImage_Manager` delete loop off-by-one** — `video/img_manager.cpp:~225`: `for( GLuint i = 0;
   i < m_high_texture_id; i++ )` never visits the highest id (`<` vs `<=`), and passes `&i` to
   `glDeleteTextures`.
5. **`Downscale_Image` block-size typo** — `video/video.cpp:~1729`: `u_block = width - i *
   block_size_y;` should use `block_size_x` (wrong only for non-square blocks at the right edge).
6. **`Color::Get_SDL_Color` leaves alpha uninitialised** — `video/color.h:~73`: fills r/g/b only;
   under SDL 2 `SDL_Color` has an `a`. Set `a = 255` (or the colour's alpha). Feeds
   `TTF_RenderUTF8_Blended`.
7. **Menu Ctrl+X debug shortcut divides by zero** — `gui/menu.cpp:~389`: `rand() %
   listbox_levels->getItemCount()` with an empty level list. Guard the count.
8. **`cMenuCore` null-deref on ESC/joy-exit** — `gui/menu.cpp:~410,452`: `m_menu_data->Exit()`
   lacks the `if( m_menu_data )` guard its siblings have; and `Load(MENU_NOTHING)` leaves
   `m_menu_data == NULL` then dereferences it (~:591). Add the guards / handle MENU_NOTHING.
9. **`cDebugDisplay::Set_Text` no-op** — `gui/hud.cpp:~1012`: `m_text_old.empty();` is a query, not
   `clear()`. Likely meant `m_text_old.clear();`.
10. **`Add_Kill_Multiplier` float-modulo** — `level/level_player.cpp:~3866`: `static_cast<int>(
    m_kill_multiplier * 10 ) % 5 == 0` on a float accumulated by `+= 0.1f`; the "every fifth kill"
    test depends on float rounding, and the comment ("add 1 live") disagrees with the code
    (`Add_Gold(100)`). Decide intended behaviour, then make it exact (integer counter).

Not in this task (bigger than a one-liner, kept in the modernization or their own decision):
savegame object-identity-by-position (a design limitation), the `posx!=0 && posy!=0` active-level
sentinel, the synchronous modal loops.

## Plan

- [ ] One small commit per item, each naming the file:symbol and the wrong-vs-right behaviour.
- [ ] For 2, coordinate with `smc-remove-dead-code.md` (delete vs fix — don't do both).
- [ ] Re-verify each against current source before touching it.

## Verification

Container build green; headless smoke run passes; where a fix is observable (1 leading-space trim,
6 text alpha, 7/8 menu robustness) confirm the behaviour on the host.

## Open questions

1. Item 10: what was the intended reward rule — 1-up every 5th kill, or 100 gold? The code and
   comment disagree; your call before it is "fixed" in either direction.
