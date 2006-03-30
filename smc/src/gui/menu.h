/***************************************************************************
                menu.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2003 - 2006 by FluXy
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

#include "../globals.h"
#include "../level/level.h"

/* *** *** *** *** *** *** *** Menu ID's *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	MENU_NOTHING = 0,
	MENU_MAIN = 1,
	MENU_OPTIONS = 2,
	MENU_LOAD = 3,
	MENU_SAVE = 4
} MenuID;

typedef enum
{
	MENU_VIDEO = 1,
	MENU_AUDIO = 2,
	MENU_CONTROLS = 3
} OptionMenuID;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Menu Item
class cMenu_Item : public cSprite
{
public:
	cMenu_Item( void );
	~cMenu_Item( void );

	// Sets the Active Modifier
	void Set_Active( bool nactive = 0 );
	// Draws the Menu Item
	void Draw( void );

	// The menu images
	cSprite *image_default;
	// The additional Menu Graphic
	cSprite *image_menu;
	
	// if this item quits the menu
	bool isquit;
private:
	// Is this Item active
	bool active;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

typedef vector<cMenu_Item *> MenuList;

/* The Menu Handler
* Can load at anytime new Menu-Items
*/
class cMenuHandler
{
public:
	cMenuHandler( GL_Surface *img_logo, GL_Surface *img_logo_quit, unsigned int posx, unsigned int posy = 10 );
	~cMenuHandler( void );

	// Adds a Menu
	void Add_MenuItem( cMenu_Item *item, Color *shadow = NULL );
	// Unloads all items
	void Reset( void );

	/* Sets the Active Menu Item
	* if set to -1 nothing is active
	*/
	void Set_Active( int num );

	// Updates the Menu Mouse Collision detection
	void Update_Mouse( void );
	// Draws the Menu
	void Draw( void );

	// Returns the currently active Menu Item
	cMenu_Item *Get_Active_Item( void );
	// Returns the number of loaded Menus
	unsigned int Get_Size( void );

	// The Logo images
	GL_Surface *image_logo;
	GL_Surface *image_logo_quit;
	// The Logo Sprite
	cSprite *logo;

	/* The Currently Active Menu Item
	* if set to -1 nothing is active
	*/
	int active;
private:
	MenuList items;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

typedef vector<cSprite *> SpriteList;

class cMainMenu
{
public:
	cMainMenu( void );
	~cMainMenu( void );

	void LoadImages( void );
	void FreeImages( void );

	// Generic Menu Input handler
	void UpdateGeneric( void );
	// hendles keydown events
	void KeyDown_menu( SDLKey key );

	// Returns a Menu with the common image filenames
	cMenu_Item *Auto_Menu( string imagename, string imagename_menu, double ypos = 0, bool is_quit = 0 );

	// Enters the given Menu
	void Enter_Menu( MenuID menu_id = MENU_MAIN );

	// Main Menu
	void Menu_Action( void );
	// Options Menu
	void Menu_Options_Action( void );
	// Controls Menu
	void Menu_Controls_Action( void );
	bool Menu_Controls_joystick_sel( const EventArgs &event ); // joystick option selected
	// Audio Menu
	void Menu_Audio_Action( void );
	bool Menu_Audio_music_sel( const EventArgs &event ); // music option selected
	bool Menu_Audio_music_vol_changed( const EventArgs &event ); // music volume changed
	bool Menu_Audio_sound_sel( const EventArgs &event ); // sound option selected
	bool Menu_Audio_sound_vol_changed( const EventArgs &event ); // sound volume changed
	// Video Menu
	void Menu_Video_Action( void );
	bool Menu_Video_res_sel( const EventArgs &event ); // resolution option selected
	bool Menu_Video_bpp_sel( const EventArgs &event ); // bpp option selected
	bool Menu_Video_fullscreen_sel( const EventArgs &event ); // fullscreen option selected
	// Savegame Load Menu
	void Menu_Load_Action( void );
	// Savegame Save Menu
	void Menu_Save_Action( void );

	// Sets the Savegame Description
	string Set_SaveDescription( unsigned int save_slot );
	// Updates all the Savegame Descriptions
	void GetSavedGames( void );

	/* For leaving the current menu
	 * 1 = default leaving
	 * 2 = The user pressed ESC
	*/
	unsigned int leave;
	// if a button got pressed or another key action
	bool action;
	// for entering another menu after leaving
	MenuID next_menu;
	// the current option menu
	OptionMenuID options_menu_id;

	// Control Items
	cSprite *controls_up_keyname;
	cSprite *controls_down_keyname;
	cSprite *controls_left_keyname;
	cSprite *controls_right_keyname;
	cSprite *controls_shoot_keyname;

	cSprite *gamepad;

	// Video Items
	cSprite *fullscreen;

	// Audio Items
	cSprite *music;
	cSprite *sounds;
	cSprite *audio_music;
	cSprite *audio_sounds;

	// Menu handler
	cMenuHandler *handler;
	// Menu Animation Manager
	cAnimationManager *pMenu_AnimManager;
private:
	// temporary video settings
	unsigned int vid_w;
	unsigned int vid_h;
	unsigned int vid_bpp;
	bool vid_fullscreen;

	// Savegame temporary images
	SpriteList savegame_temp;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
