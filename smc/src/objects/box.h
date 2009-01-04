/***************************************************************************
 * box.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2003 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_BOX_H
#define SMC_BOX_H

#include "../core/globals.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** *** *** *** Box invisible types *** *** *** *** *** *** *** *** *** */

enum Box_Invisible_Type
{
	// always visible
    BOX_VISIBLE = 0,
	// visible after activation
    BOX_INVISIBLE_MASSIVE = 1,
	// only visible in ghost mode
    BOX_GHOST = 2,
	// visible after activation and only touchable in the activation direction
    BOX_INVISIBLE_SEMI_MASSIVE = 3
};

/* *** *** *** *** *** *** *** *** cBaseBox *** *** *** *** *** *** *** *** *** */

class cBaseBox : public cImageObjectSprite
{
public:
	cBaseBox( float x = 0, float y = 0 );
	virtual ~cBaseBox( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
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
	virtual void Set_Useable_Count( int count, bool new_startcount = 0 );
	// Set invisible type
	void Set_Invisible( Box_Invisible_Type type );

	// activates collision movement
	void Activate_Collision( ObjectDirection cdirection );
	// updates the collision movement
	void Update_Collision( void );

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
	void Generate_Activation_Particles( void );

	// if update is valid for the current state
	virtual bool Is_Update_Valid( void );
	// if draw is valid for the current state and position
	virtual bool Is_Draw_Valid( void );

	/* Validate the given collision object
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
	virtual unsigned int Validate_Collision( cSprite *obj );
	// handle the basic box player collision
	virtual void Handle_Collision_Player( cObjectCollision *collision );
	// handle the basic box enemy collision
	virtual void Handle_Collision_Enemy( cObjectCollision *collision );

	// editor activation
	virtual void Editor_Activate( void );
	// editor useable count key up event
	bool Editor_Useable_Count_Key( const CEGUI::EventArgs &event );
	// editor invisible option selected event
	bool Editor_Invisible_Select( const CEGUI::EventArgs &event );

	// animation type
	string anim_type;
	// animation counter
	float counter;
	unsigned int anim_counter_min;
	unsigned int anim_counter_max;
	// box type
	SpriteType box_type;

	// leveleditor item image
	cGL_Surface *item_image;

	// moving direction when activated
	ObjectDirection move_col_dir;
	// current moving counter ( if activated )
	float move_counter;
	// if object is moving back to the original position
	bool move_back;
	// times the box object can be activated ( -1 = infinite )
	int useable_count, start_useable_count;

	// box invisible type
	Box_Invisible_Type box_invisible;

	// active particle animation counter
	float particle_counter_active;

protected:
	// Create the Name from the current settings
    void Create_Name( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
