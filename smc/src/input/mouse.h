/***************************************************************************
			mouse.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "../core/globals.h"
#include "../objects/movingsprite.h"
#include "../core/math/rect.h"
#include "../core/math/vector.h"

/* *** *** *** *** *** *** cSelectedObject *** *** *** *** *** *** *** *** *** *** *** */

class cSelectedObject
{
public:
	cSelectedObject( void );
	~cSelectedObject( void );
	
	// base object
	cSprite *obj;
	// additional selection mouse position
	int mouse_w, mouse_h;

	// if user controlled
	bool user;
};

typedef vector<cSelectedObject *> SelectedObjectList;

/* *** *** *** *** *** *** cCopyObject *** *** *** *** *** *** *** *** *** *** *** */

class cCopyObject
{
public:
	cCopyObject( void );
	~cCopyObject( void );
	
	// base object
	cSprite *obj;
};

typedef vector<cCopyObject *> CopyObjectList;

/* *** *** *** *** *** *** cMouseCursor *** *** *** *** *** *** *** *** *** *** *** */

class cMouseCursor : public cMovingSprite
{
public:
	cMouseCursor( void );
	virtual ~cMouseCursor( void );

	// resets leveleditor features
	void Reset( bool clear_copy_buffer = 1 );
	// only reset buttons
	void Reset_keys( void );

	// Handles the Mouse events
	bool Handle_Event( SDL_Event *ev );
	/* Function: Gui_handle_Mouse_down
	 * GUI handle mouse down function
	 * Returns true if GUI handled the event
	*/
	bool Handle_Mouse_down( Uint8 button );
	/* Function: Gui_handle_Mouse_up
	 * GUI handle mouse up function
	 * Returns true if GUI handled the event
	*/
	bool Handle_Mouse_up( Uint8 button );

	// Checks if the Mousecursor is in the given rect
	bool Collsion_Check ( GL_rect *crect );

	// Checks for a collision for the editor
	bool Editor_Collsion_Check( float px = 0, float py = 0 );
	/* Get the Level/World Object Collision if available
	 * returns 1 if collision found else 0
	*/
	bool Get_Collision_Level( GL_rect *mouse_rect );
	bool Get_Collision_World( GL_rect *mouse_rect );

	// Update
	virtual void Update( void );
	// Draw
	virtual void Draw( void );
	// Render
	void Render( void );

	// Update the Mouse Position
	void Update_Position( void );
	// Update the Double Click event
	void Update_Doubleclick( void );


	// Editor default click
	void Click( void );
	// Editor default float click
	void Double_Click( bool activate = 1 );

	// ## Mouse Object
	// Set new mouse object
	void Set_mouse_object( cSprite *sprite );
	// update the mouse object position
	void Update_mouse_object( void );
	// clear mouse object
	void Clear_mouse_object( void );

	// ## Copy Objects
	// Add new copy object
	void Add_copy_object( cSprite *sprite );
	void Add_copy_objects( SpriteList spritelist );
	/* Remove a copy_object
	 * returns true if removed
	*/
	bool Remove_copy_object( cSprite *sprite );
	// clear copy object list
	void Clear_copy_objects( void );
	/* returns the position of the first top left copy object
	 * if no copy_objects returns 0, 0
	*/
	GL_Vector Get_copy_object_base( float px, float py );
	// paste copy object list
	void Paste_copy_objects( float px, float py );

	// ## Selected Objects
	/* Add new selected_object
	 * Returns true if object got added or changed to user object
	*/
	bool Add_selected_object( cSprite *sprite, bool from_user = 0 );
	void Add_selected_objects( SpriteList spritelist, bool from_user = 0 );
	/* Remove a selected_object
	 * returns true if found and removed
	*/
	bool Remove_selected_object( cSprite *sprite, bool no_user = 0 );
	// Returns all selected objects
	SpriteList Get_selected_objects( void );
	// clear selected object list
	void Clear_selected_objects( void );
	// updates the selected objects positions
	void Update_selected_objects( void );
	/* returns true if the given object is a selected_object
	 * if only_user is given only check user objects
	*/
	bool is_selected_object( cSprite *sprite, bool only_user = 0 );
	// deletes the selected objects
	void Delete_selected_objects( void );
	// returns the selected objects size
	unsigned int Get_selected_object_size( void );

	// ## Active Object
	// Set new active object
	void Set_active_object( cSprite *sprite );
	// clear active object
	void Clear_active_object( void );

	/* Copies the given object to the given position
	 * object is automatically added to the object manager
	 * returns the new object
	*/
	cSprite *Copy( cSprite *ncopy_object, float px, float py );
	// Deletes the given Object
	void Delete( cSprite *sprite );

	// Draws a rect around the mouse_object and selected objects
	void Draw_object_rects( void );

	// Start selection mode
	void Start_selection( void );
	// End selection mode
	void End_selection( void );
	// update selection mode
	void Update_selection( void );

	// Toggle Mover mode
	void Toggle_Mover_mode( void );
	// Updates the Mover Mode
	void Mover_Update( Sint16 move_x, Sint16 move_y );
	// Updates the Leveleditor Mouse
	void Editor_Update( void );
	
	// current internal mouse position
	int x, y;
	
	// selection mode
	bool mode_selection;
	// Selection rect
	GL_rect selection_rect;

	// if activated the mouse cursor movement moves the screen	
	bool mover_mode;
	// fastcopy mode
	bool mode_fastcopy;

	/* selected objects
	 * the mouse object is also always a selected object
	*/
	SelectedObjectList selected_objects;
	// currently colliding object with the mouse
	cSelectedObject *mouse_object;
	// objects selected for copying
	CopyObjectList copy_objects;
	// settings activated object
	cSprite *active_object;

	// pressed Buttons
	bool left, right, middle;

	// last clicked object
	cSprite *click_temp;

	// counter for catching float-clicks
	float clickcounter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Mouse
extern cMouseCursor *pMouseCursor;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
