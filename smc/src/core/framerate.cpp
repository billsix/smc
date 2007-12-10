/***************************************************************************
	framerate.cpp  -  Framerate independant class and a time correcting function
                             -------------------
	copyright                : (C) 2003 - 2007 by Florian Richter
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

/* *** *** *** *** *** *** cFramerate *** *** *** *** *** *** *** *** *** *** *** */

cFramerate :: cFramerate( float tfps /* = DESIRED_FPS */ )
{
	speedfactor = 0.1f;
	fps = 0;
	fps_best = 0;
	fps_worst = 100;
	fps_average = 0;

	force_speedfactor = 0;

	Init( tfps );
}

cFramerate :: ~cFramerate ( void )
{
	//
}

void cFramerate :: Init( float tfps )
{
	targetfps = tfps;
	maxspeedfactor = tfps * 0.15f;
	framedelay = SDL_GetTicks();
}

void cFramerate :: Update( void )
{
	currentticks = SDL_GetTicks();

	// if speedfactor is forced
	if( force_speedfactor != 0 )
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
			speedfactor = 0.00001f;
		}
		else
		{
			// frame speedfactor calculation
			speedfactor = static_cast<float>(( currentticks - framedelay ) / ( 1000 / targetfps ));
		}

		if( speedfactor <= 0 )
		{
			speedfactor = 0.00001f;
		}
		else if( speedfactor > maxspeedfactor ) 
		{
			speedfactor = maxspeedfactor;
		}
	}

	// current frame fps
	fps = targetfps / speedfactor;
	
	// calculate average fps every second
	if( currentticks - fps_average_framedelay > 1000 )
	{
		fps_average = fps_average_counter / fps_average_times_counted;

		fps_average_framedelay = static_cast<float>(currentticks);
		fps_average_times_counted = 0;
		fps_average_counter = 0;
	}
	// count a fps
	else
	{
		fps_average_times_counted++;
		fps_average_counter += fps;
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
	fps_worst = 100;
	fps_average = 0;
}

void cFramerate :: SetMaxSpeedFactor( float maximum )
{
	maxspeedfactor = maximum;
}

/* Set the given fixed speedfactor
 * if value is 0 no fixed speedfactor will be used
*/
void cFramerate :: Set_Fixed_Speedfacor( float val )
{
	force_speedfactor = val;
}

/* Fixed framerate method */
void CorrectFrameTime( unsigned int fps )
{
	static Uint32 stime = 0;
	
	while( SDL_GetTicks () - stime < 1000 / fps )
	{
		SDL_Delay( 1 );
	}

	stime = SDL_GetTicks();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cFramerate *pFramerate = NULL;
