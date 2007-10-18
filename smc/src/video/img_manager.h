/***************************************************************************
                img_manager.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :  (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __IMG_MANAGER_H__
#define __IMG_MANAGER_H__

#include "../core/globals.h"
#include "../video/video.h"
#include "../core/obj_manager.h"

/* *** *** *** *** *** cSoftware_texture *** *** *** *** *** *** *** *** *** *** *** *** */

// software_texture data
class cSoftware_texture
{
public:
	cSoftware_texture( void );
	~cSoftware_texture( void );

	// base surface
	GL_Surface *base;

    uint8 *pixels;

    GLint width;
    GLint height;

    GLint min_filter;
    GLint mag_filter;
    GLint wrap_s;
    GLint wrap_t;
};

typedef vector<cSoftware_texture *> SoftwareTextureList;
typedef vector<GL_Surface *> SurfaceList;

/* *** *** *** *** *** *** cImageManager *** *** *** *** *** *** *** *** *** *** *** */

//  Keeps track of all the images in memory
//
// Operators:
//  - cImageManager [path]
//  - cImageManager [ID]
class cImageManager : public cObject_Manager<GL_Surface>
{
public:
	cImageManager( void );
	virtual ~cImageManager( void );

	// Return the Surface by Path
	GL_Surface *Get_Pointer( string path );

	// Return the copied image
	GL_Surface *Copy( string path );

	GL_Surface *operator [] ( unsigned int identifier )
	{
		return cObject_Manager<GL_Surface>::Get_Pointer( identifier );
	}
	
	GL_Surface *operator [] ( string path )
	{
		return Get_Pointer( path );
	}

	/* Save hardware textures in software memory
	*/
	void Grab_Textures( void );

	/* Load the saved software textures back into hardware textures
	*/
	void Restore_Textures( void );

	/* Delete all surface textures, but keep object vector entries
	*/
	void Delete_Image_Textures( void );

	/* Delete all hardware surfaces
	*/
	void Delete_Hardware_Textures( void );

	/* Delete all Surfaces
	*/
	virtual void Delete_All( void );

	// highest opengl texture id found
	GLuint high_texture_id;

private:
	// saved software textures for reloading
	SoftwareTextureList software_textures;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Image Manager
extern cImageManager *pImageManager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
