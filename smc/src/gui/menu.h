/***************************************************************************
 * menu.h  -  header for the corresponding cpp file
 *
 * Copyright (C) 2003 - 2008 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_MENU_H
#define SMC_MENU_H

#include "../core/globals.h"
#include "../video/animation.h"
#include "../video/video.h"
#include "../gui/hud.h"
#include "../core/camera.h"

/* *** *** *** *** *** *** *** Menu Types *** *** *** *** *** *** *** *** *** *** */

namespace SMC
{ // work around conflicts caused by lack of namespace use in smc

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
}
using namespace SMC;

/* *** *** *** *** *** *** cMenu_Item *** *** *** *** *** *** *** *** *** *** *** */

class cMenu_Item : public cHudSprite
{
public:
	cMenu_Item( void );
	virtual ~cMenu_Item( void );

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
	void Add_Menu_Item( cMenu_Item *item, float shadow_pos = 0, Color shadow_color = static_cast<Uint8>(0) );

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

	// Logo image
	GL_Surface *image_ground;
	// Logo Sprite
	cHudSprite *logo;
	// background Sprites
	cHudSprite *ground;
	cHudSprite *moon;
	cHudSprite *background;
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

	// Handle Input event
	bool Handle_Event( SDL_Event *ev );
	/* handle key down event
	 * returns true if processed
	*/
	bool Key_Down( SDLKey key );
	/* handle key up event
	 * returns true if processed
	*/
	bool Key_Up( SDLKey key );
	/* handle mouse button down event
	 * returns true if processed
	*/
	bool Mouse_Down( Uint8 button );
	/* handle mouse button up event
	 * returns true if processed
	*/
	bool Mouse_Up( Uint8 button );
	/* handle joystick button down event
	 * returns true if processed
	*/
	bool Joy_Button_Down( Uint8 button );
	/* handle joystick button up event
	 * returns true if processed
	*/
	bool Joy_Button_Up( Uint8 button );


	// Returns a Menu with the common image filenames
	cMenu_Item *Auto_Menu( string imagename, string imagename_menu, float ypos = 0, bool is_quit = 0 );

	// Load the given Menu
	void Load( MenuID menu = MENU_MAIN );
	// Unload
	void Unload( void );

	// Update current Menu
	void Update( void );
	// Draw current Menu
	void Draw( void );

	// current menu id
	MenuID menu_id;
	// for entering another menu after leaving
	MenuID next_menu;
	// the current option menu
	OptionMenuID options_menu_id;

	// menu camera
	cCamera *camera;
	// Menu class
	cMenu_Base *pMenu_Data;

	// Menu handler
	cMenuHandler *handler;
	// Menu Animation Manager
	cAnimation_Manager *pMenu_AnimManager;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Menu
extern cMenuCore *pMenuCore;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
