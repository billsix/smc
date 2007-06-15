/***************************************************************************
                obj_manager.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :  (C) 2005 - 2007 by Florian Richter
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

#include "../core/globals.h"
#include "../objects/movingsprite.h"

/* *** *** *** *** *** cObjectManager *** *** *** *** *** *** *** *** *** *** *** *** */

class cObjectManager
{
public:	
	cObjectManager( unsigned int reserve_items = 2000, unsigned int zpos_items = 100 );
	~cObjectManager( void );

	/*	Adds a sprite Object
	 */
	void Add( cSprite *sprite );

	// Returns the copied sprite
	cSprite *Copy( unsigned int identifier );

	/* Deletes the given item
	 */
	void Delete( unsigned int array_num, bool delete_data = 1 );
	bool Delete( cSprite *obj, bool delete_data = 1 );

	/* Deletes all objects
	 * if delayed is set deletion will occur on the next update
	 */
	void Delete_Objects( bool delayed = 0 );

	/* Deletes all items completely.
	 * if delayed is set deletion will occur on the next update
	 */
	void Delete_All( bool delayed = 0 );

	// returns the object pointer if found
	cSprite *Get_Pointer( unsigned int identifier );
	// returns the first Z positioned object from the given type
	cSprite *Get_First( SpriteType type );
	// returns the last Z positioned object from the given type
	cSprite *Get_Last( SpriteType type );
	/* returns the matching object from the given startposition
	 * type : if not set to zero only returns the object with the given type
	*/
	cSprite *Get_from_Position( int posx, int posy, SpriteType type = TYPE_UNDEFINED );

	/* Get a sorted Objects Array
	 * editor_sort : if set sorts from editor zpos
	*/
	void Get_Objects_sorted( SpriteList &objects, bool editor_sort = 0, bool with_player = 0 );

	/* Returns the object array number
	 * if not found returns -1
	*/
	int Get_Array_num( cSprite *obj );

	// Update items drawing validation
	void Update_items_valid_draw( void );
	// Update items
	void Update_items( void );
	// Draw items
	void Draw_items( void );
	// Create Collision data and Handle the collisions
	void Handle_Collision_items( void );

	// Updates the destroy handler
	void Update( void );

	/* Returns the current size
	 */
	unsigned int size( void );
	/* Returns the current size
	 * of the specified sprite array
	 */
	unsigned int Get_Size_array( ArrayType sprite_array );

	// Switch objects array position
	void Switch( cSprite *obj1, cSprite *obj2 );

	// returns a object pointer if found
	cSprite *operator [] ( unsigned int identifier )
	{
		return Get_Pointer( identifier );
	}

	// Set the sprite Z position ( updates zpos list )
	void Set_Z( cSprite *sprite );

	// objects array
	SpriteList items;

	typedef vector<float> ZposList;
	// biggest type z position
	ZposList zposdata;
	// biggest editor type z position
	ZposList zposdata_editor;

	// Z position sort
	struct zpos_sort
	{
		bool operator()( const cSprite *a, const cSprite *b ) const
        {
            return a->posz > b->posz;
        }
	};

	// Editor Z position sort
	struct editor_zpos_sort
	{
		bool operator()( const cSprite *a, const cSprite *b ) const
        {
			// check if the editor zpos is available
			if( !a->editor_posz )
			{
				if( !b->editor_posz )
				{
					return a->posz < b->posz;
				}

				return a->posz < b->editor_posz;
			}
			if( !b->editor_posz )
			{
				if( !a->editor_posz )
				{
					return a->posz < b->posz;
				}

				return a->editor_posz < b->posz;
			}

			// both objects have an editor z pos
            return a->editor_posz < b->editor_posz;
        }
	};
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Sprite Manager
extern cObjectManager *pObjManager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
