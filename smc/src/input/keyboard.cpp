/***************************************************************************
			keyboard.cpp  -  keyboard functions
                             -------------------
    copyright            :	(C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/game_core.h"
#include "../core/main.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../player/player.h"
#include "../gui/menu.h"
#include "../overworld/overworld.h"
#include "../core/framerate.h"
#include "../audio/audio.h"
#include "../level/level.h"
#include "../user/preferences.h"
#include "../level/level_editor.h"
#include "../level/level_settings.h"

/* *** *** *** *** *** *** *** *** Functions  *** *** *** *** *** *** *** *** *** */

cKeyboard :: cKeyboard( void )
{
	Reset_keys();
}

cKeyboard :: ~cKeyboard( void )
{

}

void cKeyboard :: Reset_keys( void )
{
	// set all keys to 0
	memset( keys, 0, sizeof( keys ) );
}

bool cKeyboard :: CEGUI_Handle_Key_Up( SDLKey key )
{
	// inject the scancode directly
	if( pGuiSystem->injectKeyUp( SDLKey_to_CEGUIKey( key ) ) )
	{
		// input was processed by the gui system
		return 1;
	}

	return 0;
}

bool cKeyboard :: Key_Up( SDLKey key )
{
	// set key to 0
	keys[key] = 0;

	// input was processed by the gui system
	if( CEGUI_Handle_Key_Up( key ) )
	{
		return 1;
	}

	// handle key in the current mode
	if( Game_Mode == MODE_LEVEL )
	{
		// got processed
		if( pLevel->Key_Up( key ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_MENU )
	{
		// got processed
		if( pMenuCore->Key_Up( key ) )
		{
			return 1;
		}
	}

	return 0;
}

bool cKeyboard :: CEGUI_Handle_Key_Down( SDLKey key )
{
	// inject the scancode
	if( pGuiSystem->injectKeyDown( SDLKey_to_CEGUIKey( key ) ) == 1 )
	{
		// input got processed by the gui system
		return 1;
	}

	// use for translated unicode value
	if( ( input_event.key.keysym.unicode & 0xFF80 ) == 0 )
	{
		if( pGuiSystem->injectChar( input_event.key.keysym.unicode & 0x7F ) )
		{
			// input got processed by the gui system
			return 1;
		}
	}

	return 0;
}

bool cKeyboard :: Key_Down( SDLKey key )
{
	// input was processed by the gui system
	if( CEGUI_Handle_Key_Down( key ) )
	{
		return 1;
	}

	// set key to 1
	keys[key] = 1;

	// ## first the internal keys

	// game exit
	if( key == SDLK_F4 && input_event.key.keysym.mod & KMOD_ALT )
	{
		done = 1;
		return 1;
	}
	// fullscreen toggle
	else if( key == SDLK_RETURN && input_event.key.keysym.mod & KMOD_ALT )
	{
		pVideo->Toggle_Fullscreen();
		return 1;
	}

	// handle key in the current mode
	if( Game_Mode == MODE_LEVEL )
	{
		// processed by the level
		if( pLevel->Key_Down( key ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_OVERWORLD )
	{
		// processed by the overworld
		if( pActive_Overworld->Key_Down( key ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_MENU )
	{
		// processed by the menu
		if( pMenuCore->Key_Down( key ) )
		{
			return 1;
		}
	}
	else if( Game_Mode == MODE_LEVEL_SETTINGS )
	{
		// processed by the level settings
		if( pLevel_Editor->pSettings->Key_Down( key ) )
		{
			return 1;
		}
	}

	// set fixed speedfactor mode
	if( key == SDLK_F6 )
	{
		float fixed_speedfactor = string_to_float( Box_Text_Input( float_to_string( pFramerate->force_speedfactor ), "Set Fixed Speedfactor", 1 ) );

		// disable fixed speedfactor mode
		if( fixed_speedfactor <= 0 )
		{
			pFramerate->Set_Fixed_Speedfacor( 0 );
			debugdisplay->Set_Text( "Fixed Speedfactor disabled" );
		}
		// enable fixed speedfactor mode
		else
		{
			pFramerate->Set_Fixed_Speedfacor( fixed_speedfactor );
			debugdisplay->Set_Text( "Fixed Speedfactor enabled" );
		}
	}
	// take a screenshot
	else if( key == SDLK_PRINT )
	{
		pVideo->Save_Screenshot();
	}
	// pause the game
	else if( key == SDLK_PAUSE )
	{
		Draw_StaticText( "Pause", &yellow, &lightgreyalpha64 );
	}
	// load a level
	else if( key == SDLK_l && ( input_event.key.keysym.mod & KMOD_LCTRL || input_event.key.keysym.mod & KMOD_RCTRL ) &&
			!( Game_Mode == MODE_OVERWORLD && pOverworld_manager->debugmode ) && Game_Mode != MODE_LEVEL_SETTINGS )
	{
		string level_name = "Name";

		// valid level
		while( level_name.length() )
		{
			level_name = Box_Text_Input( level_name, "Load a Level", level_name.compare( "Name" ) == 0 ? 1 : 0 );

			// break if empty
			if( level_name.empty() )
			{
				break;
			}

			// if available
			if( pLevel->Get_path( level_name ) )
			{
				Game_Action = GA_ENTER_LEVEL;
				Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
				Game_Action_Data.add( "level", level_name.c_str() );

				debugdisplay->Set_Text( "Loaded " + Get_filename( level_name, 0, 0 ) );

				break;
			}
			// not found
			else
			{
				pAudio->PlaySound( "error.ogg" );
			}
		}
	}
	// load an overworld
	else if( key == SDLK_w && ( input_event.key.keysym.mod & KMOD_LCTRL || input_event.key.keysym.mod & KMOD_RCTRL ) &&
			!( Game_Mode == MODE_OVERWORLD && pOverworld_manager->debugmode ) && Game_Mode != MODE_LEVEL_SETTINGS )
	{
		string world_name = "Name";

		// valid world
		while( world_name.length() > 0 )
		{
			world_name = Box_Text_Input( world_name, "Load an Overworld", world_name.compare( "Name" ) == 0 ? 1 : 0 );

			// break if empty
			if( world_name.empty() )
			{
				break;
			}

			// success
			if( pOverworld_manager->Set_Active( world_name ) )
			{
				Game_Action = GA_ENTER_WORLD;

				debugdisplay->Set_Text( "Loaded " + world_name );

				break;
			}
			// failed
			else
			{
				pAudio->PlaySound( "error.ogg" );
			}
		}
	}
	// sound toggle
	else if( key == SDLK_F10 )
	{
		pAudio->ToggleSounds();

		if( !pAudio->sound_enabled )
		{
			debugdisplay->Set_Text( "Sound Disabled" );
		}
		else
		{
			debugdisplay->Set_Text( "Sound Enabled" );
		}
	}
	// music toggle
	else if( key == SDLK_F11 )
	{
		pAudio->ToggleMusic();

		if( !pAudio->music_enabled )
		{
			debugdisplay->Set_Text( "Music Disabled" );
		}
		else
		{
			debugdisplay->Set_Text( "Music Enabled" );
		}
	}
	// debug mode
	else if( key == SDLK_d && ( input_event.key.keysym.mod & KMOD_LCTRL || input_event.key.keysym.mod & KMOD_RCTRL ) )
	{
		if( Game_debug )
		{
			debugdisplay->Set_Text( "Debugmode disabled" );
		}
		else
		{
			pFramerate->fps_worst = 1000;
			pFramerate->fps_best = 0;
			debugdisplay->Set_Text( "Debugmode enabled" );
		}

		Game_debug = !Game_debug;
	}

	return 0;
}

unsigned int cKeyboard :: SDLKey_to_CEGUIKey( SDLKey key )
{
    switch( key )
    {
    case SDLK_BACKSPACE:    return Key::Backspace;
    case SDLK_TAB:          return Key::Tab;
    case SDLK_RETURN:       return Key::Return;
    case SDLK_PAUSE:        return Key::Pause;
    case SDLK_ESCAPE:       return Key::Escape;
    case SDLK_SPACE:        return Key::Space;
    case SDLK_COMMA:        return Key::Comma;
    case SDLK_MINUS:        return Key::Minus;
    case SDLK_PERIOD:       return Key::Period;
    case SDLK_SLASH:        return Key::Slash;
    case SDLK_0:            return Key::Zero;
    case SDLK_1:            return Key::One;
    case SDLK_2:            return Key::Two;
    case SDLK_3:            return Key::Three;
    case SDLK_4:            return Key::Four;
    case SDLK_5:            return Key::Five;
    case SDLK_6:            return Key::Six;
    case SDLK_7:            return Key::Seven;
    case SDLK_8:            return Key::Eight;
    case SDLK_9:            return Key::Nine;
    case SDLK_COLON:        return Key::Colon;
    case SDLK_SEMICOLON:    return Key::Semicolon;
    case SDLK_EQUALS:       return Key::Equals;
    case SDLK_LEFTBRACKET:  return Key::LeftBracket;
    case SDLK_BACKSLASH:    return Key::Backslash;
    case SDLK_RIGHTBRACKET: return Key::RightBracket;
    case SDLK_a:            return Key::A;
    case SDLK_b:            return Key::B;
    case SDLK_c:            return Key::C;
    case SDLK_d:            return Key::D;
    case SDLK_e:            return Key::E;
    case SDLK_f:            return Key::F;
    case SDLK_g:            return Key::G;
    case SDLK_h:            return Key::H;
    case SDLK_i:            return Key::I;
    case SDLK_j:            return Key::J;
    case SDLK_k:            return Key::K;
    case SDLK_l:            return Key::L;
    case SDLK_m:            return Key::M;
    case SDLK_n:            return Key::N;
    case SDLK_o:            return Key::O;
    case SDLK_p:            return Key::P;
    case SDLK_q:            return Key::Q;
    case SDLK_r:            return Key::R;
    case SDLK_s:            return Key::S;
    case SDLK_t:            return Key::T;
    case SDLK_u:            return Key::U;
    case SDLK_v:            return Key::V;
    case SDLK_w:            return Key::W;
    case SDLK_x:            return Key::X;
    case SDLK_y:            return Key::Y;
    case SDLK_z:            return Key::Z;
    case SDLK_DELETE:       return Key::Delete;
    case SDLK_KP0:          return Key::Numpad0;
    case SDLK_KP1:          return Key::Numpad1;
    case SDLK_KP2:          return Key::Numpad2;
    case SDLK_KP3:          return Key::Numpad3;
    case SDLK_KP4:          return Key::Numpad4;
    case SDLK_KP5:          return Key::Numpad5;
    case SDLK_KP6:          return Key::Numpad6;
    case SDLK_KP7:          return Key::Numpad7;
    case SDLK_KP8:          return Key::Numpad8;
    case SDLK_KP9:          return Key::Numpad9;
    case SDLK_KP_PERIOD:    return Key::Decimal;
    case SDLK_KP_DIVIDE:    return Key::Divide;
    case SDLK_KP_MULTIPLY:  return Key::Multiply;
    case SDLK_KP_MINUS:     return Key::Subtract;
    case SDLK_KP_PLUS:      return Key::Add;
    case SDLK_KP_ENTER:     return Key::NumpadEnter;
    case SDLK_KP_EQUALS:    return Key::NumpadEquals;
    case SDLK_UP:           return Key::ArrowUp;
    case SDLK_DOWN:         return Key::ArrowDown;
    case SDLK_RIGHT:        return Key::ArrowRight;
    case SDLK_LEFT:         return Key::ArrowLeft;
    case SDLK_INSERT:       return Key::Insert;
    case SDLK_HOME:         return Key::Home;
    case SDLK_END:          return Key::End;
    case SDLK_PAGEUP:       return Key::PageUp;
    case SDLK_PAGEDOWN:     return Key::PageDown;
    case SDLK_F1:           return Key::F1;
    case SDLK_F2:           return Key::F2;
    case SDLK_F3:           return Key::F3;
    case SDLK_F4:           return Key::F4;
    case SDLK_F5:           return Key::F5;
    case SDLK_F6:           return Key::F6;
    case SDLK_F7:           return Key::F7;
    case SDLK_F8:           return Key::F8;
    case SDLK_F9:           return Key::F9;
    case SDLK_F10:          return Key::F10;
    case SDLK_F11:          return Key::F11;
    case SDLK_F12:          return Key::F12;
    case SDLK_F13:          return Key::F13;
    case SDLK_F14:          return Key::F14;
    case SDLK_F15:          return Key::F15;
    case SDLK_NUMLOCK:      return Key::NumLock;
    case SDLK_SCROLLOCK:    return Key::ScrollLock;
    case SDLK_RSHIFT:       return Key::RightShift;
    case SDLK_LSHIFT:       return Key::LeftShift;
    case SDLK_RCTRL:        return Key::RightControl;
    case SDLK_LCTRL:        return Key::LeftControl;
    case SDLK_RALT:         return Key::RightAlt;
    case SDLK_LALT:         return Key::LeftAlt;
    case SDLK_LSUPER:       return Key::LeftWindows;
    case SDLK_RSUPER:       return Key::RightWindows;
    case SDLK_SYSREQ:       return Key::SysRq;
    case SDLK_MENU:         return Key::AppMenu;
    case SDLK_POWER:        return Key::Power;
    default:                return 0;
    }
    return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cKeyboard *pKeyboard = NULL;
