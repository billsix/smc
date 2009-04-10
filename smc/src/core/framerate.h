/***************************************************************************
 * framerate.h  -  header for the corresponding cpp file
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

#ifndef SMC_FRAMERATE_H
#define SMC_FRAMERATE_H

#include "SDL.h"

namespace SMC
{

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// counts milliseconds for 100 frames and sets them to ms
class cPerformance_Timer
{
public:
	cPerformance_Timer( void );
	~cPerformance_Timer( void );

	// reset
	void Reset( void );

	// Update and set new framerate ticks
	void Update( void );

	// current frame counter
	Uint32 frame_counter;
	// current milliseconds per frames counted
	Uint32 ms_counter;
	// milliseconds per 100 frames
	Uint32 ms;
};

/* The Speedfactor is the heart of this class.
 * It's a float with that you multiply all your motions. For instance, if the targetfps is 100,
 * and the actual fps is 80, the speedfactor will be set to 100/80, or 1.25.
*/
class cFramerate
{
public:
	cFramerate( const float tfps = speedfactor_fps );
	~cFramerate( void );
	
	// Initialize with the given target fps
	void Init( const float tfps );
	// update speedfactor
	void Update( void );
	// reset speedfactor and worst/best fps statistic
	void Reset( void );
	
	// set maximum allowed speedfactor
	void Set_Max_Speed_Factor( const float maximum );

	/* Set the given fixed speedfactor
	 * if value is 0 no fixed speedfactor will be used
	*/
	void Set_Fixed_Speedfacor( const float val );

	// target fps for speedfactor calculations
	float fps_target;
	// current fps
	float fps;
	// best fps
	float fps_best;
	// worst fps
	float fps_worst;
	// average fps in the last second
	unsigned int fps_average;
	// delay since last average fps calculation
	Uint32 fps_average_framedelay;
	// amount of frames counted
	unsigned int frames_counted;

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


	// ## performance values ##
	// ticks since last section
	Uint32 perf_last_ticks;

	typedef vector<cPerformance_Timer *> Performance_Timer_List;
	Performance_Timer_List perf_timer;
};

/* Fixed framerate method
 * if next frame is not ready wait until it is
*/
void Correct_Frame_Time( const unsigned int fps );
// Return true if the next frame is ready for the given framerate
bool Is_Frame_Time( const unsigned int fps );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Framerate class
extern cFramerate *pFramerate;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace SMC

#endif
