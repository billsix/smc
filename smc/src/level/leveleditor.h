/***************************************************************************
     leveleditor.h  -  header for the corresponding cpp file
                             -------------------
    copyright            : (C) 2004 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __LEVELEDITOR_H__
#define __LEVELEDITOR_H__

#include "../globals.h"

// Leveleditor cameraspeed
#define CAMERASPEED 35

/* *** *** *** *** *** *** *** Main Menus *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	MENU_BOX = 1,
	MENU_GROUND_GREEN_1 = 2,
	MENU_GROUND_SAND_1 = 16,
	MENU_GROUND_GREEN_3 = 17,
	MENU_PLASTIC_1 = 15,
	MENU_JUNGLE_1 = 13,
	MENU_GHOST_1 = 14,
	MENU_HEDGES_GREEN = 3,
	MENU_PIPES = 4,
	MENU_HILLS = 5,
	MENU_SIGNS = 6,
	MENU_BLOCKS = 7,
	MENU_CLOUDS = 8,
	MENU_ENEMY = 10,
	MENU_SPECIAL = 11,
	MENU_EXTRA = 12,
	// special types
	MENU_FUNCTION = 1000,
	MENU_SPACER = 2000
} editor_main_type;

/* *** *** *** *** *** *** *** Leveleditor sprite types *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	EDITOR_MASSIVE = 1001,
	EDITOR_PASSIVE = 1002,
	EDITOR_HALFMASSIVE = 1003,
	EDITOR_ENEMY = 1004
} editor_sprite_type;

/* *** *** *** *** *** *** *** *** Main Menu Object *** *** *** *** *** *** *** *** *** */

class cMenu_Main_Object
{
public:
	cMenu_Main_Object( void );
	~cMenu_Main_Object( void );

	// Description
	string name;
	// number
	unsigned int CountId;
	// The default menu id
	editor_main_type Item_Menu_Id;
	/* 0 = default
	 * 1 = hovered
	 * 2 = active
	 */
	unsigned int state;
	/* if the menu is a function
	 * this function will be called
	*/ 
	void ( *pfunction )( void );
	
	GL_Surface *def_img;
	GL_Surface *hover_img;
	
	// collision rect
	GL_rect Col_rect;

	// color
	Color color;

	// current window position
	int cur_posy;

	// spacer height
	int spacer_height;
};

/* *** *** *** *** *** *** *** *** Item Menu Object *** *** *** *** *** *** *** *** *** */

class cMenu_Item_Object
{
public:
	cMenu_Item_Object( void );
	~cMenu_Item_Object( void );

	GL_Surface *preview_img;

	GL_Surface *name_img;
	GL_Surface *size_img;

	// Description
	string name;
	// The image path
	string path;

	// sprite
	cSprite *Sprite_type;

	unsigned int CountId;

	// collision rect
	GL_rect Col_rect;

	// scale factor
	double scale;

	// visible on screen
	bool visible;
};

/* *** *** *** *** *** *** *** ObjectScroller *** *** *** *** *** *** *** *** *** *** */

class cObjectScroller : public cSprite
{
public:
	cObjectScroller( void ) : cSprite()
	{
		active = 0;
		image = 0;
		image_default = NULL;
		image_hover = NULL;
	}
	~cObjectScroller( void ) {};

	bool active;

	GL_Surface *image_default, *image_hover;
};

/* *** *** *** *** *** *** *** Leveleditor *** *** *** *** *** *** *** *** *** *** */

class cLevelEditor
{
public:
	cLevelEditor( void );
	~cLevelEditor( void );

	// Load the default Main Menu
	void Load_Default_Menu( void );
	// Unload the Main Menu
	void Unload_Main_Menu( void );

	// Adds a Main Menu Entry
	void Add_Main_Object( string name, editor_main_type Item_Menu_link, void ( *nfunction )( void ), Color def_color = (Uint8)0 );

	// Gets an Item Object
	cMenu_Main_Object *Get_Main_Object( unsigned int nId );

	// Sets the Selectable Objects Position
	void Set_Main_Objects_Pos( void );
	// Set the Main Menu active Object
	void Set_Main_Active( unsigned int object_id );

	// Loads an defined Menu
	void Load_Item_Menu( unsigned int menu_id );
	// Unloads the Menu
	void Unload_Item_Menu( void );

	// Draws the Leveleditor Menu
	void Draw( void );

	/* Adds an Object to the Item list
	 *	use no_basepath if the image is in an other directory
	 *	than the given basepath directory
	 */
	void Add_Item_Object( cSprite *nSprite_type, string nName = "", GL_Surface *image = NULL );
	// Gets an Item Object
	cMenu_Item_Object *Get_Item_Object( unsigned int nId );
	// Sets the Selectable Objects Position
	void Set_Item_Objects_Pos( void );
	
	// switches the current state of the MouseObject
	void Switch_ObjectState( void );
	// push the MouseObject into the front or  back
	void Change_DrawPosition( bool push_back );

	// Main Window Position
	GL_rect wMain;
	// Item Window Position	
	GL_rect wItem;

	// The Item Object Scroller
	cObjectScroller *Item_scroller_left, *Item_scroller_right;

	// The Vertical Scroll Position of the Item Window	
	unsigned int Item_pos;

	// The Horizontal Scroll Position of the Item Objects
	double Item_scoll;

	// The Timer until the Menu will be minimized
	double wMain_timer, wItem_timer;
	// The size if the Menu ( for animation )
	double wMain_open, wItem_open;

	// The current y main window position
	int main_cur_posy;

	// Item Menu objects
	vector<cMenu_Item_Object *> wItem_Objects;
	// Main Menu objects
	vector<cMenu_Main_Object *> wMenu_Objects;
};

/* *** *** *** *** *** *** *** Menu functions *** *** *** *** *** *** *** *** *** *** */

void LE_Settings( void );
void LE_New( void );
void LE_Load( void );
void LE_Save( void );
void LE_Save_as( void );
void LE_Clear( void );
void LE_Exit( void );
void LE_Enter_Overworld( void );

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
