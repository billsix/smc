/***************************************************************************
                menu.h  -  header for the corresponding cpp file
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

#ifndef __MENU_H__
#define __MENU_H__

#include "../core/globals.h"
#include "../video/animation.h"
#include "../video/video.h"
#include "../gui/hud.h"

/* *** *** *** *** *** *** *** Menu Types *** *** *** *** *** *** *** *** *** *** */

enum MenuID
{
	MENU_NOTHING = 0,
	MENU_MAIN = 1,
	MENU_START = 5,
	MENU_OPTIONS = 2,
	MENU_LOAD = 3,
	MENU_SAVE = 4,
	MENU_CREDITS = 6
};

enum OptionMenuID
{
	MENU_GAME = 0,
	MENU_VIDEO = 1,
	MENU_AUDIO = 2,
	MENU_CONTROLS = 3
};

/* *** *** *** *** *** *** cMenu_Item *** *** *** *** *** *** *** *** *** *** *** */

class cMenu_Item : public cHudSprite
{
public:
	cMenu_Item( void );
	~cMenu_Item( void );

	// Sets the Active Modifier
	void Set_Active( bool nactive = 0 );
	// Draws the Menu Item
	virtual void Draw( cSurfaceRequest *request = NULL );

	// The menu images
	cHudSprite *image_default;
	// The additional Menu Graphic
	cHudSprite *image_menu;
	
	// if this item quits the menu
	bool isquit;
private:
	// Is this Item active
	bool active;
};

typedef vector<cMenu_Item *> MenuList;

/* *** *** *** *** *** *** cMenuHandler *** *** *** *** *** *** *** *** *** *** *** */

/*
* handle dynamic Menu-Items
*/
class cMenuHandler
{
public:
	cMenuHandler( void );
	~cMenuHandler( void );

	// Adds a Menu
	void Add_MenuItem( cMenu_Item *item, float shadow_pos = 0, Color shadow_color = (Uint8)0 );

	// Unloads all items
	void Reset( void );

	/* Sets the Active Menu Item
	* if set to -1 nothing is active
	*/
	void Set_Active( int num );

	// Set if quit ground image is used
	void Use_Quit_Image( bool enabled = 1 );

	// Updates the Menu Mouse Collision detection
	void Update_Mouse( void );
	// Draws the Menu
	void Draw( void );

	// Returns the currently active Menu Item
	cMenu_Item *Get_Active_Item( void );
	// Returns the number of loaded Menus
	unsigned int Get_Size( void );

	// Logo images
	GL_Surface *image_ground;
	GL_Surface *image_ground_quit;
	// Logo Sprite
	cHudSprite *logo;
	// Ground Sprite
	cHudSprite *ground;
	// Moon Sprite
	cHudSprite *moon;
	// Cloud Sprite
	cHudSprite *cloud;
	// Gradient Colors
	Color color_start;
	Color color_end;

	/* The Currently Active Menu Item
	* if set to -1 nothing is active
	*/
	int active;
private:
	MenuList items;

	// if quit ground image is allowed
	bool use_quit_image;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

class cMenuCore
{
public:
	cMenuCore( void );
	~cMenuCore( void );

	// Generic Menu Input handler
	void Update_Generic( void );
	/* handle key down event
	 * returns true if the key was processed
	*/
	bool Key_Down( SDLKey key );
	/* handle key up event
	 * returns true if the key was processed
	*/
	bool Key_Up( SDLKey key );

	// Returns a Menu with the common image filenames
	cMenu_Item *Auto_Menu( string imagename, string imagename_menu, float ypos = 0, bool is_quit = 0 );

	// Enters the given Menu
	void Enter( MenuID menu_id = MENU_MAIN );

	// Update current Menu
	void Update( void );
	// Update the Menu Animation
	void Update_Animation( void );
	// Draw current Menu
	void Draw( void );

	/* For leaving the current menu
	 * 1 = default leaving
	 * 2 = The user pressed ESC
	*/
	unsigned int leave;
	// for entering another menu after leaving
	MenuID next_menu;
	// the current option menu
	OptionMenuID options_menu_id;
	// background particle animation counter
	float particle_counter;

	// Menu class
	cMenu_Base *pMenu_Data;

	// Menu handler
	cMenuHandler *handler;
	// Menu Animation Manager
	cAnimationManager *pMenu_AnimManager;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Menu
extern cMenuCore *pMenuCore;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
