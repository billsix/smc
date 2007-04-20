/***************************************************************************
				spinbox.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __SPINBOX_H__
#define __SPINBOX_H__

#include "../core/globals.h"
#include "../objects/box.h"

/* *** *** *** *** *** *** *** *** cSpinBox *** *** *** *** *** *** *** *** *** */

class cSpinBox : public cBaseBox
{
public:
	// constructor
	cSpinBox( float x, float y );
	// create from stream
	cSpinBox( XMLAttributes &attributes );
	// destructor
	virtual ~cSpinBox( void );

	// init defaults
	void Init( void );

	// copy
	virtual cSpinBox *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// Activate the Spinning
	virtual void Activate( void );
	// Stop the Spinning
	void Stop( void );
	// update
	virtual void Update( void );

	// ignore onground check
	virtual void Check_onGround( void ) {};

	// spin counter
	float spin_counter;

	// if spinning
	bool spin;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
