/***************************************************************************
				bonusbox.h  -  header for the corresponding cpp file
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

#ifndef __BONUSBOX_H__
#define __BONUSBOX_H__

#include "../core/globals.h"
#include "../objects/box.h"
#include "../objects/powerup.h"

/* *** *** *** *** *** *** *** *** cBonusBox *** *** *** *** *** *** *** *** *** */

class cBonusBox : public cBaseBox
{
public:
	// constructor
	cBonusBox( float x, float y );
	// create from stream
	cBonusBox( XMLAttributes &attributes );
	// destructor
	virtual ~cBonusBox( void );

	// init defaults
	void Init( void );

	// copy
	virtual cBonusBox *Copy( void );

	// create from stream
	virtual void Create_from_Stream( XMLAttributes &attributes );
	// save to stream
	virtual void Save_to_Stream( ofstream &file );

	// sets the count this object can be activated
	virtual void Set_UseableCount( int count, bool new_startcount = 0 );
	/* Set the bonus type
	 * TYPE_UNDEFINED			for empty
	 * TYPE_MUSHROOM_DEFAULT	for Mushroom
	 * TYPE_FIREPLANT			for Mushroom or Fireplant
	 * TYPE_MUSHROOM_BLUE		for Mushroom or Blue Mushroom
	 * TYPE_MUSHROOM_GHOST		for Mushroom or Ghost Mushroom
	 * TYPE_MUSHROOM_LIVE_1		for 1-UP Mushroom
	 * TYPE_JSTAR				for Star
	 * TYPE_GOLDPIECE			for Goldpiece
	 * TYPE_MUSHROOM_POISON		for Poison Mushroom
	*/
	void Set_Bonus_Type( SpriteType nbonus_type );
	/* Set if to force the best possible item
	 * and not to use lower items automatically
	*/
	void Set_Force_best_item( bool enable );
	// Set the gold color
	void Set_Goldcolor( DefaultColor ncolor );

	// Activates the bonus item
	virtual void Activate( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	// ignore onground check
	virtual void Check_onGround( void ) {};

	// leveleditor activation
	virtual void Editor_Activate( void );
	// leveleditor events
	bool Editor_Animation_Select( const EventArgs &event ); // editor animation option selected
	bool Editor_Item_Select( const EventArgs &event ); // editor item option selected
	bool Editor_Force_best_item_Select( const EventArgs &event ); // editor force best item option selected
	bool Editor_Gold_Color_Select( const EventArgs &event ); // editor gold color option selected

	// force best possible item
	bool force_best_item;

	/* active items
	 * list of active item is available
	 * and gets added to the level objects if finished
	*/
	typedef vector<cMovingSprite *> MovingSpriteList;
	MovingSpriteList active_items;
	// Goldpiece color
	DefaultColor gold_color;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
