/***************************************************************************
			gl_surface.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2005 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __GL_SURFACE_H__
#define __GL_SURFACE_H__

#include "../core/globals.h"
#include "../core/math/point.h"

/* *** *** *** *** *** *** *** *** OpenGL Surface *** *** *** *** *** *** *** *** *** */

class GL_Surface
{
public:
	GL_Surface( void );
	~GL_Surface( void );

	/* Blit the surface on the given position
	* if request is NULL automatically creates the request
	*/
	void Blit( float x, float y, float z, cSurfaceRequest *request = NULL );
	/* Blit only the surface data on the given request
	*/
	void Blit_data( cSurfaceRequest *request );

	/* copy this image and returns it
	*/
	GL_Surface *Copy( void );

	/* save this image texture to a file
	*/
	void Save( string filename );

	// Set the ground type
	void Set_Ground_type( GroundType gtype );

	// returns a software texture copy
	cSoftware_texture *Get_Software_Texture( void );
	// loads a software texture
	void Load_Software_Texture( cSoftware_texture *soft_tex );

	// Returns the filename
	string Get_filename( int with_dir = 2, bool with_end = 1 );
	// set a function called on destruction
	void Set_destruction_function( void ( *nfunction )( GL_Surface * ) );

	// GL texture number
	GLuint image;
	// internal drawing offset
	float int_x, int_y;
	// starting drawing dimension without modifications like rotation and scaling
	float start_w, start_h;
	// final drawing dimension
	float w, h;
	// texture dimension
	unsigned int tex_w, tex_h;
	// internal rotation
	float base_rotx, base_roty, base_rotz;
	// image collision data
	GL_point col_pos;
	float col_w, col_h;

	// origin if created from a file
	string filename;
	// should the image be deleted
	bool auto_del_img;
	// if the image is tagged as obsolete 
	bool obsolete;

	// editor tags
	string editor_tags;
	// name
	string name;
	// default sprite type
	unsigned int type;
	// ground type
	GroundType ground_type;
private:
	// function called on destruction
	void ( *destruction_function )( GL_Surface * );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
