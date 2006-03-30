/***************************************************************************
	framerate.h  -  Framerate independant class and a time correcting function
	
	The Speedfactor is the heart of this class. When it is set by SetSpeedFactor, 
	it becomes a number with that you multiply all your motions. For instance, if the targetfps is 100,
	and the actual fps is 85, the speedfactor will be set to 100/85, or about 1.2.
	You then multiply all your motion is the game, at its lowest level, by this number.
	
	
	copyright : (C) 2003 - 2006 by FluXy

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the LGPL.                                       *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __EP_FRAMERATE_H__
#define __EP_FRAMERATE_H__

class cFramerate
{
public:
	cFramerate( double tfps )
	{
		speedfactor = 1;
		fps = 0;
		fps_best = 0;
		fps_worst = 100;
		fps_average = 0;

		Init( tfps );
	}
	
	~cFramerate ( void )
	{
		//
	}
	
	void Init( double tfps )
	{
		targetfps = tfps;
		maxspeedfactor = tfps / 6;
		framedelay = SDL_GetTicks();
	}

	void Update( void )
	{
		currentticks = SDL_GetTicks();

		// check if the update was too early for SDL
		if( currentticks == framedelay )
		{
			// prevent division by 0
			currentticks++;
		}

		// current frame speedfactor calculation
		speedfactor = (double)( currentticks - framedelay ) / ( 1000 / targetfps );

		if( speedfactor <= 0 )
		{
			speedfactor = 0.000001;
		}
		else if( speedfactor > maxspeedfactor ) 
		{
			speedfactor = maxspeedfactor;
		}

		// current frame fps
		fps = targetfps / speedfactor;
		
		// calculate average fps every second
		if( currentticks - fps_average_framedelay > 1000 )
		{
			fps_average = fps_average_counter / fps_average_times_counted;

			fps_average_framedelay = currentticks;
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

	void Reset( void )
	{
		framedelay = SDL_GetTicks();
		speedfactor = 1;
		fps_best = 0;
		fps_worst = 100;
		fps_average = 0;
	}
	
	void SetMaxSpeedFactor( double maximum )
	{
		maxspeedfactor = maximum;
	}

	// target fps for speedfactor calculations
	double targetfps;
	// current fps
	double fps;
	// best fps
	double fps_best;
	// worst fps
	double fps_worst;
	// average fps in the last second
	double fps_average;
	// delay since last calculation
	double fps_average_framedelay;
	// fps counted
	double fps_average_counter;
	// times fps were counted
	int fps_average_times_counted;

	// last updated ticks
	Uint32 currentticks;
	// last updated ticks for speedfactor calculation
	Uint32 framedelay;

	// current factor based on target fps
	double speedfactor;
	// maximum factor
	double maxspeedfactor;
};

/* Fixed framerate method */
inline void CorrectFrameTime( unsigned int fps = 32 )
{
	static Uint32 stime = 0;
	
	while( SDL_GetTicks () - stime < 1000 / fps )
	{
		SDL_Delay( 1 );
	}

	stime = SDL_GetTicks();
}

#endif
