/***************************************************************************
			enemystopper.h  -  header for the corresponding cpp file
                         -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __ENEMYSTOPPER_H__
#define __ENEMYSTOPPER_H__

#include "../core/globals.h"
#include "../objects/objectsprite.h"

/* *** *** *** *** *** cEnemyStopper *** *** *** *** *** *** *** *** *** *** *** *** */

class cEnemyStopper : public cImageObjectSprite
{
public:
	// constructor
	cEnemyStopper( float x, float y );
	// create from stream
	cEnemyStopper( XMLAttributes &attributes );
	// destructor
	virtual ~cEnemyStopper( void );
	
	// init defaults
	void Init( void );
	// copy
	virtual cEnemyStopper *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// if draw is valid for the current state and position
	virtual bool is_Draw_valid( void );

	// editor color
	Color editor_color;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
