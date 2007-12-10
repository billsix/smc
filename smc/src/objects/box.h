/***************************************************************************
                box.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __BOX_H__
#define __BOX_H__

#include "../core/globals.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** *** *** *** cBaseBox *** *** *** *** *** *** *** *** *** */

class cBaseBox : public cImageObjectSprite
{
public:
	cBaseBox( float x = 0, float y = 0 );
	virtual ~cBaseBox( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// load from savegame
	virtual void Load_from_Savegame( cSave_Level_Object *save_object );
	// save to savegame
	virtual cSave_Level_Object *Save_to_Savegame( void );

	/* Set the Animation Type
	 * new_anim_type can be : Bonus, Default or Power
	*/
	void Set_Animation( string new_anim_type );
	// sets the count this object can be activated
	virtual void Set_UseableCount( int count, bool new_startcount = 0 );
	/* Set invisible type
	 * 1 : invisible until activation
	 * 2 : ghost box
	*/
	void Set_Invisible( unsigned int type );

	// activates collision movement
	void Activate_collision( ObjectDirection cdirection );
	// updates the collision movement
	void Update_collision( void );

	// check for collision with objects
	void Check_Collision( ObjectDirection cdirection );
	// collision with the given enemy
	void Col_Enemy( cSprite *obj );

	// activate
	virtual void Activate( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// Generate activation Particles
	void Generate_active_Particles( void );

	// if update is valid for the current state
	virtual bool is_Update_valid( void );
	// if draw is valid for the current state and position
	virtual bool is_Draw_valid( void );

	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// handle the basic box player collision
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// handle the basic box enemy collision
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );

	// leveleditor activation
	virtual void Editor_Activate( void );
	bool Editor_Useable_Count_Key( const EventArgs &event ); // editor useable count key up
	bool Editor_Invisible_Select( const EventArgs &event ); // editor invisible option selected

	// animation type
	string anim_type;
	// animation counter
	float counter;
	unsigned int anim_counter_min;
	unsigned int anim_counter_max;
	// box type
	SpriteType box_type;

	// leveleditor item image
	GL_Surface *item_image;

	// moving direction when activated
	ObjectDirection move_col_dir;
	// current moving counter ( if activated )
	float move_counter;
	// if object is moving back to the original position
	bool move_back;
	// times the box object can be activated ( -1 = infinite )
	int useable_count, start_useable_count;

	// box invisible type
	unsigned int box_invisible;

	// active particle animation counter
	float particle_counter_active;

protected:
	// Create the Name from the current settings
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
