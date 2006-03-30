/***************************************************************************
    waypoint.cpp  -  waypoint class for the Overworld
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

/* *** *** *** *** *** *** *** *** Waypoint *** *** *** *** *** *** *** *** *** */

cWaypoint :: cWaypoint( void )
{
	gcolor = rand() % 100;
	glim = 1;
	
	access = 0;
	access_default = 0;
	
	ID = 0;

	rect.x = 0;
	rect.y = 0;
	// default dimension
	rect.w = 20;
	rect.h = 20;

	direction_back = DIR_NOTHING;
	direction_forward = DIR_NOTHING;

	type = WAYPOINT_NORMAL;

	levelname = "";

	arrow_blue_l = GetImage( "world/arrows/blue_left_small.png" );
	arrow_blue_r = GetImage( "world/arrows/blue_right_small.png" );
	arrow_blue_u = GetImage( "world/arrows/blue_up_small.png" );
	arrow_blue_d = GetImage( "world/arrows/blue_down_small.png" );

	arrow_white_l = GetImage( "world/arrows/left_small.png" );
	arrow_white_r = GetImage( "world/arrows/right_small.png" );
	arrow_white_u = GetImage( "world/arrows/up_small.png" );
	arrow_white_d = GetImage( "world/arrows/down_small.png" );

	image = GetImage( "world/Waypoints/default_1.png" );
}

cWaypoint :: ~cWaypoint( void )
{
	//
}

void cWaypoint :: Draw( void )
{
	if( !image || ( !access && !pOverWorld->debugmode ) )
	{
		return;
	}
	
	if( glim )
	{
		gcolor += Framerate.speedfactor * 3;
	}
	else
	{
		gcolor -= Framerate.speedfactor * 3;
	}

	if( gcolor > 120 )
	{
		glim = 0;
	}
	else if( gcolor < 7 )
	{
		glim = 1;
	}

	if( pOverWorld->debugmode )
	{
		double x = rect.x - cameraposx;
		double y = rect.y - cameraposy;
		
		// direction back
		if( direction_back == DIR_RIGHT )
		{
			x += image->w;
			y -= 2;
			arrow_blue_r->Blit( x, y, &lightgreyalpha );
		}
		else if( direction_back == DIR_LEFT )
		{
			x -= arrow_blue_l->w;
			y -= 2;
			arrow_blue_l->Blit( x, y, &lightgreyalpha );
		}
		else if( direction_back == DIR_UP )
		{
			y -= arrow_blue_u->h;
			x += 4;
			arrow_blue_u->Blit( x, y, &lightgreyalpha );
		}
		else if( direction_back == DIR_DOWN )
		{
			y += image->h;
			x += 4;
			arrow_blue_d->Blit( x, y, &lightgreyalpha );
		}

		x = rect.x - cameraposx;
		y = rect.y - cameraposy;

		// direction forward
		if( direction_forward == DIR_RIGHT )
		{
			x += image->w;
			y -= 2;
			arrow_white_r->Blit( x, y, &lightgreyalpha );
		}
		else if( direction_forward == DIR_LEFT )
		{
			x -= arrow_white_l->w;
			y -= 2;
			arrow_white_l->Blit( x, y, &lightgreyalpha );
		}
		else if( direction_forward == DIR_UP )
		{
			y -= arrow_white_u->h;
			x += 4;
			arrow_white_u->Blit( x, y, &lightgreyalpha );
		}
		else if( direction_forward == DIR_DOWN )
		{
			y += image->h;
			x += 4;
			arrow_white_d->Blit( x, y, &lightgreyalpha );
		}
	}

	if( type == 1 || type == 2 || pOverWorld->debugmode )
	{
		if( !pOverWorld->debugmode )
		{
			glColor3ub( 255, 100 + (Uint8)gcolor, 10 );
		}
		else
		{
			if( access )
			{
				glColor3ub( 255, 100 + (Uint8)gcolor, 200 );
			}
			else
			{
				glColor3ub( 20, 100 + (Uint8)gcolor, 10 );
			}
		}

		image->Blit( rect.x - cameraposx, rect.y - cameraposy );

		glColor3ub( 255, 255, 255 );
	}
}

string cWaypoint :: Get_Levelfile( bool with_dir /* = 1 */, bool with_end /* = 1 */ )
{
	string name = levelname;

	if( with_dir && name.find( pPreferences->level_dir ) == string::npos ) 
	{
		name.insert( 0, "/" );
		name.insert( 0, pPreferences->level_dir );
	}

	if( with_end && name.find( ".txt" ) == string::npos ) 
	{
		name.insert( name.length(), ".txt" );
	}
	else if( !with_end && name.find( ".txt" ) != string::npos ) 
	{
		name.erase( name.find( ".txt" ), 4 );
	}

	return name;
}
