# `video/` — source analysis (SMC at `smc/src/video/`, HEAD after the SDL 2 port, 2026-09-23)

Scope: the 15 files under `smc/src/video/` (8,512 lines, 233 KB). Every claim below was checked
against the source with `rg`/`git`; "callers" means call sites outside `video/` unless stated.
Anchors are `file:Symbol` (line numbers where the symbol alone is ambiguous; they are the
current snapshot and will rot).

## 1. Files and roles

| File | Lines | Role | Status |
|---|---|---|---|
| `video.h/.cpp` | 302 / 2,470 | `cVideo`: SDL window + GL context, CEGUI 0.7 bring-up, texture creation, image cache, screenshots, the `Draw_*` request factories, the fade effects and the loading screen (free functions) | live; `Get_Pixel`, `Render_From_Thread`, `Make_GL_Context_*` dead (below) |
| `renderer.h/.cpp` | 283 / 791 | `cRenderQueue` + the request classes; the only place that emits geometry (106 of the 186 GL call sites) | live |
| `gl_surface.h/.cpp` | 116 / 335 | `cGL_Surface`: a GL texture id plus draw/collision metadata; `Blit` = "make a `cSurface_Request`" | live; `Save`, `Set_Ground_Type` have no callers |
| `img_manager.h/.cpp` | 128 / 250 | `cImage_Manager` (path-keyed cache of `cGL_Surface`), `cSaved_Texture` (CPU copy used across a GL-context loss) | live; `Copy(path)`, `operator[]` unused |
| `img_settings.h/.cpp` | 107 / 604 | `.settings` sidecar parser (`cImage_Settings_Parser : cFile_parser`) and the value object `cImage_Settings_Data` | live |
| `font.h/.cpp` | 80 / 186 | `cFont_Manager`: three `TTF_Font`s, text → `cGL_Surface`, grab/restore | live |
| `animation.h/.cpp` | 402 / 2,278 | `cAnimation` family, `cParticle`/`cParticle_Emitter` (particle system, its XML I/O and 37 editor handlers), `cAnimation_Manager` | live; 6 editor handlers never subscribed |
| `color.h` | 180 | header-only `Color` (4×`Uint8`) + 22 `static const` named colours | live |

Git: everything but `video.cpp/.h` last changed in upstream commits (2011–2014); `video.cpp/.h`
were changed by `d438165 update to SDL2` (+95/−128 lines, the only port hunks in this directory).

## 2. `cVideo` (video.cpp)

**Creation and init order** (`core/main.cpp:Init_Game`, lines 251–291): `new cVideo` →
`Init_CEGUI_Fake` → `pPreferences->Load()` → `Delete_CEGUI_Fake` → `Init_SDL` → `Init_Video` →
`Init_CEGUI` → `Init_CEGUI_Data` → `pFont->Init()`. Teardown in `Exit_Game` (main.cpp:440–505):
renderers, then `pGuiSystem->destroy()` + `delete rp` + `delete logger`, then
`OpenGLRenderer::destroy(*pGuiRenderer)`, then `delete pVideo` (its destructor is empty:
`video.cpp:cVideo::~cVideo` — no `SDL_GL_DeleteContext`/`SDL_DestroyWindow`; `atexit(SDL_Quit)` from
`Init_SDL` is what tears SDL down).

**The CEGUI "fake"** (`video.cpp:Init_CEGUI_Fake`): before any GL context exists, the game needs
CEGUI's XML parser for `preferences.xml`, so it builds a `CEGUI::System` on
`CEGUI::NullRenderer::create()` with a `DefaultResourceProvider`, log to `/dev/null` (`main.cpp:263–283`
explains why). `Delete_CEGUI_Fake` calls `pGuiSystem->destroy()` then `delete renderer` on the
`CEGUI::Renderer*` (works: `CEGUIRenderer.h:295` has a public virtual dtor; the idiomatic call would be
`NullRenderer::destroy`). This is why `configure.ac` needs `CEGUINullRenderer`.

**The real CEGUI bring-up** (`video.cpp:Init_CEGUI`, `Init_CEGUI_Data`), CEGUI 0.7 API used:
- `CEGUI::OpenGLRenderer::create( CEGUI::Size(w,h) )` (window size from `SDL_GetWindowSize`), then
  `enableExtraStateSettings(1)` (CEGUI sets/clears the extra GL state itself; that is what lets the
  game's fixed-function state survive `renderGUI()`).
- `DefaultResourceProvider` with resource groups `schemes`, `imagesets`, `fonts`, `looknfeels`,
  `layouts` (all under `DATA_DIR "/" GUI_*_DIR`), plus `schemas` only if
  `System::getDefaultXMLParserName() == "XercesParser"` (the vendored build uses expat, so that branch
  is inert: `entrypoint/build.sh:40`).
- `new CEGUI::DefaultLogger()` (level `Errors`, `Informative` under `_DEBUG`),
  `MouseCursor::setInitialMousePosition`, `WindowFactoryManager::addFactory<SMC_SpinnerFactory>()`
  (the game's own widget, `gui/spinner.h`).
- `CEGUI::System::create( *pGuiRenderer, rp, NULL /*xmlParser*/, NULL /*imageCodec*/, NULL /*scriptModule*/, "" /*configFile*/, logFile )` — XML parser and image codec are left to CEGUI's compiled-in defaults (expat + FreeImage in our build). Logfile `~/.smc/cegui.log`.
- `Init_CEGUI_Data`: `Scheme/Imageset/Font/WidgetLookManager/WindowManager::setDefaultResourceGroup`,
  `SchemeManager::getSingleton().create("TaharezLook.scheme")`, default cursor `TaharezLook/MouseArrow`,
  tooltip `TaharezLook/Tooltip`, root sheet `WindowManager::loadWindowLayout("default.layout")`.
- Every CEGUI failure is `printf` + `exit(EXIT_FAILURE)` (4 sites in this file; 8 `exit(` in video/).

**Window and GL context** (`video.cpp:Init_Video`): flags `SDL_WINDOW_OPENGL [| SDL_WINDOW_FULLSCREEN]`;
requested bpp → `SDL_GL_RED/GREEN/BLUE_SIZE` (8→3-3-2, 15→5-5-5, 24/32→8-8-8, else 5-6-5),
`SDL_GL_DOUBLEBUFFER 1`, no alpha/depth attribute (both commented out at lines 362/364, "hangs on
16 bit"). First call: `SDL_CreateWindow(CAPTION, centered, w, h, flags)` + `SDL_GL_CreateContext` +
`SDL_SetWindowIcon` from `window_32.png`. Later calls (`preferences.cpp:403` after an options-menu
change; `use_preferences` is never 0 from any caller): `SDL_SetWindowFullscreen` + `SDL_SetWindowSize`
+ `SDL_GL_MakeCurrent` — the context is kept, but the code still does the full SDL-1.2 texture
round-trip around it (`Grab_Textures`/`grabTextures`/`Delete_Hardware_Textures` before,
`restoreTextures`/`Restore_Textures`/`notifyDisplaySizeChanged` after, lines 368–390 and 510–547).
Then `glGetIntegerv(GL_DRAW_BUFFER)`, `glGetIntegerv(GL_MAX_TEXTURE_SIZE)` → `m_default_buffer`
(never read anywhere), `m_max_texture_size`; `Init_OpenGL`; on first init parse `glGetString(GL_VERSION)`
into `m_opengl_version` (float, first 3 chars, used only to pick `GL_GENERATE_MIPMAP` vs `gluBuild2DMipmaps`).

**Fixed-function state** (`video.cpp:Init_OpenGL`, `Init_Geometry`, `Init_Texture_Detail`):
`glViewport(0,0,pref_w,pref_h)`; `GL_PROJECTION` ← `glOrtho(0, w, h, 0, -1, 1)` (y down, z in
[-1,1]); `GL_MODELVIEW` identity; `glShadeModel(GL_SMOOTH)`; clear colour black;
`GL_DEPTH_TEST` on with `GL_LEQUAL`, `glClearDepth(1)`; `GL_BLEND` with
`GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`; `GL_ALPHA_TEST` with `GL_GREATER 0.01`;
`GL_POINT_SMOOTH`/`GL_LINE_SMOOTH` + hints by `m_geometry_quality` (`GL_POLYGON_SMOOTH` commented out,
"open source ATi drivers 2008"), `GL_PERSPECTIVE_CORRECTION_HINT`, `GL_GENERATE_MIPMAP_HINT` (GL ≥ 1.4).
Note the viewport/ortho always use `pPreferences->m_video_screen_w/h`, so a window created at a
different size (the `use_preferences=0` fallback 800×600×16, or the 640×480 fallback) would render
with the wrong projection — dead today because both paths are unreachable (below).

**Resolution scale** (`video.cpp:Init_Resolution_Scale`): the four globals
`global_upscalex/y = pref_w / game_res_w` and `global_downscalex/y` (defined in `core/game_core.cpp:60–63`,
`game_res_w/h = 800×600`). The game draws in an 800×600 logical space; `cRender_Request_Advanced::Render_Basic`
applies `glScalef(global_upscalex, global_upscaley, 1)` per request when either ≠ 1, and
`cImage_Settings_Data::Get_Surface_Size` picks the mip level by `m_width * global_upscalex`. So
"software scaling" is really a per-request modelview scale plus a load-time texture-size choice, not a
CPU resample of the framebuffer.

**vsync / fullscreen / double-buffer**: `SDL_GL_SetSwapInterval(pref_vsync)` + `SDL_GL_GetSwapInterval`
warning; `SDL_GetWindowFlags & SDL_WINDOW_FULLSCREEN` warning; `SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER)`
→ `m_double_buffer` (never read outside this function). `Toggle_Fullscreen` (F-key `input/keyboard.cpp:157`,
`preferences.cpp:419`): flips the preference and calls `SDL_SetWindowFullscreen`; it still saves and
restores `GL_COLOR_CLEAR_VALUE` around the call — a leftover from SDL 1.2 where Windows re-ran
`Init_Video()` (see the removed `#ifdef _WIN32` in `git show d438165`); now a no-op pair.

**`Test_Video`** (`video.cpp:909`): `return bpp;` — the port replaced `SDL_VideoModeOK`. Its three
callers still branch on the result: `Init_Video:289–320` (the `== 0` fallback to 640×480 and the
"closest bpp" branch can only fire for bpp 0, which no caller passes: prefs default 32, menu offers
"16"/"32" — `gui/menu_data.cpp:1432–1435`) and `gui/menu_data.cpp:2435,2454` (`if(!Test_Video(...)) return 0;`,
never true now). **`Get_Supported_Resolutions`**: `SDL_GetNumDisplayModes(0)`/`SDL_GetDisplayMode`,
deduplicated by `(w,h)` with an O(n²) inner scan, fallback list 2048×1536…640×480 when SDL reports none;
one caller (`menu_data.cpp:1374`, the options combobox). The `flags` parameter of both is unused.

**Screenshots** (`video.cpp:Save_Screenshot`, F-key via `input/keyboard.cpp` + 3 more callers):
`glReadPixels(0,0,w,h,GL_RGB)` into `new GLubyte[]`, `Save_Surface` writes PNG with libpng directly
(`png_create_write_struct` … `png_write_image`), rows reversed for GL's bottom-up order; filenames
`~/.smc/screenshots/<1..999>.png`, first free wins. `Save_Surface` copies the buffer once more byte-by-byte
(`for( int i = 0; i < img_size; ++i ) image[i] = data[i];`) before building row pointers.

**Image cache** (`video.cpp:Init_Image_Cache`): `~/.smc/cache/<w>x<h>/pixmaps/...png` — every `.settings`
image with `width`/`height` is loaded, `Convert_To_Final_Software_Image`d, box-downscaled
(`Downscale_Image`, Jonathan Dummer's MIT code) to the size `Get_Surface_Size` picks for this resolution,
and written back as PNG; `Load_Image` prefers the cached file when present. Cache is wiped when
`pPreferences->m_game_version != smc_version` or `recreate` (`menu_data.cpp:2607`). Runs under the
CEGUI loading screen with a progress bar; `_DEBUG` builds also blit the current filename with `pFont`.

**Threaded render experiment** — dead: `Render(bool threaded)`, `Render_From_Thread`,
`Make_GL_Context_Current/Inactive`, `boost::thread m_render_thread`, `pRenderer_current` swapping
are all behind `SMC_RENDER_THREAD_TEST`, which is commented out in `core/global_basic.h:22`
(upstream commit `2a1aa52`: "draws correctly only on windows"). Consequences in the default build:
`Render_Finish` returns at its first line (`#ifndef SMC_RENDER_THREAD_TEST return;`), so its 10 call
sites are no-ops; `Render()` takes the single-thread branch; `pRenderer_current` is allocated
(`main.cpp:256`) but never rendered; `boost/thread` is the only Boost use in `video/`.

**Still SDL-1.2-shaped after the port (candidates to simplify now that SDL 2 is the target):**
1. `video.h:24,36` still include `<GL/glx.h>` and `SDL_syswm.h`; no `glX*`/`SDL_SysWMinfo` use remains
   in `src/` (checked; `gui/generic.cpp:27` has its own `SDL_syswm.h` include).
2. `Test_Video` and its three caller branches (above); `Get_Supported_Resolutions( flags )`'s unused flag.
3. The bpp → `SDL_GL_*_SIZE` table and the "smaller red/green/blue bit size" warnings, `m_rgb_size`,
   `m_double_buffer`, `m_default_buffer` — computed, never consumed outside `Init_Video`.
4. The grab/delete/restore texture round-trip on re-init (`Init_Video:368–390, 510–547`,
   `img_manager.cpp:Grab_Textures/Restore_Textures`, `font.cpp:Grab_Textures/Restore_Textures`,
   `cSaved_Texture`, `pGuiRenderer->grabTextures/restoreTextures`) exists only because
   `SDL_SetVideoMode` destroyed the context; with a persistent `SDL_GLContext` only the projection
   (`Init_OpenGL`) and `notifyDisplaySizeChanged` are needed. `Delete_Hardware_Textures` (the
   `glIsTexture` sweep over ids `0..m_high_texture_id`) goes with it.
5. `Toggle_Fullscreen`'s clear-colour save/restore.
6. `Init_Video(reload_textures_from_file, use_preferences)`: the `use_preferences=0` path is unreachable.
7. `Init_SDL` still does `SDL_Init(SDL_INIT_VIDEO)` then two `SDL_InitSubSystem` calls with `== -1`
   checks; `IMG_Init(IMG_INIT_PNG)` result is ignored.

## 3. Fixed-function OpenGL inventory

186 GL/GLU call sites in `video/` (renderer.cpp 106, video.cpp 52, gl_surface.cpp 19, img_manager.cpp 7,
font.cpp 2), 44 distinct functions; outside `video/` only 3 `glClearColor` (`gui/menu.cpp:554`,
`gui/menu_data.cpp:3418,3435`). Command: `rg -o -N '\bglu?[A-Z]\w*\s*\(' video/ | sort | uniq -c`.

- **Immediate mode (all geometry):** 8 `glBegin`/6 `glEnd`, 21 `glVertex2f`, 4 `glTexCoord2f`,
  13 `glColor4ub/4f`. Modes: `GL_QUADS` (`renderer.cpp:cSurface_Request::Draw`, the sprite quad),
  `GL_POLYGON` ×3 (filled rect, both gradients), `GL_LINE_LOOP` (outline rect), `GL_LINES` (line),
  `GL_TRIANGLE_FAN`/`GL_LINE_STRIP` (circle, `sin/cos` per vertex, step `1/(r*0.05)` capped at 0.2 rad).
  No display lists, no vertex arrays/VBOs (`renderer.cpp:668` comments them out of consideration).
- **Matrix stack:** `glMatrixMode`/`glOrtho` (init), `glLoadIdentity` ×4 (once per request in
  `Render_Basic` — "gl push and pop … was a lot slower on a Radeon X850 Pro"), `glTranslatef` ×8,
  `glScalef` ×3, `glRotatef` ×3. No `glPushMatrix`/`glPopMatrix`.
- **Texturing:** `glGenTextures`/`glBindTexture`/`glDeleteTextures`/`glIsTexture` (7 — used as a liveness
  query in destructors and sweeps), `glTexParameteri` ×10 (`CLAMP_TO_EDGE`, `LINEAR`,
  `LINEAR_MIPMAP_LINEAR`, `GL_GENERATE_MIPMAP`), `glTexImage2D` ×2 (always `GL_RGBA`/`GL_UNSIGNED_BYTE`),
  `gluBuild2DMipmaps` (GL < 1.4 fallback), `glPixelStorei(GL_UNPACK_ROW_LENGTH)`, `glGetTexImage` ×2
  and `glGetTexLevelParameteriv`/`glGetTexParameteriv` (the grab-to-CPU path), `glTexEnvi/fv` ×8
  (`GL_COMBINE` with `GL_CONSTANT`×`GL_TEXTURE` for `m_combine_type`, reset to `GL_MODULATE`).
- **State:** `glEnable`/`glDisable` ×17 (`GL_TEXTURE_2D` toggled per request after a `glIsEnabled`
  query — 5 `glIsEnabled` calls, i.e. a GL round-trip per primitive), `glBlendFunc` ×3,
  `glLineWidth` ×6, `glLineStipple`/`GL_LINE_STIPPLE`, `glAlphaFunc`, `glDepthFunc`, `glShadeModel`,
  9 `glHint`.
- **Readback:** `glReadPixels` ×2 (`Save_Screenshot`; `Get_Pixel` is dead), `glGetString`,
  `glGetIntegerv` ×2, `glGetFloatv`, `glGetError` ×2 (`_DEBUG` only) + `gluErrorString`.
- **Core-profile blockers, as facts:** immediate mode, the matrix stack, `GL_ALPHA_TEST`,
  `GL_LINE_STIPPLE`, `GL_POINT/LINE_SMOOTH`, `GL_TEXTURE_ENV`/`GL_COMBINE`, `GL_POLYGON`/`GL_QUADS`,
  `glShadeModel`, `GL_PERSPECTIVE_CORRECTION_HINT`, `GL_GENERATE_MIPMAP`, GLU. Nothing in `video/`
  is core-profile-clean except texture upload/readback and `glClear`/`glViewport`.
- **Draw calls per frame implied by the design:** one `glBegin/glEnd` per queued request; each
  request also issues `glLoadIdentity`, `glTranslatef`, 0–2 `glScalef`, 0–3 `glRotatef`, `glIsEnabled`,
  and (`cSurface_Request`) `glColor4ub` + restore `glColor4f` when the tint ≠ white. The only state
  cache is `renderer.cpp:last_bind_texture` (skip a redundant `glBindTexture`); requests are sorted by z,
  not by texture, so sprites alternate textures freely. Queues are reserved at 200 requests
  (`main.cpp:255`); every visible sprite (`objects/sprite.cpp:Draw_Image_Normal`, via
  `cGL_Surface::Blit_Data`) is one request, plus HUD/text/editor overlays. Call-site counts outside
  `video/`: `new cSurface_Request` 19, `new cRect_Request` 15, `new cLine_Request` 4,
  `new cCircle_Request` 3, `pRenderer->Add(` 41, `->Blit(` 13.

## 4. `cGL_Surface` and `cImage_Manager`

**What a surface is** (`gl_surface.h:cGL_Surface`): a `GLuint m_image` plus draw metadata (`m_int_x/y`
drawing offset, `m_start_w/h` logical size, `m_w/h` current size, `m_tex_w/h` texture size, base
rotations), collision box (`m_col_pos`, `m_col_w/h`), and editor metadata from `.settings`
(`m_filename`, `m_editor_tags`, `m_name`, `m_type` = `SpriteType`, `m_ground_type`, `m_obsolete`).
Flags: `m_auto_del_img` (default 1: the destructor `glDeleteTextures` the id), `m_managed` (set by
`cImage_Manager::Add`), `m_obsolete`. Optional `destruction_function` (a plain function pointer; the only
user is `font.cpp:Font_Delete_Ref`).

**Lifetime rules, verified:**
- `cVideo::Create_Texture(SDL_Surface*)` (`video.cpp:1280`) is the sole `new cGL_Surface()` for
  textures: converts the surface to power-of-two 32-bit RGBA (`Convert_To_Final_Software_Image`,
  `SDL_CreateRGSurface` + `SDL_BLENDMODE_NONE` blit), applies `Apply_Max_Texture_Size` (box downscale in
  place via `Downscale_Image` when over `GL_MAX_TEXTURE_SIZE`), `glGenTextures` + parameters +
  `Create_GL_Texture` (mipmapped or not), frees the `SDL_Surface`, and records
  `pImage_Manager->m_high_texture_id`. The caller owns the result.
- `cVideo::Get_Surface(filename)` (`video.cpp:1080`) is the cache front door: normalises
  `.settings`→`.png` and prefixes `DATA_DIR/pixmaps/`, `pImage_Manager->Get_Pointer(path)` (linear scan
  over `objects` comparing `m_filename`), else `Load_GL_Surface` + `pImage_Manager->Add`. The header says
  "should not be deleted or modified"; managed surfaces die in `cImage_Manager::Delete_All` → 
  `Delete_Image_Textures` (deletes GL ids first "so the destructor doesn't check `Is_Texture_Use_Multiple`")
  → `cObject_Manager::Delete_All` (`delete` each). `pImage_Manager` itself is created/deleted in `main.cpp:258/488`
  and is touched from outside `video/` only for `Grab_Textures`/`Restore_Textures` (`menu_data.cpp:2604,2610`).
- `cGL_Surface::Copy()` returns a `new` surface **sharing the same `m_image`** (no refcount); the copy
  is not managed, so its destructor would `glDeleteTextures` the shared id unless the caller sets
  `m_auto_del_img = 0` — the destructor guard `( !m_managed || !Is_Texture_Use_Multiple() )` protects only
  managed surfaces. Callers: `level/level.cpp:1410,1417`, `overworld/overworld.cpp:1200`,
  `core/editor.cpp:1192` (all editor/level-loading paths). `cImage_Manager::Copy(path)` and both
  `operator[]` have no callers.
- `Load_Software_Texture` (restore path) with `m_pixels == NULL` reloads from file via `Load_GL_Surface`,
  steals `m_image`, sets `surface_copy->m_auto_del_img = 0`, deletes the temp — the same
  steal-the-id idiom.
- Software surfaces (`SDL_Surface`) never outlive loading: `Load_Image` returns a `cSoftware_Image`
  {`SDL_Surface*`, `cImage_Settings_Data*`} whose surface is consumed by `Create_Texture` (freed there)
  and whose settings the caller `delete`s (`Load_GL_Surface`, `Init_Image_Cache`). `IMG_Load` is used for
  both the PNG and the cached PNG.

**`.settings` files** (`img_settings.cpp:cImage_Settings_Parser::HandleMessage`): a whitespace-tokenised
line format from `core/file_parser.h`, one command per line, all integers except strings:
`base <file> [use_settings 0/1]` (image file to load instead, with optional settings inheritance —
recursive through a temporary parser, `Apply_Base` copies every field; the `todo` at line 336 notes the
override order is wrong: base overwrites derived), `int_x`, `int_y`, `col_rect x y w h`, `width`, `height`
(logical draw size — also gates caching), `rotation x [y [z]]` (0/90/180/270 handled in `Apply`, swapping
w/h and col rect for 90/270), `mipmap 0/1`, `editor_tags`, `name` (`_`→space), `type` (via
`Get_Sprite_Type_Id`), `ground_type`, `author`, `obsolete`. Unknown commands print and return 0. Each
`Get()` allocates a fresh `cImage_Settings_Data` the caller must `delete`; `pSettingsParser` is a global
(`main.cpp:260`). The `col_rect` error message prints `parts[1]` for every failing index (copy-paste).

## 5. The renderer queue (renderer.cpp/h)

**Request types** (`renderer.h`): `cRender_Request` {`m_type`: `RenderType`, `m_pos_z`, `m_render_count`}
→ `cClear_Request` (`glClear` + `glLoadIdentity`) and `cRender_Request_Advanced` {`m_global_scale`,
`m_no_camera` (default 1; when 0, `pActive_Camera->m_x/y` is subtracted), rotations, blend factors,
`m_shadow_pos/color`, `m_combine_type/color`} → `cLine_Request` (width, stipple), `cRect_Request`
(filled or outline, scale, width, stipple), `cGradient_Request` (two colours, `DIR_VERTICAL/HORIZONTAL`
only; any other `m_dir` draws nothing), `cCircle_Request` (radius, `m_line_width` 0 = filled),
`cSurface_Request` (`m_texture_id`, pos, scale, `m_w/h`, `m_color`, `m_delete_texture`). `REND_TEXT = 5`
is declared "todo" and has no class. `cSurface_Request::Draw` implements the drop shadow by recursively
calling itself once with `m_shadow_pos` temporarily 0, colour black×`m_shadow_color.alpha`, and
`GL_REPLACE` combine with `m_shadow_color/260` — two quads per shadowed sprite.

**Ownership:** requests are heap objects created by the caller (`new cXxx_Request()`, or by the
`cVideo::Draw_*`/`cGL_Surface::Blit` helpers when passed `NULL`) and handed to `cRenderQueue::Add`,
which takes ownership (it `delete`s a `REND_NOTHING` request immediately). There is no pool: each frame
`Render()` z-sorts (`std::sort` with `zpos_sort`, ascending `m_pos_z`, unstable), calls `Draw()`,
decrements `m_render_count`, then `Clear(0)` deletes every request with `m_render_count <= 0` using
`vector::erase` inside the loop (O(n²) on a 200+ element vector every frame). `m_render_count > 1` keeps a
request alive across frames (fade effects use 2, `world_layer.cpp` 50, `generic.cpp` 4);
`Fake_Render` decrements without drawing (loading screen at < 60 fps). `Clear(1)` (forced) is called by
`Draw_Effect_In` and the destructor. `cSurface_Request::~cSurface_Request` deletes the texture when
`m_delete_texture` (3 external sites: `input/mouse.cpp:1793,1823`, `gui/hud.cpp:1380` — one-shot text).

**Globals:** `pRenderer` (the queue the game fills), `pRenderer_current` (only meaningful in the dead
threaded mode, where `cVideo::Render(1)` swaps the two and re-queues leftovers; in the default build it is
an empty 200-slot vector). There is **no `pRenderer_GUI`** anywhere in `src/` (`rg pRenderer_GUI` → 0).

**Per-frame order** (`core/main.cpp:main` loop → `Draw_Game()` → `pVideo->Render()`,
`video.cpp:995`): `pRenderer->Render()` (game, z-sorted, `cClear_Request` sorts to z 0 — added by
`cVideo::Clear_Screen` and drawn first only because everything else has z > 0) → `pGuiSystem->renderGUI()`
(CEGUI draws over it with its own state) → `SDL_GL_SwapWindow(screen)`, with `pFramerate` perf timers
between. The loading screen has its own copy of this sequence (`video.cpp:Loading_Screen_Draw`).

## 6. `cFont_Manager` (font.cpp)

`Init` (`main.cpp:292`): `TTF_Init`, opens `default_bold.ttf` at 18/11/9 pt (`m_font_normal/small/very_small`;
all three used, 21 external references to the smallest). `Render_Text(font, text, color)`:
`TTF_RenderUTF8_Blended` → `pVideo->Create_Texture` (so every string becomes its own power-of-two RGBA GL
texture), `m_filename = text`, `Set_Destruction_Function(&Font_Delete_Ref)`, `Add_Ref` into
`m_active_fonts`. **Ownership of the result is the caller's**: 44 `pFont->Render_Text` call sites outside
`video/`, typically `Set_Image( pFont->Render_Text(...), 0, 1 )` (`gui/hud.cpp:443`, the sprite takes
`delete`-responsibility via its own flag) or a `cSurface_Request` with `m_delete_texture = 1`; the manager
only tracks live ones so `Grab_Textures`/`Restore_Textures` can survive a context loss, and
`Delete_Ref` (linear erase) runs from the surface's destructor. The destructor closes fonts and
`TTF_Quit()`s if `TTF_WasInit()`. `Color::Get_SDL_Color` (`color.h:73`) fills `r,g,b` only; under SDL 2
`SDL_Color` has an `a` member (`/usr/include/SDL2/SDL_pixels.h:328`), left uninitialised — see §9.

## 7. animation.cpp/h — the particle system

**Classes:** `cAnimation : cAnimated_Sprite` (base: `m_pos_z_rand`, `m_fading_speed`, `m_time_to_live[_rand]`;
`Init_Anim`/`Update`/`Draw` are empty virtuals) → `cAnimation_Goldpiece` (4 `cSprite` blink points, 3
images, 12-tick lifetime), `cAnimation_Fireball` (N `cAnimation_Fireball_Item : cAnimated_Sprite`, 4 fire
frames, hand-rolled `cSurface_Request` per item), `cParticle_Emitter` (the real one; `m_type_name
"particle_emitter"`, `TYPE_PARTICLE_EMITTER`, `ARRAY_ACTIVE`, XML `Load_From_XML`/`Save_To_XML` with 36
attributes, `Copy()`, `Pre_Update` fast-forwards `ttl` frames with `m_speed_factor` forced to 1 so a
freshly placed emitter looks steady-state). `cParticle : cMovingSprite` (owned by its emitter's
`m_objects`, `m_parent` back-pointer; `Update` fades `m_fade_pos` by `speedfactor_fps*0.001*speed/ttl`,
moves, adds gravity, rotates; `Draw` builds its request via `Draw_Image_Normal` then applies
`BLEND_ADD` (`GL_SRC_ALPHA, GL_ONE`) / `BLEND_DRIVE` (`GL_SRC_COLOR, GL_DST_ALPHA`), colour and alpha fading).
Emission (`Emit`): `Set_Quota` clamps 1..1000 particles per interval; `Update_Particles` erases
finished particles in-loop, emits while `m_emit_counter > interval`, deactivates the emitter once its
TTL is over and no particles remain. Clipping: `Keep_Particles_In_Rect` with `PCM_MOVE` (wrap),
`PCM_REVERSE`, `PCM_DELETE`.

**`cAnimation_Manager : cObject_Manager<cAnimation>`**: `Update` deletes inactive animations in-loop,
`Draw` iterates, `Add` forwards (with a `todo` about zero-TTL spawned emitters). **`pActive_Animation_Manager`
is a borrowed pointer, verified:** defined NULL at `animation.cpp:2274`; assigned to
`pActive_Level->m_animation_manager` (`core/main.cpp:305`), `cLevel::m_animation_manager`
(`level/level.cpp:465`, owned: `new` at `:76`, `delete` at `:91`), `cOverworld::m_animation_manager`
(`overworld/overworld.cpp:425`; `new :177`, `delete :203`), `cMenuCore::m_animation_manager`
(`gui/menu.cpp:602`; `new :278`, `delete :318`). Nothing in `video/` ever creates, deletes or even
dereferences it; `animation.cpp` only defines it.

**Editor handlers:** `animation.h:274–310` declares **37** `Editor_*_Changed/Select` members;
`animation.cpp:Editor_Activate` has **37 textual `subscribeEvent`** but only **31 live** — 6 are inside
`/* … */` blocks (`animation.cpp:1592–1654`: start rotation x/y, constant rotation x/y base+rand), so
`Editor_Rotation_X_Base_Text_Changed`, `Editor_Rotation_Y_Base_Text_Changed`,
`Editor_Const_Rotation_X_{Base,Rand}_Text_Changed`, `Editor_Const_Rotation_Y_{Base,Rand}_Text_Changed`
are defined but never wired (0 callers). Each live handler: `static_cast` the `WindowEventArgs`, read the
editbox text, call the setter, `Pre_Update()`, `return 1` — 31 near-identical 10-line functions.

## 8. Old-pattern counts (`cd smc/src/video`)

| Pattern | Command | Count |
|---|---|---|
| raw `new` / `delete` | `rg -c '\bnew\b'` / `'\bdelete\b'` | 53 / 47 (no smart pointers anywhere) |
| `NULL` | `rg -c '\bNULL\b'` | 86 (`nullptr`: 0) |
| `typedef` | `rg -n '\btypedef\b'` | 8, all `typedef vector<T*> Name;` |
| explicit iterator loops | `rg -c '::(const_)?iterator\b'` | 27 (range-for: 0) |
| `virtual` / `override` | `rg -c '\bvirtual\b'` / `'\boverride\b'` | 58 / 0 (the one textual hit is a comment) |
| unscoped `enum` | `rg -n '^\s*enum\b'` | 5 (`RenderType`, `BlendingMode`, `ParticleClipMode`, `Effect_Fadeout`, `Effect_Fadein`); `enum class`: 0 |
| `#define` | `rg -n '^\s*#\s*define'` | 11 = 8 include guards + `M_PI` fallback + `PNG_COLOR_TYPE_RGBA` + `NO_SDL_GLEXT` |
| `bool = 0/1` style | `rg -c '\b(bool\s+\w+\s*=\s*[01]\b|=\s*[01];)'` | 30 lines; plus 78 `return 0;`/`return 1;` (most from `bool` functions) |
| `sprintf` / `char buf[]` | `rg -c 'sprintf\|char\s+\w+\['` | 0 (strings are `std::string`; output is 93 `printf`) |
| casts | `rg -c 'static_cast<'` / `'reinterpret_cast<'` / C-style `(T)x` | 241 / 2 / 0 (the port removed the last one, `(SDL_Rect**)-1`) |
| `(void)` parameter lists | — | every zero-arg declaration (`cVideo( void )`, …) |
| `exit(EXIT_FAILURE)` | `rg -c '\bexit\('` | 8 |
| Boost | `rg -n 'boost::'` | 3, all the dead `m_render_thread` |

## 9. Dead or suspicious code

**Dead (0 callers, verified with `rg` over `src/`):** `cVideo::Get_Pixel` (allocates `new GLubyte[3]` for
one pixel), `cVideo::Render_From_Thread`, `Make_GL_Context_Current/Inactive`, `Render_Finish`'s body and
`m_render_thread` (all under the disabled `SMC_RENDER_THREAD_TEST`), `cVideo::m_default_buffer`,
`m_rgb_size`, `m_double_buffer` (written, never read), `cGL_Surface::Save`, `cGL_Surface::Set_Ground_Type`
(only via `Copy`), `cImage_Manager::Copy(path)` and both `operator[]`, `REND_TEXT`, the six unsubscribed
editor handlers (§7), `video.h:206` commented `Draw_Circle(GL_circle*)` overload, `Init_Video`'s
`use_preferences=0` branch, the `flags` parameters of `Test_Video`/`Get_Supported_Resolutions`, the
`"XercesParser"` schema branches (×2). Commented-out code: `img_settings.cpp:129,141,153` (`rotate3d`),
`video.cpp:362,364,501–504,644,646,990`, `animation.cpp:1592–1654` (6 editor blocks).

**`todo`/`fixme`/`hack`** (`rg -n -i 'todo|fixme|hack' video/` → 15): CEGUI utf8 string workaround ×2
(`video.cpp:98,170`, Windows-only), "use opengl in only one thread" (`:1058`), "make Create_Texture a
render request" (`:1290`), `_wfopen` portability (`:1792`), loading-screen re-show (`:2451`), `REND_TEXT`,
settings override order (`img_settings.cpp:336`), gravity max / TTL estimate / zero-TTL emitters /
`Set_Emitter_Rect` position hack (`animation.cpp:434,759,2266,1290`), `animation.h:32,270,355`.

**Bug smells (quoted):**
- `video.cpp:1729` in `Downscale_Image`: `u_block = width - i * block_size_y;` — should be
  `block_size_x` (the y line below is correct); wrong only for non-square block sizes at the right edge.
- `img_manager.cpp:225` `for( GLuint i = 0; i < m_high_texture_id; i++ )` — the highest id itself,
  which `Create_Texture` recorded, is never visited (`<` vs `<=`). Also passes `&i` to `glDeleteTextures`.
- `video.cpp:1651` `Apply_Max_Texture_Size`: `int scale_down = width / m_max_texture_size;` then
  `height = height / scale_down; width = m_max_texture_size;` — integer division, so a 1500-wide image
  on a 1024 limit gets `scale_down = 1`, width clamped, height unchanged: aspect not preserved.
- `color.h:73` `Get_SDL_Color` returns an `SDL_Color` with `a` uninitialised; it feeds
  `TTF_RenderUTF8_Blended` (`font.cpp:125`). SDL2_ttf (header here is 2.24) documents Blended output as
  ARGB "using alpha blending … with the given color"; whether `fg.a` is honoured was not verified from
  the library source in this pass — if it is, text alpha is garbage-dependent. Cheap fix either way.
- `color.h:88–115` `operator+`/`+=`/`-` on `Uint8` wrap silently (e.g. `Color + Color` used by
  `cParticle_Emitter::Emit` via `rand() % m_color_rand.red` adds, guarded only in `_DEBUG` by a printf).
- `renderer.cpp:764` `Clear`: `vector::erase` in a loop (quadratic); `img_manager.cpp:Get_Pointer`
  is a linear `std::string::compare` scan per `Get_Surface` call (called per sprite load).
- `animation.cpp:1997,2008,2019,2030` — the four dead const-rotation X/Y handlers pass
  `m_const_rot_z_rand` / `m_const_rot_z` (copy-paste from Z); harmless because unsubscribed.
- `video.cpp:2383,2410` `printf( "Warning: Loading Screen already initialized." )` — no `\n`.
- `video.cpp:Init_CEGUI` leaks the `DefaultLogger` on the `exit()` paths and `Exit_Game` `delete`s it by
  `Logger::getSingletonPtr()` — fine, but the `Init_CEGUI_Fake` path creates no logger, so CEGUI's own
  default logger is used there (writes `CEGUI.log` in the cwd unless the fake's `/dev/null` path applies;
  it does: the `logFile` argument to `System::create`).
- `video.cpp:Init_OpenGL` runs `glClear` + `SDL_GL_SwapWindow` as part of initialisation (a visible black
  frame), and every `Init_Geometry`/`Init_Texture_Detail` call (from the options menu) re-swaps nothing but
  calls the no-op `Render_Finish`.

## 10. Modern-C++ hot spots (top five)

1. **Owning raw pointers → `std::unique_ptr`/values.** `cRenderQueue::m_render_data` (`vector<cRender_Request*>`,
   `delete` in `Clear`), `cImage_Manager::objects`/`m_saved_textures`, `cFont_Manager::m_active_fonts`,
   `cParticle_Emitter::m_objects`, `cAnimation_Goldpiece/Fireball::m_objects`, `cImage_Settings_Parser::Get`'s
   returned `cImage_Settings_Data*`, `cSoftware_Image`. 53 `new`/47 `delete` in `video/`; the request
   queue alone is the hottest allocation site in the game (one `new` per drawn sprite per frame) — a
   `std::vector<std::variant<…>>` or an arena would remove both the allocation and the virtual `Draw`.
2. **`cGL_Surface::Copy` id-sharing + `m_auto_del_img`/`m_managed`/`Is_Texture_Use_Multiple` (a linear scan
   in a destructor) → a `shared_ptr<GLTexture>` (RAII `glDeleteTextures`) held by value in `cGL_Surface`.**
   Also retires `Delete_Image_Textures`/`Delete_Hardware_Textures` and the destruction-function hook
   (`font.cpp:Font_Delete_Ref`).
3. **The 31 live editor handlers in `animation.cpp:1794–2212`** (and their 37 declarations) → one
   generic `subscribeEvent(..., [this](const EventArgs& e){ Set_X(parse(e)); Pre_Update(); return true; })`
   per widget, or a table of {name, getter, setter}; ~420 lines → ~60. Same shape recurs in every
   `Editor_Activate` across `objects/` (out of scope here).
4. **`typedef vector<T*>` + 27 explicit-iterator loops → `using` + range-for / `std::erase_if`**
   (`cRenderQueue::Clear`, `cParticle_Emitter::Update_Particles`, `cAnimation_Manager::Update`,
   `Delete_Ref`, `Get_Pointer`); `cImage_Manager::Get_Pointer` → `std::unordered_map<std::string, …>`.
5. **`bool = 0/1`, `NULL`, `(void)`, `virtual` without `override`, unscoped enums** — 86 `NULL`, 58 `virtual`/0
   `override`, 5 `enum` → `enum class` (`RenderType`, `BlendingMode`, `ParticleClipMode`, the two
   `Effect_*`); mechanical, `clang-tidy modernize-*` does most of it. Plus: drop `boost::thread` with the
   dead render-thread experiment (the only Boost in `video/`), and replace the hand-written libpng
   writer (`Save_Surface`, 80 lines) with `IMG_SavePNG` (SDL2_image ≥ 2.0.2).

## 11. Cross-references

**Depends on (includes from `video/`, `rg '#include "../'`):** `core/` ×32 (`global_basic.h`,
`global_game.h`, `game_core.h` — `pActive_Camera`, `game_res_w/h`, `global_upscalex`, `editor_enabled`;
`framerate.h` — `pFramerate`, `speedfactor_fps`, `Is_Frame_Time`; `main.h` — `Draw_Game`; `math/*`;
`filesystem/*`; `i18n.h`; `obj_manager.h`; `file_parser.h`), `objects/` ×2 (`sprite.h`,
`animated_sprite.h` — `cAnimation` *is a* sprite), `input/mouse.h` ×2 (`pMouseCursor->m_active_object`),
`gui/` ×2 (`hud.h` — `pHud_Debug` for the screenshot toast; `spinner.h` — the CEGUI factory),
`user/preferences.h`. Third party: SDL2 (+image, +ttf), OpenGL/GLU (+ the stale `GL/glx.h`), CEGUI 0.7
(`System`, `OpenGLRenderer`, `NullRenderer`, `DefaultResourceProvider`, `DefaultLogger`, `WindowManager`,
`SchemeManager`, `ImagesetManager`, `FontManager`, `WidgetLookManager`, `ProgressBar`, `Editbox`,
`Checkbox`, `Combobox`, `ListboxTextItem`, `XMLAttributes`/`XMLSerializer`, `colour`), libpng,
Boost.Thread (dead).

**Depended on by:** 52 files outside `video/` include a `video/` header — `renderer.h` 21,
`gl_surface.h` 20, `animation.h` 19, `font.h` 11, `video.h` 7, `img_settings.h` 1 (`core/editor.cpp`),
`color.h` 1. Globals consumed elsewhere: `pVideo` (init: `core/main.cpp`; `Draw_*`/`Get_Surface`/`Render`
everywhere), `pRenderer` (41 `Add` sites), `pFont` (44 `Render_Text` sites), `pGuiRenderer`
(`core/editor.cpp:149` builds a `cEditor_CEGUI_Texture : CEGUI::OpenGLTexture` from a `cGL_Surface`'s raw
`m_image` — the one place a GL texture id crosses into CEGUI), `pGuiSystem` (GUI code), `screen`
(`SDL_Window*`; `input/mouse.cpp` warps against it, `input/keyboard.cpp`), `pImage_Manager` (only
`gui/menu_data.cpp` for grab/restore), `pSettingsParser` (`core/editor.cpp`),
`pActive_Animation_Manager` (set by `level/`, `overworld/`, `gui/menu.cpp`, `core/main.cpp`; used by
objects that spawn effects). `Draw_Effect_Out/In` are called from `level/level.cpp`,
`level/level_manager.cpp`, `core/game_core.cpp`; `Loading_Screen_*` from `core/main.cpp`,
`core/game_core.cpp`, `gui/menu_data.cpp`. GL calls outside `video/`: three `glClearColor`
(`gui/menu.cpp:554`, `gui/menu_data.cpp:3418,3435`).
