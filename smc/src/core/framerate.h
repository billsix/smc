/***************************************************************************
	framerate.h  -  Framerate independant class and a time correcting function
	
	The Speedfactor is the heart of this class.
	It's a float with that you multiply all your motions. For instance, if the targetfps is 100,
	and the actual fps is 85, the speedfactor will be set to 100/85, or about 1.2.
	Multiply all your motion in the game at its lowest level with this value.
	
	copyright : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the LGPL; either version 2 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __FRAMERATE_H__
#define __FRAMERATE_H__

class cFramerate
{
public:
	cFramerate( float tfps = DESIRED_FPS );
	~cFramerate ( void );
	
	// Initialize with the given target fps
	void Init( float tfps );
	// update speedfactor
	void Update( void );
	// reset speedfactor and worst/best fps statistic
	void Reset( void );
	
	// set maximum allowed speedfactor
	void SetMaxSpeedFactor( float maximum );

	/* Set the given fixed speedfactor
	 * if value is 0 no fixed speedfactor will be used
	*/
	void Set_Fixed_Speedfacor( float val );

	// target fps for speedfactor calculations
	float targetfps;
	// current fps
	float fps;
	// best fps
	float fps_best;
	// worst fps
	float fps_worst;
	// average fps in the last second
	float fps_average;
	// delay since last calculation
	float fps_average_framedelay;
	// fps counted
	float fps_average_counter;
	// times fps were counted
	int fps_average_times_counted;

	// last updated ticks
	Uint32 currentticks;
	// last updated ticks for speedfactor calculation
	Uint32 framedelay;

	// current factor based on target fps
	float speedfactor;
	// maximum factor
	float maxspeedfactor;

	// fixed speedfactor value
	float force_speedfactor;
};

/* Fixed framerate method */
void CorrectFrameTime( unsigned int fps = DESIRED_FPS );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Framerate class
extern cFramerate *pFramerate;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
