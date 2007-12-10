/***************************************************************************
           joystick.cpp  -  Joystick class
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/globals.h"
#include "../input/keyboard.h"
#include "../input/joystick.h"
#include "../user/preferences.h"
#include "../core/game_core.h"
#include "../player/player.h"
#include "../gui/hud.h"
#include "../gui/menu.h"

/* *** *** *** *** *** *** cJoystick *** *** *** *** *** *** *** *** *** *** *** */

cJoystick :: cJoystick( void )
{
	joystick = NULL;

	stick_open = 0;

	cur_stick = 0;
	num_buttons = 0;
	num_axes = 0;
	num_balls = 0;

	debug = 0;

	Reset_keys();

	Init();
}

cJoystick :: ~cJoystick( void )
{
	Close();
}

int cJoystick :: Init( void )
{
	// if not enabled
	if( !pPreferences->joy_enabled )
	{
		return 0;
	}

	int joy_count = SDL_NumJoysticks();

	// no joystick available
	if( joy_count <= 0 )
    {
		printf( "No joysticks available\n" );
		pPreferences->joy_enabled = 0;
		return 0;
    }

	if( debug )
	{
		printf( "Joysticks found : %d\n\n", joy_count );
	}

	unsigned int default_joy = 0;

	// if default joy name is given
	if( !pPreferences->joy_name.empty() )
	{
		vector<string> joy_names = Get_Names();

		for( unsigned int i = 0; i < joy_names.size(); i++ )
		{
			// found default joy
			if( joy_names[i].compare( pPreferences->joy_name ) == 0 )
			{
				default_joy = i;
				break;
			}
		}
	}

	// setup
	SDL_JoystickEventState( SDL_ENABLE );
	Stick_Open( default_joy );

	if( debug )
	{
		printf( "Joypad System Initialized\n" );
	}

	return 1;
}

void cJoystick :: Close( void )
{
	Stick_Close();
}

bool cJoystick :: Stick_Open( unsigned int index )
{
	// if a joystick is already opened close it first
	if( stick_open )
	{
		Stick_Close();
	}

	joystick = SDL_JoystickOpen( index );

	if( !joystick )
	{
		printf( "Couldn't open joystick %d\n", index );
		stick_open = 0;
		return 0;
	}

	cur_stick = index;

	num_buttons = SDL_JoystickNumButtons( joystick );
	num_axes = SDL_JoystickNumAxes( joystick );
	num_balls = SDL_JoystickNumBalls( joystick );

	// setup available button
	for( unsigned int i = 0; i < num_buttons; i++ )
	{
		buttons.push_back( 0 );
	}

	if( debug )
	{
		printf( "Opened Joystick %d\n", cur_stick );
		printf( "Name: %s\n", Get_Name().c_str() );
		printf( "Number of Buttons: %d\n", num_buttons );
		printf( "Number of Axes: %d\n", num_axes );
		printf( "Number of Balls: %d\n\n", num_balls );
	}

	stick_open = 1;
	return 1;
}

void cJoystick :: Stick_Close( void )
{
	// not available
	if( !joystick )
	{
		return;
	}

	SDL_JoystickClose( joystick );
	joystick = NULL;

	Reset_keys();

	num_buttons = 0;
	num_axes = 0;
	num_balls = 0;

	buttons.clear();
	stick_open = 0;

	if( debug )
	{
		printf( "Joystick %d closed\n", cur_stick );
	}

	cur_stick = 0;
}

void cJoystick :: Reset_keys( void )
{
	// clear buttons
	for( unsigned int i = 0; i < buttons.size(); i++ )
	{
		buttons[i] = 0;
	}
	
	left = 0;
	right = 0;
	up = 0;
	down = 0;
}

bool cJoystick :: Handle_Event( SDL_Event *ev )
{
	// not enabled
	if( !pPreferences->joy_enabled )
	{
		return 0;
	}

	// not opened
	if( !stick_open )
	{
		if( debug )
		{
			printf( "No Joysticks opened : Joypad is now Disabled\n" );
		}

		pPreferences->joy_enabled = 0;
		return 0;
	}

	// handle events
	switch( ev->type )
	{
		case SDL_JOYBUTTONDOWN:
		{
			if( Handle_Button_Event( ev ) )
			{
				return 1;
			}
			break;
		}
		case SDL_JOYBUTTONUP:
		{
			if( Handle_Button_Event( ev ) )
			{
				return 1;
			}
			break;
		}
		case SDL_JOYAXISMOTION:
		{
			Handle_Motion( ev );
			break;
		}
		default:
		{
			break;
		}
	}

	return 0;
}

void cJoystick :: Handle_Motion( SDL_Event *ev )
{
	// Vertical Axis
	if( ev->jaxis.axis == pPreferences->joy_axis_ver )
	{
		// Up
		if( ev->jaxis.value < -pPreferences->joy_axis_threshold )
		{
			if( debug )
			{
				printf( "Joystick %d : Up Button pressed\n", cur_stick );
			}

			if( !up )
			{
				pKeyboard->Key_Down( pPreferences->key_up );
				up = 1;
			}

			if( down )
			{
				pKeyboard->Key_Up( pPreferences->key_down );
				down = 0;
			}
		}
		// Down
		else if( ev->jaxis.value > pPreferences->joy_axis_threshold )
		{
			if( debug )
			{
				printf( "Joystick %d : Down Button pressed\n", cur_stick );
			}

			if( !down )
			{
				pKeyboard->Key_Down( pPreferences->key_down );
				down = 1;
			}

			if( up )
			{
				pKeyboard->Key_Up( pPreferences->key_up );
				up = 0;
			}
		}
		// No Down/Left
		else
		{
			if( down )
			{
				pKeyboard->Key_Up( pPreferences->key_down );
				down = 0;
			}

			if( up )
			{
				pKeyboard->Key_Up( pPreferences->key_up );
				up = 0;
			}
		}
	}
	// Horizontal Axis
	else if( ev->jaxis.axis == pPreferences->joy_axis_hor )
	{
		// Left
		if( ev->jaxis.value < -pPreferences->joy_axis_threshold )
		{
			if( debug )
			{
				printf( "Joystick %d : Left Button pressed\n", cur_stick );
			}

			if( !left )
			{
				pKeyboard->Key_Down( pPreferences->key_left );
				left = 1;
			}

			if( right )
			{
				pKeyboard->Key_Up( pPreferences->key_right );
				right = 0;
			}
		}
		// Right
		else if( ev->jaxis.value > pPreferences->joy_axis_threshold )
		{
			if( debug )
			{
				printf( "Joystick %d : Right Button pressed\n", cur_stick );
			}

			if( !right )
			{
				pKeyboard->Key_Down( pPreferences->key_right );
				right = 1;
			}

			if( left )
			{
				pKeyboard->Key_Up( pPreferences->key_left );
				left = 0;
			}
		}
		// No Left/Right
		else
		{
			if( left )
			{
				pKeyboard->Key_Up( pPreferences->key_left );
				left = 0;
			}

			if( right )
			{
				pKeyboard->Key_Up( pPreferences->key_right );
				right = 0;
			}
		}
	}
}

bool cJoystick :: Handle_Button_Event( SDL_Event *ev )
{
	// not enabled or opened
	if( !pPreferences->joy_enabled || !stick_open )
	{
		return 0;
	}

	// handle button events
	switch( ev->type )
	{
	case SDL_JOYBUTTONDOWN:
	{
		Set_Button( ev->jbutton.button, 1 );

		if( ev->jbutton.button < buttons.size() )
		{
			// Jump
			if( ev->jbutton.button == pPreferences->joy_button_jump )
			{
				if( Game_Mode == MODE_LEVEL )
				{
					pPlayer->Action_Jump();
					return 1;
				}
			}
			// Shoot
			else if( ev->jbutton.button == pPreferences->joy_button_shoot )
			{
				if( Game_Mode == MODE_LEVEL )
				{
					pPlayer->Action_Shoot();
				}
				else
				{
					pKeyboard->Key_Down( pPreferences->key_shoot );
				}

				return 1;
			}
			// Request Itembox Item
			else if( ev->jbutton.button == pPreferences->joy_button_item )
			{
				if( Game_Mode == MODE_LEVEL )
				{
					Itembox->Request_Item();
				}
				else
				{
					pKeyboard->Key_Down( SDLK_RETURN );
				}

				return 1;
			}
			// Interaction
			else if( ev->jbutton.button == pPreferences->joy_button_action )
			{
				if( Game_Mode == MODE_LEVEL )
				{
					pPlayer->Action_Interact( pPreferences->key_action );
				}
				else
				{
					pKeyboard->Key_Down( pPreferences->key_action );
				}

				return 1;
			}
			// Exit
			else if( ev->jbutton.button == pPreferences->joy_button_exit )
			{
				if( Game_Mode == MODE_LEVEL )
				{
					Game_Action = GA_ENTER_MENU;
				}
				else
				{
					pKeyboard->Key_Down( SDLK_ESCAPE );
				}

				return 1;
			}
			else if( ev->jbutton.button == 9 )
			{
				pKeyboard->Key_Down( SDLK_PAUSE );
				return 1;
			}
		}
		break;
	}
	case SDL_JOYBUTTONUP:
	{
		Set_Button( ev->jbutton.button, 0 );

		if( ev->jbutton.button < buttons.size() )
		{
			if( ev->jbutton.button == pPreferences->joy_button_jump )
			{
				if( Game_Mode == MODE_LEVEL )
				{
					pPlayer->Stop_flying();
					pPlayer->UpKeytime = 0;
				}
				else
				{
					pKeyboard->Key_Up( pPreferences->key_jump );
				}

				return 1;
			}
			else if( ev->jbutton.button == pPreferences->joy_button_shoot )
			{
				pKeyboard->Key_Up( pPreferences->key_shoot );
				return 1;
			}
			else if( ev->jbutton.button == pPreferences->joy_button_item )
			{
				pKeyboard->Key_Up( SDLK_RETURN );
				return 1;
			}
			else if( ev->jbutton.button == pPreferences->joy_button_action )
			{
				if( Game_Mode == MODE_LEVEL )
				{
					pPlayer->Release_item();
				}
				else
				{
					pKeyboard->Key_Up( pPreferences->key_action );
				}

				return 1;
			}
			else if( ev->jbutton.button == pPreferences->joy_button_exit )
			{
				pKeyboard->Key_Up( SDLK_ESCAPE );
				return 1;
			}
		}
		break;
	}
	default:
	{
		break;
	}
	}

	return 0;
}

string cJoystick :: Get_Name( void )
{
	return SDL_JoystickName( cur_stick );
}

vector<string> cJoystick :: Get_Names( void )
{
	vector<string> names;
	// get joy count
	int joy_count = SDL_NumJoysticks();

	// joystick names
	for( int i = 0; i < joy_count; i++  )
    {
		names.push_back( SDL_JoystickName( i ) );
	}

	return names;
}

void cJoystick :: Set_Button( unsigned int num, bool pressed )
{
	// not available
	if( num >= buttons.size() )
	{
		return;
	}

	if( debug )
	{
		if( pressed )
		{
			printf( "Joystick %d : Joy Button %d pressed\n", cur_stick, num );
		}
		else
		{
			printf( "Joystick %d : Joy Button %d released\n", cur_stick, num );
		}
	}

	buttons[num] = pressed;
}

bool cJoystick :: Left( void )
{
	if( pPreferences->joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value < -pPreferences->joy_axis_threshold && 
		input_event.jaxis.axis == pPreferences->joy_axis_hor )
	{
		return 1;
	}

	return 0;
}

bool cJoystick :: Right( void )
{
	if( pPreferences->joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value > pPreferences->joy_axis_threshold && 
		input_event.jaxis.axis == pPreferences->joy_axis_hor )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Up( void )
{
	if( pPreferences->joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value < -pPreferences->joy_axis_threshold && 
		input_event.jaxis.axis == pPreferences->joy_axis_ver )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Down( void )
{
	if( pPreferences->joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value > pPreferences->joy_axis_threshold && 
		input_event.jaxis.axis == pPreferences->joy_axis_ver )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Button( unsigned int num )
{
	// if available and pressed
	if( num < buttons.size() && buttons[num] )
	{
		return 1;
	}

	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cJoystick *pJoystick = NULL;
