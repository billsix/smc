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

/* *** *** *** *** *** image_item *** *** *** *** *** *** *** *** *** *** *** *** */
 
// image_item Structure
struct image_item
{
	GL_Surface *item;
	unsigned int count_id;
};

typedef vector<image_item> ImageList;

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

/* *** *** *** *** *** *** cImageManager *** *** *** *** *** *** *** *** *** *** *** */

//  Keeps track of all the images in memory
//
// Operators:
//  - cImageManager [path]
//  - cImageManager [ID]
class cImageManager
{
public:
	cImageManager( void );
	~cImageManager( void );

	// Function: Get_Pointer
	// Returns the Surface by Path
	GL_Surface *Get_Pointer( string path );

	// Function: Get_Pointer
	// Returns the Surface by ID
	GL_Surface *Get_Pointer( unsigned int identifier );

	// Function: Get_Pointer_array
	// Returns an array by ID
	GL_Surface *Get_Pointer_array( unsigned int identifier );

	// Function: Copy
	// Returns the copied image
	GL_Surface *Copy( string path );

	// Function: Get_Path
	// Returns the image Path in a string
	string Get_Path( unsigned int identifier );

	/*	Function: Get_Size
	 *	Returns the Current size
	 */
	unsigned int Get_Size( void );

	/* Function: Add
	 *  Adds an Image
	 *	Should always have a Path
	 */
	void Add( GL_Surface *item );

	GL_Surface *operator [] ( unsigned int identifier )
	{
		return Get_Pointer( identifier );
	}
	
	GL_Surface *operator [] ( string path )
	{
		return Get_Pointer( path );
	}

	/*
	* Function: Grab_Textures
	* saves hardware textures in software memory
	*/
	void Grab_Textures( void );

	/*
	* Function: Reload_Textures
	* Loads the saved software textures back into hardware textures
	*/
	void Restore_Textures( void );

	/*
	* Function: Delete_Images
	* Frees all bitmap surfaces, but keep Items vector entries
	*/
	void Delete_Images( void );

	/*
	* Function: Delete_Hardware_Textures
	* Frees all hardware surfaces
	*/
	void Delete_Hardware_Textures( void );

	/*
	* Function: Delete_All
	* Deletes all ImageItems completely.
	*/
	void Delete_All( void );

	// highest opengl texture id found
	GLuint high_texture_id;

private:
	// images loaded since initialization
	unsigned int load_count;
	// image items array
	ImageList items;
	// saved software textures for reloading
	SoftwareTextureList software_textures;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Image Manager
extern cImageManager *pImageManager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
