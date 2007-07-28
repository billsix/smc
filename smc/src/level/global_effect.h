/***************************************************************************
                global_effect.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __GLOBAL_EFFECT_H__
#define __GLOBAL_EFFECT_H__

#include "../core/globals.h"
#include "../core/math/rect.h"
#include "../video/animation.h"

/* *** *** *** *** *** *** *** Global Effect Type *** *** *** *** *** *** *** *** *** *** */

enum  GlobalEffectType
{
	GL_EFF_NONE = 0,	// no global effect
	GL_EFF_FALLING = 1,	// falling down images ( leafs or rain )
	GL_EFF_FLYING = 2	// moving upwards images ( heat points or dust )
};

/* *** *** *** *** *** *** *** Global effect class *** *** *** *** *** *** *** *** *** *** */

class cGlobal_effect : public cParticleAnimation
{
public:
	// default constructor
	cGlobal_effect( void );
	// create from stream
	cGlobal_effect( XMLAttributes &attributes );
	// destructor
	virtual ~cGlobal_effect( void );

	// init defaults
	void Init_default( void );
	// initialise
	void Init( void );
	// clear
	virtual void Clear( void );

	// create from stream
	void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	void Save_to_Stream( ofstream &file );

	// update
	void Update( void );
	// update animation
	void Update_anim( void );
	// draw
	void Draw( void );

	// Sets global effect type
	void Set_type( GlobalEffectType ntype );
	void Set_type( string ntype );
	// Set image
	void Set_image( string nimg_file );

	// Return the type name
	string Get_Typename( void );

	// type
	GlobalEffectType ge_type;

	// image filename
	string image_filename;

	// valid global effect
	bool valid;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
