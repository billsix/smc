/***************************************************************************
 * global_effect.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2006 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_GLOBAL_EFFECT_H
#define SMC_GLOBAL_EFFECT_H

#include "../core/globals.h"
#include "../core/math/rect.h"
#include "../video/animation.h"

/* *** *** *** *** *** *** *** Global Effect Type *** *** *** *** *** *** *** *** *** *** */

enum  GlobalEffectType
{
	GL_EFF_NONE = 0,	// disabled
	GL_EFF_FALLING = 1
};

/* *** *** *** *** *** *** *** Global effect class *** *** *** *** *** *** *** *** *** *** */

class cGlobal_effect : public cParticle_Emitter
{
public:
	// default constructor
	cGlobal_effect( void );
	// create from stream
	cGlobal_effect( CEGUI::XMLAttributes &attributes );
	// destructor
	virtual ~cGlobal_effect( void );

	// initialise
	virtual void Init_Anim( void );
	// clear
	virtual void Clear( void );

	// create from stream
	virtual void Create_from_Stream( CEGUI::XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// update
	virtual void Update( void );
	// find out of camera particles and move them to the opposite screen side
	void Update_Particles( void );
	// draw
	virtual void Draw( void );

	// Sets global effect type
	void Set_Type( GlobalEffectType ntype );
	void Set_Type( string ntype );
	// Set image
	void Set_Image( string nimg_file );

	// Return the type name
	string Get_Type_Name( void );

	// type
	GlobalEffectType ge_type;

	// image filename
	string image_filename;

	// valid global effect
	bool valid;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
