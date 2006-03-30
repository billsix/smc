/***************************************************************************
                obj_manager.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :  (C) 2005 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 


#ifndef __OBJ_MANAGER_H__
#define __OBJ_MANAGER_H__

#include "globals.h"
#include "objects/sprite.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

typedef vector<cSprite *> ObjectList;

class cObjectManager
{
public:	
	cObjectManager( void );
	~cObjectManager( void );

	// Returns the sprite
	cSprite *Get_Pointer( unsigned int identifier );

	// Returns the copied sprite
	cSprite *Copy( unsigned int identifier );

	// Updates the destroy handler
	void Update( void );

	/* Returns the current size
	 */
	unsigned int Get_Size( void );
	unsigned int Get_Size_array( unsigned int type );

	/*	Adds a sprite Object
	 */
	void Add( cSprite *sprite );

	cSprite *operator [] ( unsigned int identifier )
	{
		return Get_Pointer( identifier );
	}

	/* Deletes the given item
	 */
	void Delete( unsigned int identifier, bool delete_data = 1 );

	/* Deletes all objects
	 * if delayed is set deletion will occur on the next update
	 */
	void Delete_Objects( bool delayed = 0 );

	/* Deletes all items completely.
	 * if delayed is set deletion will occur on the next update
	 */
	void Delete_All( bool delayed = 0 );

	// The ObjectItems Array
	ObjectList items;
};

#endif
