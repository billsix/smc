/***************************************************************************
                box.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003-2004 by Artur Hallmann
							(C) 2003-2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __BOX_H__
#define __BOX_H__

#include "../globals.h"

/* *** *** *** *** *** *** *** *** cBaseBox *** *** *** *** *** *** *** *** *** */

class cBaseBox : public cActiveSprite
{
public:
	cBaseBox( double x, double y, SpriteType boxtype );
	virtual ~cBaseBox( void );

	// sets the count this object can be activated
	void Set_UseableCount( int count, bool new_startcount = 0 );

	// activates collision movement
	void Activate_collision( ObjectDirection cdirection );
	// updates the colision movement
	void Update_collision( void );

	// updates the BaseBox
	virtual void Update( void );
	// draws the BaseBox
	virtual void Draw( void );

	// handle the basic box player collision
	virtual void PlayerCollision( ObjectDirection direction );
	// handle the basic box enemy collision
	virtual void EnemyCollision( ObjectDirection direction );

	// moving direction when activated
	ObjectDirection move_col_dir;
	// if object is moving back to the original position
	bool move_back;
	// times the box object can be activated ( -1 = infinite )
	int useable_count, start_useable_count;
	// leveleditor item image
	GL_Surface *item_image;
};

/* *** *** *** *** *** *** *** *** cGoldBox *** *** *** *** *** *** *** *** *** */

class cGoldBox : public cBaseBox
{
public:
	cGoldBox( double x, double y, DefaultColor goldcolor = COL_YELLOW );
	virtual ~cGoldBox( void );
	
	// activates the goldpiece
	void Activate( void );

	virtual void Update( void );

	virtual void PlayerCollision( ObjectDirection direction );
	virtual void EnemyCollision( ObjectDirection direction );

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor deactivation
	virtual void Editor_Deactivate( void );
	// leveleditor position update
	virtual void Editor_pos_update( void );
	// leveleditor value update
	virtual void Editor_val_update( void );

	// bouncing goldpiece
	typedef vector<cBouncingGoldPiece *> BouncingGoldpieceList;
	BouncingGoldpieceList goldpieces;
	// gold color
	DefaultColor color;
};

/* *** *** *** *** *** *** *** *** cBonusBox *** *** *** *** *** *** *** *** *** */

class cBonusBox : public cBaseBox
{
public:
	cBonusBox( double x, double y, SpriteType stype = TYPE_BONUSBOX_MUSHROOM_FIRE );
	virtual ~cBonusBox( void );

	// activates the bonus
	void Activate( void );

	virtual void Update( void );

	virtual void PlayerCollision( ObjectDirection direction );
	virtual void EnemyCollision( ObjectDirection direction );

	double counter;
	cPowerUp *powerup;
};

/* *** *** *** *** *** *** *** *** cSpinBox *** *** *** *** *** *** *** *** *** */

class cSpinBox : public cBaseBox
{
public:
	cSpinBox( double x, double y );
	virtual ~cSpinBox( void );

	virtual void Update( void );

	virtual void PlayerCollision( ObjectDirection direction );
	virtual void EnemyCollision( ObjectDirection direction );
	
	double counter, spin_counter;
	bool spin;
};

#endif
