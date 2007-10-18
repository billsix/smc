/***************************************************************************
                hud.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __STATUSTEXT_H__
#define __STATUSTEXT_H__

#include "../objects/movingsprite.h"
#include "../core/obj_manager.h"

/* *** *** *** *** *** *** *** cHudSprite *** *** *** *** *** *** *** *** *** *** */

class cHudSprite : public cSprite
{
public:
	cHudSprite( GL_Surface *new_image = NULL, float x = 0, float y = 0, bool del_img = 0 );
	virtual ~cHudSprite( void );
	
	// copy this sprite
	virtual cHudSprite *Copy( void );
};

typedef vector<cHudSprite *> HudSpriteList;

/* *** *** *** *** *** *** *** HudManager *** *** *** *** *** *** *** *** *** *** */

class cHudManager : public cObject_Manager<cHudSprite>
{
public:
	cHudManager( void );
	virtual ~cHudManager( void );

	// Loads the complete HUD
	void Load( void );
	// Unloads the complete HUD
	void Unload( void );

	// Update and reload the Text Displays
	void Update_Text( void );
	// Update the Objects
	void Update( void );
	// Draw the Objects
	void Draw( void );

	// is the HUD loaded
	bool loaded;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The HUD Manager
extern cHudManager *pHudManager;

/* *** *** *** *** *** *** *** *** PointsText *** *** *** *** *** *** *** *** *** */

struct PointsText : public cHudSprite
{
public:
	PointsText( void );
	virtual ~PointsText( void );

	float vely;
	unsigned int points;
};

/* *** *** *** *** *** cMenuBackground *** *** *** *** *** *** *** *** *** *** *** *** */

class cMenuBackground : public cHudSprite
{
public:
	cMenuBackground( float x = 0, float y = 0 );
	virtual ~cMenuBackground( void );

	virtual void Draw( cSurfaceRequest *request = NULL );

	GL_Surface *maryo_head, *goldpiece;
	SDL_Rect rect_maryo_head, rect_goldpiece;
};

/* *** *** *** *** *** cStatusText *** *** *** *** *** *** *** *** *** *** *** *** */

class cStatusText : public cHudSprite
{
public:
	cStatusText( float x = 0, float y = 0 );
	virtual ~cStatusText( void );
	
	virtual void Draw( cSurfaceRequest *request = NULL );
};

/* *** *** *** *** *** cPlayerPoints *** *** *** *** *** *** *** *** *** *** *** *** */

class cPlayerPoints : public cStatusText
{
public:
	cPlayerPoints( float x = 0, float y = 0 );
	virtual ~cPlayerPoints( void );

	void Set_Points( unsigned int points );
	void Add_Points( unsigned int points, float x = 0, float y = 0, string strtext = "", Color color = (Uint8)255, bool allow_multiplier = 0 );

	// removes all point texts
	void Clear( void );

	virtual void Draw( cSurfaceRequest *request = NULL );

	typedef vector<PointsText *> PointsTextList;
	PointsTextList pointsobjects;
};

/* *** *** *** *** *** cGoldDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cGoldDisplay : public cStatusText
{
public:
	cGoldDisplay( float x = 0, float y = 0 );
	virtual ~cGoldDisplay( void );

	void SetGold( int gold );
	void AddGold( int gold );

	virtual void Draw( cSurfaceRequest *request = NULL );
};

/* *** *** *** *** *** cLiveDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cLiveDisplay : public cStatusText
{
public:
	cLiveDisplay( float x = 0, float y = 0 );
	virtual ~cLiveDisplay( void );

	void SetLives( int lives );
	void AddLives( int lives );

	virtual void Draw( cSurfaceRequest *request = NULL );
};

/* *** *** *** *** *** cTimeDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cTimeDisplay : public cStatusText
{
public:
	cTimeDisplay( float x = 0, float y = 0 );
	virtual ~cTimeDisplay( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	char text[50];
	unsigned int old_seconds;
	float counter;
};

/* *** *** *** *** *** cItemBox *** *** *** *** *** *** *** *** *** *** *** *** */

class cItemBox : public cStatusText
{
public:
	cItemBox( float x = 0, float y = 0 );
	virtual ~cItemBox( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );

	/* Set the item
	* sound : if set the box sound is played
	*/
	void Set_Item( SpriteType item_type, bool sound = 1 );
	// Activates the itembox
	void Request_Item( void );
	// push the item back to the itembox
	void Push_back( void );
	
	void Reset( void );

	/* The current Item
	 * uses the Item defines
	 */
	SpriteType item_id;

	// alpha effect
	float item_counter;
	// alpha effect mod
	bool item_counter_mod;

	// itembox color
	Color box_color;

	// stored item
	cMovingSprite *item;
};

/* *** *** *** *** *** cDebugDisplay *** *** *** *** *** *** *** *** *** *** *** *** */

class cDebugDisplay : public cStatusText
{
public:
	cDebugDisplay( float x = 0, float y = 0 );
	virtual ~cDebugDisplay( void );

	// update
	virtual void Update( void );
	// draw
	virtual void Draw( cSurfaceRequest *request = NULL );
	// draw the debugging box
	void DebugBox_Draw( void );

	// set the debug text to display
	void Set_Text( string ntext, float display_time = DESIRED_FPS * 2 );

	// display text
	string text, text_old;
	// text counter
	float counter;

	// CEGUI debugtext
	Window *window_debugtext, *text_debugtext;

	// last game mode
	GameMode Game_Mode_last;
	// last level text
	string level_old;
	// last object counters
	int obj_counter, pass_counter, mass_counter, enemy_counter, active_counter;
	// sprites
	HudSpriteList sprites;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The HUD
extern cPlayerPoints *pointsdisplay;
extern cDebugDisplay *debugdisplay;
extern cGoldDisplay *golddisplay;
extern cLiveDisplay *livedisplay;
extern cTimeDisplay *timedisplay;
extern cItemBox *Itembox;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
