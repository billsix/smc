# Bug: oversized textures are distorted (aspect ratio not preserved) when clamped to GL_MAX_TEXTURE_SIZE

**Status:** proposed — needs go-ahead (found in the source analysis, 2026-09-23)
**Priority:** 6 · **Difficulty:** 2
**Do before:** `tasks/smc-modernize-cpp.md`.

## BLUF

`cVideo::Apply_Max_Texture_Size` computes an integer `scale_down = width / m_max_texture_size` and
divides height by it while clamping width to the max — so a 1500-wide image on a 1024 limit gets
`scale_down = 1`, width clamped to 1024, height unchanged: the texture is squashed. Compute the
scale as a float ratio and apply it to both dimensions.

## Context

- Verified in `tasks/reference/smc/video.md` (§9, "Bug smells").
- `smc/src/video/video.cpp`, `cVideo::Apply_Max_Texture_Size` (~:1651):
  ```cpp
  int scale_down = width / m_max_texture_size;   // integer: 1500/1024 == 1
  height = height / scale_down;                   // unchanged
  width  = m_max_texture_size;                    // clamped -> distorted
  ```
- Only bites images wider (or taller) than the GPU's max texture size, so it is rare on modern
  hardware — but wrong when it fires.

## Fix

`float scale = static_cast<float>(m_max_texture_size) / std::max(width, height);` (only when a
dimension exceeds the max), then scale both `width` and `height` by it and round. Reuse
`Downscale_Image` for the resample. Handle the height-exceeds case symmetrically.

## Verification

Container build green; feed an image larger than `GL_MAX_TEXTURE_SIZE` (or temporarily lower the cap
in a debug build) and confirm the downscaled texture keeps its aspect ratio.

## Open questions

None.
