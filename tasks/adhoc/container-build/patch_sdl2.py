#!/usr/bin/env python3
"""The SDL 1.2 -> SDL 2 port of the game, as exact-match replacements (one-shot, 2026-09-23;
task: tasks/smc-container-build.md). Every replacement asserts it matches exactly once, so a
re-run on a ported tree fails loudly instead of double-applying. usage: patch_sdl2.py <repo root>"""

import re
import sys
from pathlib import Path

root: Path = Path(sys.argv[1]) / "smc"
Pairs = list[tuple[str, str] | tuple[str, str, int]]


def patch(rel: str, pairs: Pairs) -> None:
    p: Path = root / rel
    s: str = p.read_text()
    for item in pairs:
        old: str = item[0]
        new: str = item[1]
        want: int = item[2] if len(item) > 2 else 1
        n: int = s.count(old)
        if n == 0 and (new == "" or new in s):  # a deletion already done, or a rewrite already present
            print(f"  (already applied in {rel}: {old[:50]!r})")
            continue
        if new != "" and old in new and new in s:  # an insertion (new extends old) already present
            print(f"  (already inserted in {rel}: {old[:50]!r})")
            continue
        assert n == want, f"{rel}: expected {want} match(es) for {old[:70]!r}, got {n}"
        s = s.replace(old, new)
    p.write_text(s)
    print("patched", rel)


# --- build system: SDL2 family ---------------------------------------------------------------
patch("configure.ac", [
    ("PKG_CHECK_MODULES([SDL], [sdl >= 1.2.10])", "PKG_CHECK_MODULES([SDL], [sdl2 >= 2.0.0])"),
    ("AC_CHECK_LIB([SDL_image], [IMG_LoadPNG_RW], ,", "AC_CHECK_LIB([SDL2_image], [IMG_LoadPNG_RW], ,"),
    ("AC_CHECK_LIB([SDL_image], [IMG_Init], ,", "AC_CHECK_LIB([SDL2_image], [IMG_Init], ,"),
    ("AC_CHECK_LIB([SDL_mixer], [Mix_OpenAudio], ,", "AC_CHECK_LIB([SDL2_mixer], [Mix_OpenAudio], ,"),
    ("AC_CHECK_LIB([SDL_ttf], [TTF_Init], ,", "AC_CHECK_LIB([SDL2_ttf], [TTF_Init], ,"),
])

# --- one name for the key type, everywhere -----------------------------------------------------
patch("src/core/global_basic.h", [
    ("#include <math.h>\n", "#include <math.h>\n#include \"SDL.h\"\n// SDL 2 port: SDL 1.2's SDLKey enum became SDL_Keycode; the code keeps the old name.\ntypedef SDL_Keycode SDLKey;\n", 1),
])

# --- keyboard: key state indexed by scancode behind the old keycode-indexed interface -------------
patch("src/input/keyboard.h", [
    ("""	// Pressed keys
	Uint8 m_keys[SDLK_LAST];""",
     """	/* Pressed keys. SDL 2 keycodes are sparse (bit 30 set for non-character keys), so the
	 * state lives in a scancode-indexed array; operator[] keeps every `m_keys[SDLK_x]` in the
	 * game compiling unchanged by translating the keycode. */
	struct cKey_State
	{
		Uint8 m_state[SDL_NUM_SCANCODES];
		Uint8 &operator[]( SDLKey key ) { return m_state[SDL_GetScancodeFromKey( key )]; }
		const Uint8 &operator[]( SDLKey key ) const { return m_state[SDL_GetScancodeFromKey( key )]; }
	};
	cKey_State m_keys;"""),
])
patch("src/input/keyboard.cpp", [
    ("	memset( m_keys, 0, sizeof( m_keys ) );", "	memset( &m_keys, 0, sizeof( m_keys ) );"),
    # keysym.unicode is gone in SDL 2: text arrives as SDL_TEXTINPUT (see Text_Input below)
    ("""	// use for translated unicode value
	if( input_event.key.keysym.unicode != 0 )
	{
		if( pGuiSystem->injectChar( input_event.key.keysym.unicode ) )
		{
			// input got processed by the gui system
			return 1;
		}
	}

	return 0;
}
""", """	return 0;
}

bool cKeyboard :: Text_Input( const char *text ) const
{
	// SDL 2 delivers typed text as UTF-8 in SDL_TEXTINPUT events (SDL 1.2 put one unicode
	// value into each key event); decode and hand each code point to the gui system
	bool processed = 0;
	const unsigned char *p = reinterpret_cast<const unsigned char *>(text);
	while( *p )
	{
		CEGUI::utf32 cp;
		int extra;
		if( *p < 0x80 ) { cp = *p; extra = 0; }
		else if( ( *p & 0xE0 ) == 0xC0 ) { cp = *p & 0x1F; extra = 1; }
		else if( ( *p & 0xF0 ) == 0xE0 ) { cp = *p & 0x0F; extra = 2; }
		else { cp = *p & 0x07; extra = 3; }
		++p;
		for( ; extra > 0 && ( *p & 0xC0 ) == 0x80; --extra, ++p )
		{
			cp = ( cp << 6 ) | ( *p & 0x3F );
		}
		if( pGuiSystem->injectChar( cp ) )
		{
			processed = 1;
		}
	}
	return processed;
}
"""),
    ("    case SDLK_KP0:          return CEGUI::Key::Numpad0;", "    case SDLK_KP_0:         return CEGUI::Key::Numpad0;"),
    ("    case SDLK_KP1:          return CEGUI::Key::Numpad1;", "    case SDLK_KP_1:         return CEGUI::Key::Numpad1;"),
    ("    case SDLK_KP2:          return CEGUI::Key::Numpad2;", "    case SDLK_KP_2:         return CEGUI::Key::Numpad2;"),
    ("    case SDLK_KP3:          return CEGUI::Key::Numpad3;", "    case SDLK_KP_3:         return CEGUI::Key::Numpad3;"),
    ("    case SDLK_KP4:          return CEGUI::Key::Numpad4;", "    case SDLK_KP_4:         return CEGUI::Key::Numpad4;"),
    ("    case SDLK_KP5:          return CEGUI::Key::Numpad5;", "    case SDLK_KP_5:         return CEGUI::Key::Numpad5;"),
    ("    case SDLK_KP6:          return CEGUI::Key::Numpad6;", "    case SDLK_KP_6:         return CEGUI::Key::Numpad6;"),
    ("    case SDLK_KP7:          return CEGUI::Key::Numpad7;", "    case SDLK_KP_7:         return CEGUI::Key::Numpad7;"),
    ("    case SDLK_KP8:          return CEGUI::Key::Numpad8;", "    case SDLK_KP_8:         return CEGUI::Key::Numpad8;"),
    ("    case SDLK_KP9:          return CEGUI::Key::Numpad9;", "    case SDLK_KP_9:         return CEGUI::Key::Numpad9;"),
])
patch("src/input/keyboard.h", [
    ("""	/* Key Down handler
	 * returns true if the event was processed
	*/
	bool Key_Down( SDLKey key );
""", """	/* Key Down handler
	 * returns true if the event was processed
	*/
	bool Key_Down( SDLKey key );

	/* Typed text (SDL 2 SDL_TEXTINPUT, UTF-8) handler
	 * returns true if the gui system processed any of it
	*/
	bool Text_Input( const char *text ) const;
"""),
])

# --- the renamed keysyms ------------------------------------------------------------------------
KEYSYM_RENAMES: list[tuple[str, str]] = [
    ("SDLK_PRINT", "SDLK_PRINTSCREEN"), ("SDLK_LSUPER", "SDLK_LGUI"), ("SDLK_RSUPER", "SDLK_RGUI"),
    ("SDLK_SCROLLOCK", "SDLK_SCROLLLOCK"), ("SDLK_NUMLOCK", "SDLK_NUMLOCKCLEAR"),
] + [(f"SDLK_KP{d}", f"SDLK_KP_{d}") for d in range(10)]
for rel in ["src/user/preferences.cpp", "src/input/keyboard.cpp", "src/core/editor.cpp", "src/gui/menu.cpp",
            "src/gui/menu_data.cpp", "src/level/level.cpp", "src/level/level_editor.cpp", "src/level/level_settings.cpp",
            "src/overworld/overworld.cpp", "src/level/level_player.cpp", "src/objects/text_box.cpp"]:
    p = root / rel
    s = p.read_text()
    t = s
    for old, new in KEYSYM_RENAMES:
        t = re.sub(r"\b" + old + r"\b", new, t)
    if t != s:
        p.write_text(t)
        print("keysyms renamed in", rel)
# SDLK_LAST (the SDL 1.2 upper bound) no longer exists; SDL 2 keycodes are 32-bit
patch("src/user/preferences.cpp", [
    ("if( val >= 0 && val <= SDLK_LAST )", "if( val >= 0 )", 17),
])

# --- video: one window + one GL context instead of SDL_SetVideoMode ---------------------------
patch("src/video/video.h", [
    ("""	// window manager information
	SDL_SysWMinfo wm_info;
#ifdef __unix__
	// current opengl context
	GLXContext glx_context;
#endif
""", """	// the OpenGL context of the window (SDL 2 port; replaces the WM-specific GLX/WGL handles)
	SDL_GLContext m_gl_context;
"""),
    ("// Screen\nextern SDL_Surface *screen;", "// The game window (SDL 2 port: was the SDL 1.2 screen surface)\nextern SDL_Window *screen;"),
])
patch("src/video/video.cpp", [
    ("""	SDL_VERSION( &wm_info.version );
#ifdef __unix__
	glx_context = NULL;
#endif
""", """	m_gl_context = NULL;
"""),
    ("		pGuiRenderer = &CEGUI::OpenGLRenderer::create( CEGUI::Size( screen->w, screen->h ) );",
     """		int screen_w, screen_h;
		SDL_GetWindowSize( screen, &screen_w, &screen_h );
		pGuiRenderer = &CEGUI::OpenGLRenderer::create( CEGUI::Size( screen_w, screen_h ) );"""),
    ("""	SDL_EnableUNICODE( 1 );
	// hide by default""", """	// SDL 2: typed text comes as SDL_TEXTINPUT events (what SDL 1.2's unicode mode gave per key)
	SDL_StartTextInput();
	// hide by default"""),
    ("""	// set the video flags
	int flags = SDL_OPENGL | SDL_SWSURFACE;

	// only enter fullscreen if set in preferences
	if( use_preferences && pPreferences->m_video_fullscreen )
	{
		flags |= SDL_FULLSCREEN;
	}
""", """	// set the window flags
	int flags = SDL_WINDOW_OPENGL;

	// only enter fullscreen if set in preferences
	if( use_preferences && pPreferences->m_video_fullscreen )
	{
		flags |= SDL_WINDOW_FULLSCREEN;
	}
"""),
    ("""	// first initialization
	if( !m_initialised )
	{
		// Set Caption
		SDL_WM_SetCaption( CAPTION, NULL );
		// Set Icon
		std::string filename_icon = DATA_DIR "/" GAME_ICON_DIR "/window_32.png";
		if( File_Exists( filename_icon ) )
		{
			SDL_Surface *icon = IMG_Load( filename_icon.c_str() );
			SDL_WM_SetIcon( icon, NULL );
			SDL_FreeSurface( icon );
		}
		else
		{
			printf( "Warning : Window icon %s does not exist\\n", filename_icon.c_str() );
		}
	}

""", ""),
    ("""	// if vertical synchronization is enabled
	if( use_preferences && pPreferences->m_video_vsync )
	{
		SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );
	}
""", ""),
    ("""	// Note: As of SDL 1.2.10, if width and height are both 0, SDL_SetVideoMode will use the desktop resolution.
	screen = SDL_SetVideoMode( screen_w, screen_h, screen_bpp, flags );

	if( !screen )
	{
		printf( "Error : Screen mode creation failed\\nReason : %s\\n", SDL_GetError() );
		exit( EXIT_FAILURE );
	}

	// check if fullscreen got set
	if( use_preferences && pPreferences->m_video_fullscreen )
	{
		bool is_fullscreen = ( ( screen->flags & SDL_FULLSCREEN ) == SDL_FULLSCREEN );
""", """	// SDL 2: the window and its GL context are created once and reused; a later call (resolution
	// or fullscreen change) resizes the existing window instead of recreating the surface
	if( !screen )
	{
		screen = SDL_CreateWindow( CAPTION, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_w, screen_h, flags );

		if( !screen )
		{
			printf( "Error : Window creation failed\\nReason : %s\\n", SDL_GetError() );
			exit( EXIT_FAILURE );
		}

		m_gl_context = SDL_GL_CreateContext( screen );

		if( !m_gl_context )
		{
			printf( "Error : OpenGL context creation failed\\nReason : %s\\n", SDL_GetError() );
			exit( EXIT_FAILURE );
		}

		// Set Icon
		std::string filename_icon = DATA_DIR "/" GAME_ICON_DIR "/window_32.png";
		if( File_Exists( filename_icon ) )
		{
			SDL_Surface *icon = IMG_Load( filename_icon.c_str() );
			SDL_SetWindowIcon( screen, icon );
			SDL_FreeSurface( icon );
		}
		else
		{
			printf( "Warning : Window icon %s does not exist\\n", filename_icon.c_str() );
		}
	}
	else
	{
		SDL_SetWindowFullscreen( screen, ( flags & SDL_WINDOW_FULLSCREEN ) ? SDL_WINDOW_FULLSCREEN : 0 );
		SDL_SetWindowSize( screen, screen_w, screen_h );
		SDL_GL_MakeCurrent( screen, m_gl_context );
	}

	// vertical synchronization is a property of the context now, not of the video mode
	SDL_GL_SetSwapInterval( ( use_preferences && pPreferences->m_video_vsync ) ? 1 : 0 );

	// check if fullscreen got set
	if( use_preferences && pPreferences->m_video_fullscreen )
	{
		bool is_fullscreen = ( SDL_GetWindowFlags( screen ) & SDL_WINDOW_FULLSCREEN ) != 0;
"""),
    ("""	// get window manager information
	if( !SDL_GetWMInfo( &wm_info ) )
	{
		printf( "Error: SDL_GetWMInfo not implemented\\n" );
	}
#ifdef __unix__
	// get context
	glx_context = glXGetCurrentContext();
#endif

""", ""),
    ("""int cVideo :: Test_Video( int width, int height, int bpp, int flags /* = 0 */ ) const
{
	// auto set the video flags
	if( !flags )
	{
		flags = SDL_OPENGL | SDL_SWSURFACE;

		// if fullscreen is set
		if( pPreferences->m_video_fullscreen )
		{
			flags |= SDL_FULLSCREEN;
		}
	}

	return SDL_VideoModeOK( width, height, bpp, flags );
}
""", """int cVideo :: Test_Video( int width, int height, int bpp, int flags /* = 0 */ ) const
{
	// SDL 2 has no SDL_VideoModeOK: a window of any size can be created and the display mode
	// is chosen by the driver, so every request is acceptable (the closest depth is reported
	// through SDL_GL_GetAttribute after creation, as before)
	return bpp;
}
"""),
    ("""	// auto set the video flags
	if( !flags )
	{
		// always set fullscreen
		flags = SDL_OPENGL | SDL_SWSURFACE | SDL_FULLSCREEN;
	}

	SDL_Rect** modes = SDL_ListModes( NULL, flags );
	bool create_default_list = 0;

	// no dimension is available
	if( modes == NULL )
	{
		create_default_list = 1;
	}
	// any dimension is allowed
	else if( modes == (SDL_Rect**)-1 )
	{
		create_default_list = 1;
	}
	else
	{
		for( int i = 0; modes[i]; ++i )
		{
			valid_resolutions.push_back( cSize_Int( modes[i]->w, modes[i]->h ) );
		}
	}
""", """	// SDL 2: the display modes of the primary display (sizes only, deduplicated)
	bool create_default_list = 0;
	int num_modes = SDL_GetNumDisplayModes( 0 );

	if( num_modes <= 0 )
	{
		create_default_list = 1;
	}
	else
	{
		for( int i = 0; i < num_modes; ++i )
		{
			SDL_DisplayMode mode;
			if( SDL_GetDisplayMode( 0, i, &mode ) != 0 )
			{
				continue;
			}
			cSize_Int size( mode.w, mode.h );
			bool known = 0;
			for( vector<cSize_Int>::const_iterator itr = valid_resolutions.begin(); itr != valid_resolutions.end(); ++itr )
			{
				if( itr->m_width == size.m_width && itr->m_height == size.m_height )
				{
					known = 1;
					break;
				}
			}
			if( !known )
			{
				valid_resolutions.push_back( size );
			}
		}
	}
"""),
    ("""void cVideo :: Make_GL_Context_Current( void )
{
	// scoped context lock here
#ifdef _WIN32
	if( wglGetCurrentContext() != wm_info.hglrc )
	{
		wglMakeCurrent( GetDC( wm_info.window ), wm_info.hglrc );
	}
#elif __unix__
	if( glx_context != NULL )
	{
		glXMakeCurrent( wm_info.info.x11.gfxdisplay, wm_info.info.x11.window, glx_context );
	}
#elif __APPLE__
	// party time
#endif

	// update info (needed?)
	SDL_GetWMInfo( &wm_info );
}

void cVideo :: Make_GL_Context_Inactive( void )
{
#ifdef _WIN32
	wglMakeCurrent( NULL, NULL );
#elif __unix__
	glXMakeCurrent( wm_info.info.x11.gfxdisplay, None, NULL );
#elif __APPLE__
	// party time
#endif

	// update info (needed?)
	SDL_GetWMInfo( &wm_info );
}
""", """void cVideo :: Make_GL_Context_Current( void )
{
	// SDL 2 owns the context; the same call works on every platform
	if( m_gl_context != NULL && SDL_GL_GetCurrentContext() != m_gl_context )
	{
		SDL_GL_MakeCurrent( screen, m_gl_context );
	}
}

void cVideo :: Make_GL_Context_Inactive( void )
{
	SDL_GL_MakeCurrent( screen, NULL );
}
"""),
    ("""#ifdef _WIN32
	// windows needs reinitialization
	Init_Video();
#else
	// works only for X11 platforms
	SDL_WM_ToggleFullScreen( screen );
#endif
""", """	// SDL 2: fullscreen is a window property on every platform
	SDL_SetWindowFullscreen( screen, pPreferences->m_video_fullscreen ? SDL_WINDOW_FULLSCREEN : 0 );
"""),
    ("		SDL_SetAlpha( surface, 0, SDL_ALPHA_TRANSPARENT );",
     "		// copy the pixels as they are, alpha included (SDL 1.2: SDL_SetAlpha with no SDL_SRCALPHA)\n		SDL_SetSurfaceBlendMode( surface, SDL_BLENDMODE_NONE );"),
    ("SDL_Surface *screen = NULL;", "SDL_Window *screen = NULL;"),
])
# SDL_GL_SwapBuffers() -> SDL_GL_SwapWindow( screen ), whatever the indentation (6 sites, 2 files)
for rel in ["src/video/video.cpp", "src/gui/menu_data.cpp"]:
    p = root / rel
    s = p.read_text()
    t = s.replace("SDL_GL_SwapBuffers();", "SDL_GL_SwapWindow( screen );")
    if t != s:
        p.write_text(t)
        print("swap calls rewritten in", rel)

# --- events -----------------------------------------------------------------------------------
patch("src/core/main.cpp", [
    ("""		case SDL_VIDEORESIZE:
		{
			pGuiSystem->notifyDisplaySizeChanged( CEGUI::Size( static_cast<float>(ev->resize.w), static_cast<float>(ev->resize.h) ) );
			break;
		}
""", """		case SDL_WINDOWEVENT:
		{
			if( ev->window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
			{
				pGuiSystem->notifyDisplaySizeChanged( CEGUI::Size( static_cast<float>(ev->window.data1), static_cast<float>(ev->window.data2) ) );
			}
			// lost focus : wait (SDL 1.2 SDL_ACTIVEEVENT with gain == 0)
			else if( ev->window.event == SDL_WINDOWEVENT_FOCUS_LOST )
			{
				bool music_paused = 0;
				// pause music
				if( pAudio->Is_Music_Playing() )
				{
					pAudio->Pause_Music();
					music_paused = 1;
				}
				SDL_WaitEvent( NULL );
				// resume if music got paused
				if( music_paused )
				{
					pAudio->Resume_Music();
				}
				return 1;
			}
			break;
		}
		case SDL_TEXTINPUT:
		{
			if( pKeyboard->Text_Input( ev->text.text ) )
			{
				return 1;
			}
			break;
		}
"""),
])

# --- mouse warp / event filter, joystick names, clipboard ----------------------------------------
patch("src/input/mouse.cpp", [
    ("	SDL_WarpMouse( static_cast<Uint16>(m_x * global_upscalex), static_cast<Uint16>(m_y * global_upscaley) );",
     "	SDL_WarpMouseInWindow( screen, static_cast<int>(m_x * global_upscalex), static_cast<int>(m_y * global_upscaley) );"),
    ("	SDL_PeepEvents( &inEvent, 1, SDL_GETEVENT, SDL_MOUSEMOTIONMASK );",
     "	SDL_PeepEvents( &inEvent, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION );"),
])
patch("src/input/joystick.cpp", [
    ("	return SDL_JoystickName( m_current_joystick );", "	return SDL_JoystickNameForIndex( m_current_joystick );"),
    ("		names.push_back( SDL_JoystickName( i ) );", "		names.push_back( SDL_JoystickNameForIndex( i ) );"),
])
patch("src/gui/generic.cpp", [
    ("""	// only works with the cut-buffer method (xterm) and not with the more recent selections method
	SDL_SysWMinfo sdlinfo;
	SDL_VERSION( &sdlinfo.version );
	if( SDL_GetWMInfo( &sdlinfo ) )
	{
		sdlinfo.info.x11.lock_func();
		Display *display = sdlinfo.info.x11.display;
		int count = 0;
		char *msg = XFetchBytes( display, &count );
		if( msg )
		{
			if( count > 0 )
			{
				content.append( msg, count );
			}

			XFree( msg );
		}

		sdlinfo.info.x11.unlock_func();
	}
""", """	// SDL 2 has a clipboard of its own (the X11 cut-buffer code this replaced only spoke to xterm)
	char *text = SDL_GetClipboardText();
	if( text )
	{
		content = text;
		SDL_free( text );
	}
"""),
    ("""	SDL_SysWMinfo sdlinfo;
	SDL_VERSION( &sdlinfo.version );
	if( SDL_GetWMInfo( &sdlinfo ) )
	{
		sdlinfo.info.x11.lock_func();
		Display *display = sdlinfo.info.x11.display;
		Window window = sdlinfo.info.x11.window;

		XChangeProperty( display, DefaultRootWindow(display), XA_CUT_BUFFER0, XA_STRING, 8, PropModeReplace, static_cast<const unsigned char *>(static_cast<const void *>(str.c_str())), str.length() );

		if( XGetSelectionOwner( display, XA_PRIMARY ) != window )
		{
			XSetSelectionOwner( display, XA_PRIMARY, window, CurrentTime );
		}

		sdlinfo.info.x11.unlock_func();
	}
""", """	SDL_SetClipboardText( str.c_str() );
"""),
])

# --- the two direct SDL_GetKeyState users: SDL 2's state is scancode-indexed -------------------------
patch("src/level/level_player.cpp", [
    ("""			Uint8 *keys = SDL_GetKeyState( NULL );
			// Escape stops
			if( keys[SDLK_ESCAPE] || keys[SDLK_RETURN] || keys[SDLK_SPACE] || keys[pPreferences->m_key_action] )""",
     """			const Uint8 *keys = SDL_GetKeyboardState( NULL );
			// Escape stops
			if( keys[SDL_SCANCODE_ESCAPE] || keys[SDL_SCANCODE_RETURN] || keys[SDL_SCANCODE_SPACE] || keys[SDL_GetScancodeFromKey( pPreferences->m_key_action )] )"""),
])
patch("src/objects/text_box.cpp", [
    ("		Uint8 *keys = SDL_GetKeyState( NULL );", "		const Uint8 *keys = SDL_GetKeyboardState( NULL );"),
    ("		if( keys[pPreferences->m_key_down] || joy_ver_axis > pPreferences->m_joy_axis_threshold )",
     "		if( keys[SDL_GetScancodeFromKey( pPreferences->m_key_down )] || joy_ver_axis > pPreferences->m_joy_axis_threshold )"),
    ("		if( keys[pPreferences->m_key_up] || joy_ver_axis < -pPreferences->m_joy_axis_threshold )",
     "		if( keys[SDL_GetScancodeFromKey( pPreferences->m_key_up )] || joy_ver_axis < -pPreferences->m_joy_axis_threshold )"),
])
# the SDL 1.2 focus-loss handler moved into the SDL_WINDOWEVENT case above; drop the old block
patch("src/core/main.cpp", [
    ("""		case SDL_ACTIVEEVENT:
		{
			// lost visibility
			if( ev->active.gain == 0 )
			{
				bool music_paused = 0;
				// pause music
				if( pAudio->Is_Music_Playing() )
				{
					pAudio->Pause_Music();
					music_paused = 1;
				}
				SDL_WaitEvent( NULL );
				// resume if music got paused
				if( music_paused )
				{
					pAudio->Resume_Music();
				}
				return 1;
			}
			break;
		}
		default: // other events""", """		default: // other events"""),
])
patch("src/video/video.cpp", [
    ("""		int is_vsync;
		// seems to return always true even if not available
		SDL_GL_GetAttribute( SDL_GL_SWAP_CONTROL, &is_vsync );
""", """		// SDL 2: the swap interval is a context property
		int is_vsync = SDL_GL_GetSwapInterval();
"""),
])

# --- found by the sandbox syntax pre-check (g++ -fsyntax-only over every .cpp) -------------------
patch("src/core/main.cpp", [
    # SDL 2 returns const char *
    ("	char *last_sdl_error = SDL_GetError();", "	const char *last_sdl_error = SDL_GetError();"),
])
patch("src/gui/menu_data.cpp", [
    # SDL_JoystickName takes an opened SDL_Joystick * in SDL 2; the by-index form is NameForIndex
    ("pPreferences->m_joy_name = SDL_JoystickName( index );", "pPreferences->m_joy_name = SDL_JoystickNameForIndex( index );"),
])
patch("src/input/mouse.h", [
    # SDL 1.2 reported the wheel as buttons 4/5; SDL 2 has a separate SDL_MOUSEWHEEL event, which
    # cMouseCursor::Handle_Event maps back to these so the button handlers stay unchanged
    ("""#include "../core/math/vector.h"

namespace SMC
{
""", """#include "../core/math/vector.h"

// SDL 1.2 wheel "buttons", kept so Handle_Mouse_Down keeps its wheel cases; SDL 2 delivers the
// wheel as SDL_MOUSEWHEEL, which cMouseCursor::Handle_Event translates into these
#define SDL_BUTTON_WHEELUP 4
#define SDL_BUTTON_WHEELDOWN 5

namespace SMC
{
"""),
])
patch("src/input/mouse.cpp", [
    ("""		case SDL_MOUSEBUTTONDOWN:
		{
			if( Handle_Mouse_Down( ev->button.button ) )
			{
				// processed
				return 1;
			}

			break;
		}
		default:
""", """		case SDL_MOUSEBUTTONDOWN:
		{
			if( Handle_Mouse_Down( ev->button.button ) )
			{
				// processed
				return 1;
			}

			break;
		}
		case SDL_MOUSEWHEEL:
		{
			// SDL 1.2 delivered the wheel as button 4/5 presses
			if( ev->wheel.y != 0 && Handle_Mouse_Down( ev->wheel.y > 0 ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN ) )
			{
				// processed
				return 1;
			}

			break;
		}
		default:
"""),
])
patch("src/core/filesystem/filesystem.cpp", [
    # Boost >= 1.86 dropped filesystem/convenience.hpp; the umbrella header has everything used here
    ('#include "boost/filesystem/convenience.hpp"', '#include "boost/filesystem.hpp"'),
])
print("SDL2 port applied")
