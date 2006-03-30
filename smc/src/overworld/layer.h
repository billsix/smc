/***************************************************************************
                layer.h  -  header for the corresponding cpp file
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

#ifndef __OVERWORLD_LAYER_H__
#define __OVERWORLD_LAYER_H__

#include "../globals.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Overworld Layer Waypoint class
class cLayer_Waypoint
{
public:
	double x, y;
	unsigned int type; // 0 = default level waypoint, 1 = line connection
};

// Overworld Layer Line class
class cLayer_Line : public GL_line
{
public:
	cLayer_Line( void );
	~cLayer_Line( void );

	// Draw the line
	void Draw( void );

	bool collision_test; // collision testing enabled
	unsigned int type; // 0 = normal walking, 1 = swimming
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Layer Line Collision data
class cLine_collision
{
public:
	void clear( void );

	int line_number; // nearest line number
	double difference; // position difference
};

// Layer Nearest Line Collision data
class cNearLine_collision
{
public:
	void clear( void );

	int line_number; // nearest line number
	bool start; // line start collision else end collision
};

// Layer Contact Collision data
class cContact_collision
{
public:
	// clears data
	void clear( void );
	// returns the best fitting line for the given direction
	int Get_best_line( ObjectDirection dir );

	bool contact; // is there a contact
	cLine_collision line_hor; // horizontal line contact
	cLine_collision line_ver; // vertical line contact
};

typedef vector<cLayer_Waypoint *> LayerWaypointList;
typedef vector<cLayer_Line *> LayerLineList;

// Layer class
// handles the line collision detection
class cLayer : public XMLHandler
{
public:
	cLayer( void );
	virtual ~cLayer( void );

	// loads the Layer settings
	void Load( string filename );
	// Draws the lines and waypoints
	void Draw( void );

	// returns the data between the nearest line and the given position
	cLine_collision Get_diff( double x, double y, ObjectDirection directions = DIR_HORIZONTAL, unsigned int check_size = 15 );

	LayerWaypointList waypoints;
	LayerLineList lines;

	// draws the layer
	bool draw;

private:
	// XML element start
    virtual void elementStart( const String &element, const XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const String &element );
	// handles a world
    void handle_line( const XMLAttributes& attributes );
};


#endif
