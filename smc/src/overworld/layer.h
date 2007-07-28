/***************************************************************************
                layer.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __OVERWORLD_LAYER_H__
#define __OVERWORLD_LAYER_H__

#include "../core/globals.h"
#include "../objects/movingsprite.h"

/* *** *** *** *** *** *** Layer_Line *** *** *** *** *** *** *** *** *** *** *** */

// Layer Line Point class
// TODO : create a real basic sprite ?
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
	bool Editor_Origin_Key( const EventArgs &event ); // editor origin key up

	float Get_Line_posx( void );
	float Get_Line_posy( void );

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
	void clear( void );

	// nearest line number
	int line_number;
	// position difference
	float difference;
};

// Layer Nearest Line Collision data
class cNearLine_collision
{
public:
	void clear( void );

	// nearest line number
	int line_number;
	// line start collision else end collision
	bool start;
};

// Layer Contact Collision data
class cContact_collision
{
public:
	// clears data
	void clear( void );
	// returns the best fitting line for the given direction
	int Get_best_line( ObjectDirection dir );

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
class cLayer : public XMLHandler
{
public:
	cLayer( void );
	virtual ~cLayer( void );

	// Load from file
	void Load( string filename );
	// Unload
	void Unload( void );
	// Save
	bool Save( string filename );

	// Draw the lines and waypoints
	void Draw( void );

	// Return the data between the nearest line and the given position
	cLine_collision Get_diff( float x, float y, ObjectDirection directions = DIR_HORIZONTAL, unsigned int check_size = 15 );

	/* Returns the line array number
	 * if not found returns -1
	*/
	int Get_Line_num( cLayer_Line *obj );

	// lines
	LayerLineList lines;

	// draw the layer
	bool draw;

private:
	// XML element start
    virtual void elementStart( const String &element, const XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const String &element );

	// handles a world
    void handle_line( const XMLAttributes &attributes );

	// XML element Property list
	XMLAttributes xml_attributes;
};


#endif
