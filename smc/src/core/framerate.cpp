/***************************************************************************
 * framerate.cpp  -  Framerate independant motion control
 *
 * Copyright (C) 2003 - 2009 Florian Richter
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
#include "../core/framerate.h"
// SDL
#include "SDL.h"

namespace SMC
{

/* *** *** *** *** *** *** cPerformance_Timer *** *** *** *** *** *** *** *** *** *** *** */

cPerformance_Timer :: cPerformance_Timer( void )
{
	Reset();
}

cPerformance_Timer :: ~cPerformance_Timer( void )
{

}

void cPerformance_Timer :: Reset( void )
{
	frame_counter = 0;
	ms_counter = 0;
	ms = 0;
}

void cPerformance_Timer :: Update( void )
{
	// count frame
	frame_counter++;

	// add milliseconds
	Uint32 new_ticks = SDL_GetTicks();
	ms_counter += new_ticks - pFramerate->perf_last_ticks;
	pFramerate->perf_last_ticks = new_ticks;

	// counted 100 frames
	if( frame_counter >= 100 )
	{
		ms = ms_counter;
		frame_counter = 0;
		ms_counter = 0;
	}
}


/* *** *** *** *** *** *** cFramerate *** *** *** *** *** *** *** *** *** *** *** */

cFramerate :: cFramerate( const float tfps /* = DESIRED_FPS */ )
{
	speedfactor = 0.1f;
	fps = 0.0f;
	fps_best = 0.0f;
	fps_worst = 100.0f;
	fps_average = 0;

	force_speedfactor = 0.0f;

	perf_last_ticks = 0;

	// create performance timers
	for( unsigned int i = 0; i < 24; i++ )
	{
		perf_timer.push_back( new cPerformance_Timer() );
	}

	Init( tfps );
}

cFramerate :: ~cFramerate( void )
{
	// clear performance timer
	for( Performance_Timer_List::iterator itr = perf_timer.begin(), itr_end = perf_timer.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	perf_timer.clear();
}

void cFramerate :: Init( const float tfps )
{
	fps_target = tfps;
	maxspeedfactor = tfps * 0.15f;
	currentticks = SDL_GetTicks();
	framedelay = currentticks;
	fps_average_framedelay = currentticks;
	frames_counted = 0;
}

void cFramerate :: Update( void )
{
	currentticks = SDL_GetTicks();

	// if speedfactor is forced
	if( force_speedfactor != 0.0f )
	{
		speedfactor = force_speedfactor;
	}
	// default speedfactor measuring
	else
	{
		// check if the update was too early for SDL
		if( currentticks == framedelay )
		{
			// prevent division by 0
			speedfactor = 0.001f;
		}
		else
		{
			// frame speedfactor calculation
			speedfactor = static_cast<float>(( currentticks - framedelay ) / ( 1000 / fps_target ));

			// minimum
			if( speedfactor < 0.001f )
			{
				speedfactor = 0.001f;
			}
			// maximum
			else if( speedfactor > maxspeedfactor ) 
			{
				speedfactor = maxspeedfactor;
			}
		}
	}

	// speedfactor based fps
	fps = fps_target / speedfactor;
	
	// calculate average fps every second
	if( currentticks - fps_average_framedelay > 1000 )
	{
		fps_average = frames_counted;

		fps_average_framedelay += 1000;
		frames_counted = 0;
	}
	// count a fps
	else
	{
		frames_counted++;
	}

	// best fps
	if( fps > fps_best )
	{
		fps_best = fps;
	}
	// worst fps
	else if( fps < fps_worst )
	{
		fps_worst = fps;
	}

	framedelay = currentticks;
}

void cFramerate :: Reset( void )
{
	framedelay = SDL_GetTicks();
	speedfactor = 0.1f;
	fps_best = 0;
	fps_worst = 100000;
	fps_average = 0;

	// reset performance timer
	for( Performance_Timer_List::iterator itr = perf_timer.begin(), itr_end = perf_timer.end(); itr != itr_end; ++itr )
	{
		(*itr)->Reset();
	}
}

void cFramerate :: Set_Max_Speed_Factor( const float maximum )
{
	maxspeedfactor = maximum;
}

void cFramerate :: Set_Fixed_Speedfacor( const float val )
{
	force_speedfactor = val;
}

void Correct_Frame_Time( const unsigned int fps )
{
	while( !Is_Frame_Time( fps ) )
	{
		SDL_Delay( 1 );
	}
}

bool Is_Frame_Time( const unsigned int fps )
{
	static Uint32 static_time = 0;

	if( SDL_GetTicks () - static_time < 1000 / fps )
	{
		return 0;
	}

	static_time = SDL_GetTicks();
	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cFramerate *pFramerate = NULL;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC
