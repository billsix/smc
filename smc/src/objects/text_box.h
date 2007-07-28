/***************************************************************************
				text_box.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __TEXT_BOX_H__
#define __TEXT_BOX_H__

#include "../core/globals.h"
#include "../objects/box.h"

/* *** *** *** *** *** *** *** *** cText_Box *** *** *** *** *** *** *** *** *** */

class cText_Box : public cBaseBox
{
public:
	// constructor
	cText_Box( float x, float y );
	// create from stream
	cText_Box( XMLAttributes &attributes );
	// destructor
	virtual ~cText_Box( void );

	// init defaults
	void Init( void );

	// copy
	virtual cText_Box *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Activate
	virtual void Activate( void );

	// update
	virtual void Update( void );

	// Set Text
	void Set_Text( string str_text );

	// ignore onground check
	virtual void Check_onGround( void ) {};

	// level editor activation
	virtual void Editor_Activate( void );
	// level editor events
	bool Editor_Text_Key( const EventArgs &event );  // editor text key up

	// the text
	string text;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
