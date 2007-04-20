/***************************************************************************
           joystick.cpp  -  Joystick class
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../core/globals.h"
#include "../input/keyboard.h"
#include "../input/joystick.h"
#include "../user/preferences.h"
#include "../core/game_core.h"

/* *** *** *** *** *** *** cJoystick *** *** *** *** *** *** *** *** *** *** *** */

cJoystick :: cJoystick( void )
{
	joystick = NULL;

	stick_open = 0;
	Threshold = 3000;

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
			Handle_Button_Event( ev );
			break;
		}
		case SDL_JOYBUTTONUP:
		{
			Handle_Button_Event( ev );
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
	// handle motion
	switch( ev->jaxis.axis )
	{
		case 1: /* Up/Down */
		{
			if( ev->jaxis.value < -Threshold ) // Up
			{
				if( debug )
				{
					printf( "Joystick %d : Up Button pressed\n", cur_stick );
				}

				if( !up )
				{
					pKeyboard->Key_Down( pPreferences->key_up );
				}

				if( down )
				{
					pKeyboard->Key_Up( pPreferences->key_down );
				}
				
				up = 1;
				down = 0;
			}
			else if( ev->jaxis.value > Threshold * 2 ) // Down ( 2 times harder )
			{
				if( debug )
				{
					printf( "Joystick %d : Down Button pressed\n", cur_stick );
				}

				if( !down )
				{
					pKeyboard->Key_Down( pPreferences->key_down );
				}

				if( up )
				{
					pKeyboard->Key_Up( pPreferences->key_up );
				}

				up = 0;
				down = 1;
			}
			else // No Down/Left
			{
				if( down )
				{
					pKeyboard->Key_Up( pPreferences->key_down );
				}
				
				if( up )
				{
					pKeyboard->Key_Up( pPreferences->key_up );
				}

				up = 0;
				down = 0;
			}
			break;
		}
		case 0: /* Left/Right */
		{
			if( ev->jaxis.value < -Threshold ) // Left
			{
				if( debug )
				{
					printf( "Joystick %d : Left Button pressed\n", cur_stick );
				}

				if( !left )
				{
					pKeyboard->Key_Down( pPreferences->key_left );
				}

				if( right )
				{
					pKeyboard->Key_Up( pPreferences->key_right );
				}
				
				left = 1;
				right = 0; 
			}
			else if( ev->jaxis.value > Threshold ) // Right
			{
				if( debug )
				{
					printf( "Joystick %d : Right Button pressed\n", cur_stick );
				}

				if( !right )
				{
					pKeyboard->Key_Down( pPreferences->key_right );
				}

				if( left )
				{
					pKeyboard->Key_Up( pPreferences->key_left );
				}

				left = 0;
				right = 1; 
			}
			else // No Left/Right
			{
				if( left )
				{
					pKeyboard->Key_Up( pPreferences->key_left );
				}
				
				if( right )
				{
					pKeyboard->Key_Up( pPreferences->key_right );
				}
				
				left = 0;
				right = 0; 
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

void cJoystick :: Handle_Button_Event( SDL_Event *ev )
{
	// not enabled or opened
	if( !pPreferences->joy_enabled || !stick_open )
	{
		return;
	}

	// handle button events
	switch( ev->type )
	{
	case SDL_JOYBUTTONDOWN:
	{
		Set_Button( ev->jbutton.button, 1 );

		if( ev->jbutton.button < buttons.size() )
		{
			if( ev->jbutton.button == pPreferences->joy_jump )
			{
				if( Game_Mode == MODE_LEVEL )
				{
					pKeyboard->Key_Down( pPreferences->key_jump );
				}
			}
			else if( ev->jbutton.button == pPreferences->joy_shoot )
			{
				pKeyboard->Key_Down( pPreferences->key_shoot );
			}
			else if( ev->jbutton.button == pPreferences->joy_item )
			{
				pKeyboard->Key_Down( SDLK_RETURN );
			}
			else if( ev->jbutton.button == pPreferences->joy_action )
			{
				pKeyboard->Key_Down( pPreferences->key_action );
			}
			else if( ev->jbutton.button == pPreferences->joy_exit )
			{
				pKeyboard->Key_Down( SDLK_ESCAPE );
			}
			else if( ev->jbutton.button == 9 )
			{
				pKeyboard->Key_Down( SDLK_PAUSE );
			}

		}
		break;
	}
	case SDL_JOYBUTTONUP:
	{
		Set_Button( ev->jbutton.button, 0 );

		if( ev->jbutton.button < buttons.size() )
		{
			if( ev->jbutton.button == pPreferences->joy_jump )
			{
				pKeyboard->Key_Up( pPreferences->key_jump );
			}
			else if( ev->jbutton.button == pPreferences->joy_shoot )
			{
				pKeyboard->Key_Up( pPreferences->key_shoot );
			}
			else if( ev->jbutton.button == pPreferences->joy_item )
			{
				pKeyboard->Key_Up( SDLK_RETURN );
			}
			else if( ev->jbutton.button == pPreferences->joy_action )
			{
				pKeyboard->Key_Up( pPreferences->key_action );
			}
			else if( ev->jbutton.button == pPreferences->joy_exit )
			{
				pKeyboard->Key_Up( SDLK_ESCAPE );
			}
		}
		break;
	}
	default:
	{
		break;
	}
	}
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

bool cJoystick :: Left( int nThreshold )
{
	if( pPreferences->joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value < -nThreshold && 
		input_event.jaxis.axis == 0 )
	{
		return 1;
	}

	return 0;
}

bool cJoystick :: Right( int nThreshold )
{
	if( pPreferences->joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value > nThreshold && 
		input_event.jaxis.axis == 0 )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Up( int nThreshold )
{
	if( pPreferences->joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value < -nThreshold && 
		input_event.jaxis.axis == 1 )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Down( int nThreshold )
{
	if( pPreferences->joy_enabled && input_event.type == SDL_JOYAXISMOTION && input_event.jaxis.value > nThreshold 
			&& input_event.jaxis.axis == 1 )
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
