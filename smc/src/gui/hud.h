/***************************************************************************
                hud.h  -  header for the corresponding cpp file
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

#ifndef __STATUSTEXT_H__
#define __STATUSTEXT_H__

#include "../globals.h"

/* *** *** *** *** *** *** *** *** Pointinfo *** *** *** *** *** *** *** *** *** */

struct PointsText : public cSprite
{
public:
	PointsText( void );
	~PointsText( void );

	double vely;
	unsigned int points;
};

/* *** *** *** *** *** *** *** HudManager *** *** *** *** *** *** *** *** *** *** */

typedef vector<cSprite*> HudSpriteList;

class cHudManager
{
public:
	cHudManager( void );
	~cHudManager( void );

	// Loads the complete HUD
	void Load( void );
	// Unloads the complete HUD
	void Unload( void );

	// Adds a HUD Object
	void AddObject( cSprite *obj );

	// Updates and reloads the Text Displays
	void UpdateText( void );
	// Updates the Objects
	void Update( void );

	// Returns the current size
	unsigned int Get_Size( void );

	// is the HUD loaded
	bool loaded;
	// HUD objects
	HudSpriteList Objects;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cMenuBackground : public cActiveSprite
{
public:
	cMenuBackground( double x = 0, double y = 0 );
	~cMenuBackground( void );

	virtual void Draw( void );
	virtual void Update( void );	

	GL_Surface *mario_head, *goldpiece;
	SDL_Rect rect_mario_head, rect_goldpiece;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cStatusText : public cSprite
{
public:
	cStatusText( double x = 0, double y = 0 );
	~cStatusText( void );
	
	virtual void Update( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cPlayerPoints : public cStatusText
{
public:
	cPlayerPoints( double x = 0, double y = 0 );
	~cPlayerPoints( void );

	void SetPoints( unsigned int points );
	void AddPoints( unsigned int points, double x = 0, double y = 0, string strtext = "", Color color = (Uint8)255 );

	// removes all point texts
	void Clear( void );

	virtual void Update( void );

	typedef vector<PointsText *> PointsTextList;
	PointsTextList PointTexts;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cGoldDisplay : public cStatusText
{
public:
	cGoldDisplay( double x = 0, double y = 0 );
	~cGoldDisplay( void );

	void SetGold( int gold );
	void AddGold( int gold );

	virtual void Update( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cLiveDisplay : public cStatusText
{
public:
	cLiveDisplay( double x = 0, double y = 0 );
	~cLiveDisplay( void );

	void SetLives( int lives );
	void AddLives( int lives );

	virtual void Update( void );
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cTimeDisplay : public cStatusText
{
public:
	cTimeDisplay( double x = 0, double y = 0 );
	~cTimeDisplay( void );

	virtual void Update( void );

	char text[50], text_old[50];
	double counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cItemBox : public cStatusText
{
public:
	cItemBox( double x = 0, double y = 0 );
	~cItemBox( void );

	virtual void Update( void );

	// Set the item
	void Set_Item( unsigned int item_type );
	// Activates the itembox
	void Request_Item( void );
	// push the item back to the itembox
	void Push_back( void );
	
	void Reset( void );

	/* The current Item
	 * uses the Item defines
	 */
	unsigned int item_id;

	// alpha effect
	double item_counter;
	// alpha effect mod
	bool item_counter_mod;

	// itembox color
	Color box_color;

	// stored item
	cSprite *item;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cDebugDisplay : public cStatusText
{
public:
	cDebugDisplay( double x = 0, double y = 0 );
	~cDebugDisplay( void );

	virtual void Update( void );
	void GameDebugDraw( void );

	// display text
	string text, text_old;
	// temp level text
	string level_old;
	// temp object counts
	int passcount, masscount, enemcount, activcount;
	// sprites
	SpriteList sprites;
	// text counter
	double counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
