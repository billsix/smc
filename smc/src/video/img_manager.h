/***************************************************************************
                img_manager.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :  (C) 2003- 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 


#ifndef __IMG_MANAGER_H__
#define __IMG_MANAGER_H__

#include "../globals.h"
#include "video.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Struct: ImageItem
// The Imageitem Structure
struct ImageItem 
{
	GL_Surface *item;
	unsigned int count_id;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

typedef vector<ImageItem> ImageList;

// Class: cImageManager
//  Keeps track of all the images (surfaces in memory).
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

	/**
	* Call this function to reload all images.
	*
	* step: 0 = complete reload
	*       1 = only delete bitmap surfaces
	*       2 = only load the bitmap surfaces
	*/
	//void ReloadImages( unsigned int step = 0 );

	/**
	* Function: DeleteImages
	* Frees all bitmap surfaces, but keep ImageItems vector entries
	*/
	void DeleteImages( void );

	/**
	* Function: DeleteAll
	* Deletes all ImageItems completely.
	*/
	void DeleteAll( void );

private:
	// The images loaded since initialization
	unsigned int load_count;
	// The ImageItems Array
	ImageList items;
};

#endif
