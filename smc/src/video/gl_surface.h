/***************************************************************************
 * gl_surface.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2005 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_GL_SURFACE_H
#define SMC_GL_SURFACE_H

#include "../core/globals.h"
#include "../core/math/point.h"

/* *** *** *** *** *** *** *** *** OpenGL Surface *** *** *** *** *** *** *** *** *** */

class cGL_Surface
{
public:
	cGL_Surface( void );
	~cGL_Surface( void );

	/* Blit the surface on the given position
	 * if request is NULL automatically creates the request
	*/
	void Blit( float x, float y, float z, cSurfaceRequest *request = NULL );
	// Blit only the surface data on the given request
	void Blit_Data( cSurfaceRequest *request );

	// Copy cGL_Surface and return it
	cGL_Surface *Copy( void );

	// Save the texture to a file
	void Save( string filename );

	// Set the ground type
	void Set_Ground_Type( GroundType gtype );

	// Check if the OpenGL texture is used by another cGL_Surface
	bool Is_Texture_Use_Multiple( void );

	/* Return a software texture copy
	 * only_filename: if set doesn't save the software texture but only the filename
	*/
	cSaved_Texture *Get_Software_Texture( bool only_filename = 0 );
	// Load a software texture
	void Load_Software_Texture( cSaved_Texture *soft_tex );

	// Return the filename
	string Get_Filename( int with_dir = 2, bool with_end = 1 );
	// Set a function called on destruction
	void Set_Destruction_Function( void ( *nfunction )( cGL_Surface * ) );

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
	// if managed over the image manager
	bool managed;
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
	void ( *destruction_function )( cGL_Surface * );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
