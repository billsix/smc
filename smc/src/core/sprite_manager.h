/***************************************************************************
 * sprite_manager.h  -  header for the corresponding cpp file
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

#ifndef SMC_SPRITE_MANAGER_H
#define SMC_SPRITE_MANAGER_H

#include "../core/globals.h"
#include "../core/obj_manager.h"
#include "../objects/movingsprite.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** cSprite_Manager *** *** *** *** *** *** *** *** *** *** *** *** */

class cSprite_Manager : public cObject_Manager<cSprite>
{
public:
	cSprite_Manager( unsigned int reserve_items = 2000, unsigned int zpos_items = 100 );
	virtual ~cSprite_Manager( void );

	/* Add a sprite
	 */
	virtual void Add( cSprite *sprite );

	// Return the copied sprite
	cSprite *Copy( unsigned int identifier );

	/* Delete all objects
	 * if delayed is set deletion will only occur if replaced
	 */
	virtual void Delete_All( bool delayed = 0 );

	// Return the first Z positioned object from the given type
	cSprite *Get_First( SpriteType type );
	// Return the last Z positioned object from the given type
	cSprite *Get_Last( SpriteType type );
	/* Return the matching object from the given startposition
	 * type : if not set to zero only returns the object with the given type
	*/
	cSprite *Get_from_Position( int posx, int posy, SpriteType type = TYPE_UNDEFINED );

	/* Get a sorted Objects Array
	 * editor_sort : if set sorts from editor zpos
	*/
	void Get_Objects_sorted( SpriteList &new_objects, bool editor_sort = 0, bool with_player = 0 );

	// Update items drawing validation
	void Update_Items_Valid_Draw( void );
	// Update items
	void Update_Items( void );
	// Draw items
	void Draw_Items( void );
	// Create Collision data and Handle the collisions
	void Handle_Collision_Items( void );


	/* Return the current size
	 * of the specified sprite array
	 */
	unsigned int Get_Size_Array( ArrayType sprite_array );

	// Return object pointer if found
	cSprite *operator [] ( unsigned int identifier )
	{
		return Get_Pointer( identifier );
	}

	// Set the sprite Z position ( updates zpos list )
	void Set_Z( cSprite *sprite );

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

#endif
