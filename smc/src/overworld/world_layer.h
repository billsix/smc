/***************************************************************************
 * layer.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2003 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_WORLD_LAYER_H
#define SMC_WORLD_LAYER_H

#include "../core/globals.h"
#include "../objects/movingsprite.h"
#include "../core/obj_manager.h"
#include "../overworld/world_waypoint.h"


/* *** *** *** *** *** *** Layer_Line *** *** *** *** *** *** *** *** *** *** *** */

// Layer Line Point class
// TODO : create a real basic/virtual sprite ?
class cLayer_Line_Point : public cSprite
{
public:
	cLayer_Line_Point( SpriteType ntype );
	virtual ~cLayer_Line_Point( void );

	// copy this sprite
	virtual cLayer_Line_Point *Copy( void ) { return NULL; };

	// save to stream
	virtual void Save_to_Stream( ofstream &file ) {};

	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// editor activation
	virtual void Editor_Activate( void );
	// editor events
	bool Editor_Origin_Key( const CEGUI::EventArgs &event ); // editor origin key up

	float Get_Line_Pos_X( void );
	float Get_Line_Pos_Y( void );

	// line
	void *line;
};

// Overworld Layer Line class
class cLayer_Line
{
public:
	cLayer_Line( void );
	virtual ~cLayer_Line( void );

	// Draw
	void Draw( void );

	// return a normal line
	GL_line Get_Line( void );

	/* Returns the Waypoint on the end of the line(s)
	 * if the line continues on another line it is followed to the end
	*/
	cWaypoint *Get_End_Waypoint( void );
	
	// start point
	cLayer_Line_Point *start;
	// end point
	cLayer_Line_Point *end;

	/* animation type 
	 * 0 = normal walking, 1 = swimming
	 */
	unsigned int anim_type;
	// waypoint origin identifier
	unsigned int origin;
};

/* *** *** *** *** *** *** *** Layer *** *** *** *** *** *** *** *** *** *** */

// Layer Line Collision data
class cLine_collision
{
public:
	cLine_collision( void );

	// nearest line pointer
	cLayer_Line *line;
	// nearest line number
	int line_number;
	// position difference
	float difference;
};

// Layer Nearest Line Collision data
class cNearLine_collision
{
public:
	cNearLine_collision( void );

	// nearest line number
	int line_number;
	// line start collision else end collision
	bool start;
};

// Layer Contact Collision data
class cContact_collision
{
public:
	// clear data
	void clear( void );
	// return the best fitting line for the given direction
	int Get_Best_Line( ObjectDirection dir );

	// is there a contact
	bool contact;
	// horizontal line contact
	cLine_collision line_hor;
	// vertical line contact
	cLine_collision line_ver;
};

typedef vector<cLayer_Line *> LayerLineList;

// Layer class
// handles the line collision detection
class cLayer : public CEGUI::XMLHandler, public cObject_Manager<cLayer_Line>
{
public:
	cLayer( void );
	virtual ~cLayer( void );

	// Load from file
	void Load( string filename );

	// Save
	bool Save( string filename );

	// Draw the lines and waypoints
	void Draw( void );

	/* Returns the colliding Line start point
	 * if not found returns NULL
	*/
	cLayer_Line *Get_Line_Collision_Start( GL_rect *line_rect );
	/* Returns the colliding Line from the given position and the added direction size
	 * if not found returns a NULL line in the class
	*/
	cLine_collision Get_Line_Collision_Direction( float x, float y, ObjectDirection dir, float dir_size = 15, unsigned int check_size = 10 );

	/* Return the collision data between the nearest line and the given position
	 * check_size is maximum size for both direction checking lines
	 * if only_origin_id is set only checks lines with the given id
	*/
	cLine_collision Get_Nearest( float x, float y, ObjectDirection dir = DIR_HORIZONTAL, unsigned int check_size = 15, int only_origin_id = -1 );
	// Return the collision data between the given line and position
	cLine_collision Get_Nearest_Line( cLayer_Line *map_layer_line, float x, float y, ObjectDirection dir = DIR_HORIZONTAL, unsigned int check_size = 15 );

private:
	// XML element start
    virtual void elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const CEGUI::String &element );

	// handles a line
    void handle_line( const CEGUI::XMLAttributes &attributes );

	// XML element Property list
	CEGUI::XMLAttributes xml_attributes;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
