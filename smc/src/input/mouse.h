/***************************************************************************
			mouse.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../globals.h"

#ifndef __MOUSE_H__
#define __MOUSE_H__

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cMouseCursor : public cSprite
{
public:
	cMouseCursor( void );
	virtual ~cMouseCursor( void );

	// resets leveleditor features
	void Reset( void );

	// Handles the Mouse events
	void Handle_Events( void );

	// Checks if the Mousecursor is in the Rect
	bool CollsionCheck ( GL_rect *rect );

	// Checks for a collision for the leveleditor
	bool Editor_CollsionCheck ( double x, double y );

	// Updates the Mousecursor
	virtual void Update( void );
	// Updates the Mouse Position
	void Update_Position( void );
	// Updates the Double Click event
	void Update_Doubleclick( void );
	// Updates the Mouseobject position
	void MouseObject_Update( void );

	// Editor default click
	void Click( void );
	// Editor default double click
	void Double_Click( bool activate = 1 );

	// Sets the MouseObject
	void Set_MouseObject( cSprite *sprite );
	// Copies the Object
	void Copy( cSprite *nCopyObject, double x, double y );
	// Deletes the current colliding Object
	void Delete( void );

	// Draws lines around the currently hoverd Object
	void Draw_HoveredObject( void );

	// Updates the Mover Mode
	void Mover_Update( Sint16 move_x, Sint16 move_y );
	// Updates the Leveleditor Mouse
	void Editor_Update( void );
	
	// current internal mouseposition
	int x, y;

	// image if in mover mode
	GL_Surface *Mover_mouse;
	
	// Hovered object Rect
	GL_rect HoveredObject;
	
	// if activated the mouscursor movement moves the screen	
	bool mover_mode;
	// fastcopy mode
	bool fastCopyMode;

	// current colliding Object
	cSprite *MouseObject;
	// Object selected for copying
	cSprite *CopyObject;
	// settings activated object
	cSprite *ActiveObject;

	// additional MouseObject position
	int mouse_w, mouse_h;

	// pressed Buttons
	bool left, right, middle;

	// last clicked object
	cSprite *click_temp;

	// counter for catching double-clicks
	double clickcounter;
};

#endif
