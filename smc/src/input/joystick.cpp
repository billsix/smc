/***************************************************************************
           joystick.cpp  -  Joystick class
                             -------------------
    copyright            : (C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../globals.h"
#include "../main.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cJoystick :: cJoystick( void )
{
	Joystick = NULL;

	Opened = 0;
	Threshold = 3000;

	cur_stick = 0;
	num_buttons = 0;
	num_axes = 0;
	num_balls = 0;

#ifdef _DEBUG
	debug = 1;
#else
	debug = 0;
#endif

	for( unsigned int i = 0; i < MAX_BUTTONS; i++ )
	{
		Buttons[i] = 0;
	}
	
	left = 0;
	right = 0;
	up = 0;
	down = 0;

	Init();
}

cJoystick :: ~cJoystick( void )
{
	Close();
}

int cJoystick :: Init( void )
{
	int joy = SDL_NumJoysticks();
	
	if( joy <= 0 )
    {
		printf( "No joysticks available\n" );
		pPreferences->Joy_enabled = 0;
		return 0;
    }

	if( debug )
	{
		printf( "Joysticks found : %d\n\n", joy );
	}

	SDL_JoystickEventState( SDL_ENABLE );
	OpenStick( 0 );

	if( debug )
	{
		printf( "Joypad System Initialised\n" );
	}

	return 1;
}

void cJoystick :: Close( void )
{
	CloseStick();
}

bool cJoystick :: OpenStick( unsigned int index )
{
	SDL_Joystick *Joystick = SDL_JoystickOpen( index );

	if( !Joystick )
	{
		printf( "Couldn't open joystick %d\n", index );
		Opened = 0;
		return 0;
	}

	cur_stick = index;

	num_buttons = SDL_JoystickNumButtons( Joystick );
	num_axes = SDL_JoystickNumAxes( Joystick );
	num_balls = SDL_JoystickNumBalls( Joystick );

	if( debug )
	{
		printf( "Opened Joystick %d\n", index );
		printf( "Name: %s\n", SDL_JoystickName( index ) );
		printf( "Number of Axes: %d\n", num_axes );
		printf( "Number of Buttons: %d\n", num_buttons );
		printf( "Number of Balls: %d\n\n", num_balls );
	}

	Opened = 1;

	return 1;
}

void cJoystick :: CloseStick( void )
{
	if( Joystick )
	{
		SDL_JoystickClose( Joystick );
		Joystick = NULL;

		Reset_keys();

		num_buttons = 0;
		num_axes = 0;
		num_balls = 0;

		Opened = 0;

		if( debug )
		{
			printf( "Joystick %d closed\n", cur_stick );
		}

		cur_stick = 0;
	}
}

void cJoystick :: Reset_keys( void )
{
	for( unsigned int i = 0; i < MAX_BUTTONS; i++ )
	{
		Buttons[i] = 0;
	}
	
	left = 0;
	right = 0;
	up = 0;
	down = 0;
}

void cJoystick :: Handle_Events( void )
{
	if( !pPreferences->Joy_enabled ) // Joystick Motion
	{
		return;
	}

	if( !Opened )
	{
		if( debug )
		{
			printf( "No Joysticks opened : Joypad is now Disabled\n" );
		}

		pPreferences->Joy_enabled = 0;
		return;
	}

	switch( event.type )
	{
		case SDL_JOYBUTTONDOWN:
		{
			Handle_Button_Event();
			break;
		}
		case SDL_JOYBUTTONUP:
		{
			Handle_Button_Event();
			break;
		}
		case SDL_JOYAXISMOTION:
		{
			Handle_Motion();
			break;
		}
		default:
		{
			break;
		}
	}
}

void cJoystick :: Handle_Motion( void )
{
	switch( event.jaxis.axis )
	{
		case 1: /* Up/Down */
		{
			if( event.jaxis.value < -Threshold ) // Up
			{
				if( debug )
				{
					printf( "Joystick %d : Up Button pressed\n", cur_stick );
				}

				if( !up )
				{
					KeyDown( pPreferences->Key_up );
				}

				if( down )
				{
					KeyUp( pPreferences->Key_down );
				}
				
				up = 1;
				down = 0;
			}
			else if( event.jaxis.value > Threshold * 2 ) // Down ( 2 times harder )
			{
				if( debug )
				{
					printf( "Joystick %d : Down Button pressed\n", cur_stick );
				}

				if( !down )
				{
					KeyDown( pPreferences->Key_down );
				}

				if( up )
				{
					KeyUp( pPreferences->Key_up );
				}

				up = 0;
				down = 1;
			}
			else // No Down/Left
			{
				if( down )
				{
					KeyUp( pPreferences->Key_down );
				}
				
				if( up )
				{
					KeyUp( pPreferences->Key_up );
				}

				up = 0;
				down = 0;
			}
			break;
		}
		case 0: /* Left/Right */
		{
			if( event.jaxis.value < -Threshold ) // Left
			{
				if( debug )
				{
					printf( "Joystick %d : Left Button pressed\n", cur_stick );
				}

				if( !left )
				{
					KeyDown( pPreferences->Key_left );
				}

				if( right )
				{
					KeyUp( pPreferences->Key_right );
				}
				
				left = 1;
				right = 0; 
			}
			else if( event.jaxis.value > Threshold ) // Right
			{
				if( debug )
				{
					printf( "Joystick %d : Right Button pressed\n", cur_stick );
				}

				if( !right )
				{
					KeyDown( pPreferences->Key_right );
				}

				if( left )
				{
					KeyUp( pPreferences->Key_left );
				}

				left = 0;
				right = 1; 
			}
			else // No Left/Right
			{
				if( left )
				{
					KeyUp( pPreferences->Key_left );
				}
				
				if( right )
				{
					KeyUp( pPreferences->Key_right );
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
void cJoystick :: Handle_Button_Event( void )
{
	if( !pPreferences->Joy_enabled || !Opened )
	{
		return;
	}

	switch( event.type )
	{
	case SDL_JOYBUTTONDOWN:
	{
		SetButton( event.jbutton.button, 1 );

		if( event.jbutton.button < MAX_BUTTONS )
		{
			if( event.jbutton.button == pPreferences->Joy_jump )
			{
				if( Game_Mode == MODE_LEVEL )
				{
					KeyDown( pPreferences->Key_up );
				}
			}
			else if( event.jbutton.button == pPreferences->Joy_shoot )
			{
				KeyDown( pPreferences->Key_shoot );
			}
			else if( event.jbutton.button == pPreferences->Joy_exit )
			{
				KeyDown( SDLK_ESCAPE );
			}
			else if( event.jbutton.button == pPreferences->Joy_item )
			{
				KeyDown( SDLK_RETURN );
			}
		}
		break;
	}
	case SDL_JOYBUTTONUP:
	{
		SetButton( event.jbutton.button, 0 );

		if( event.jbutton.button < MAX_BUTTONS )
		{
			if( event.jbutton.button == pPreferences->Joy_jump )
			{
				KeyUp( pPreferences->Key_up );
			}
			else if( event.jbutton.button == pPreferences->Joy_shoot )
			{
				KeyUp( pPreferences->Key_shoot );
			}
			else if( event.jbutton.button == pPreferences->Joy_exit )
			{
				KeyUp( SDLK_ESCAPE );
			}
			else if( event.jbutton.button == pPreferences->Joy_item )
			{
				KeyUp( SDLK_RETURN );
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

void cJoystick :: SetButton( unsigned int num, bool pressed )
{
	if( num > MAX_BUTTONS )
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

	Buttons[num] = pressed;
}

bool cJoystick :: Left( int nThreshold )
{
	if( pPreferences->Joy_enabled && event.type == SDL_JOYAXISMOTION && event.jaxis.value < -nThreshold && 
		event.jaxis.axis == 0 )
	{
		return 1;
	}

	return 0;
}

bool cJoystick :: Right( int nThreshold )
{
	if( pPreferences->Joy_enabled && event.type == SDL_JOYAXISMOTION && event.jaxis.value > nThreshold && 
		event.jaxis.axis == 0 )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Up( int nThreshold )
{
	if( pPreferences->Joy_enabled && event.type == SDL_JOYAXISMOTION && event.jaxis.value < -nThreshold && 
		event.jaxis.axis == 1 )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Down( int nThreshold )
{
	if( pPreferences->Joy_enabled && event.type == SDL_JOYAXISMOTION && event.jaxis.value > nThreshold 
			&& event.jaxis.axis == 1 )
	{
		return 1;
	}
	
	return 0;
}

bool cJoystick :: Button( unsigned int num )
{
	if( num < MAX_BUTTONS && Buttons[num] )
	{
		return 1;
	}

	return 0;
}
