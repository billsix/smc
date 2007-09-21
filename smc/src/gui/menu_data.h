/***************************************************************************
                menu_data.h  -  header for the corresponding cpp file
                             -------------------
    copyright            :	(C) 2004 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MENU_DATA_H__
#define __MENU_DATA_H__

#include "../core/globals.h"
#include "../gui/menu.h"
#include "../gui/hud.h"

/* *** *** *** *** *** *** *** cMenu_Base *** *** *** *** *** *** *** *** *** *** */

class cMenu_Base
{
public:
	cMenu_Base( void );
	~cMenu_Base( void );

	virtual void Init( void );
	virtual void Init_GUI( void );
	virtual void Update( void );
	virtual void Draw( void );
	void Draw_End( void );

	// Exit
	virtual void Exit( void );

	// gui layout filename
	string layout_file;
	// CEGUI window
	Window *guiwindow;

	// if button/key action
	bool action;

	// menu position
	float menu_posy;
	// default text color
	Color text_color;
	// value text color
	Color text_color_value;

	// current menu sprites
	typedef vector<cHudSprite *> HudSpriteList;
	HudSpriteList drawlist;
};

/* *** *** *** *** *** *** *** cMenu_Main *** *** *** *** *** *** *** *** *** *** */

class cMenu_Main : public cMenu_Base
{
public:
	cMenu_Main( void );
	virtual ~cMenu_Main( void );

	virtual void Init( void );
	virtual void Init_GUI( void );
	virtual void Update( void );
	virtual void Draw( void );

	// Exit
	virtual void Exit( void );
};

/* *** *** *** *** *** *** *** cMenu_Start *** *** *** *** *** *** *** *** *** *** */

class cMenu_Start : public cMenu_Base
{
public:
	cMenu_Start( void );
	virtual ~cMenu_Start( void );

	virtual void Init( void );
	virtual void Init_GUI( void );
	virtual void Update( void );
	virtual void Draw( void );

	// Exit
	virtual void Exit( void );

	// Get all levels from the given directory
	void Get_Levels( string dir, colour color );

	/* Load the Selected World
	 * and exits if successful
	*/
	void Load_World( string level_name );
	/* Load the Selected Level
	 * and exits if successful
	*/
	void Load_Level( string name );

	// CEGUI events
	bool window_keydown( const EventArgs &event ); // key down
	bool world_Select( const EventArgs &event ); // world selected
	bool world_Select_final_list( const EventArgs &event ); // world selected for entering
	bool world_Select_final_button( const EventArgs &event ); // button for world entering
	bool level_Select( const EventArgs &event ); // level selected
	bool level_Select_final_list( const EventArgs &event ); // level selected for entering
	bool level_Select_final_button( const EventArgs &event ); // button for level entering 
};

/* *** *** *** *** *** *** *** cMenu_Options *** *** *** *** *** *** *** *** *** *** */

class cMenu_Options : public cMenu_Base
{
public:
	cMenu_Options( void );
	virtual ~cMenu_Options( void );

	virtual void Init( void );
	virtual void Init_GUI( void );
	virtual void Update( void );
	virtual void Draw( void );

	// Exit
	virtual void Exit( void );
};

/* *** *** *** *** *** *** *** cMenu_Options_Game *** *** *** *** *** *** *** *** *** *** */

class cMenu_Options_Game : public cMenu_Options
{
public:
	cMenu_Options_Game( void );
	virtual ~cMenu_Options_Game( void );

	virtual void Init( void );
	virtual void Init_GUI( void );
	virtual void Update( void );
	virtual void Draw( void );

	// CEGUI events
	bool always_run_Select( const EventArgs &event ); // always run option selected
	bool camera_hor_Select( const EventArgs &event ); // Camera Horizontal Value changed
	bool camera_ver_Select( const EventArgs &event ); // Camera Vertical Value changed
};

/* *** *** *** *** *** *** *** cMenu_Options_Video *** *** *** *** *** *** *** *** *** *** */

class cMenu_Options_Video : public cMenu_Options
{
public:
	cMenu_Options_Video( void );
	virtual ~cMenu_Options_Video( void );

	virtual void Init( void );
	virtual void Init_GUI( void );
	virtual void Update( void );
	virtual void Draw( void );

	// CEGUI events
	bool res_Select( const EventArgs &event ); // resolution option selected
	bool bpp_Select( const EventArgs &event ); // bpp option selected
	bool fullscreen_Select( const EventArgs &event ); // fullscreen option selected
	bool vsync_Select( const EventArgs &event ); // vsync option selected

	// Fullscreen Image
	cHudSprite *fullscreen;

	// video settings
	unsigned int vid_w;
	unsigned int vid_h;
	unsigned int vid_bpp;
	bool vid_fullscreen;
	bool vid_vsync;
};

/* *** *** *** *** *** *** *** cMenu_Options_Audio *** *** *** *** *** *** *** *** *** *** */

class cMenu_Options_Audio : public cMenu_Options
{
public:
	cMenu_Options_Audio( void );
	virtual ~cMenu_Options_Audio( void );

	virtual void Init( void );
	virtual void Init_GUI( void );
	virtual void Update( void );
	virtual void Draw( void );

	// CEGUI events
	bool music_Select( const EventArgs &event ); // music option selected
	bool music_vol_changed( const EventArgs &event ); // music volume changed
	bool sound_Select( const EventArgs &event ); // sound option selected
	bool sound_vol_changed( const EventArgs &event ); // sound volume changed

	// Audio
	cHudSprite *music;
	cHudSprite *sounds;
	cHudSprite *audio_music;
	cHudSprite *audio_sounds;
};

/* *** *** *** *** *** *** *** cMenu_Options_Controls *** *** *** *** *** *** *** *** *** *** */

class cMenu_Options_Controls : public cMenu_Options
{
public:
	cMenu_Options_Controls( void );
	virtual ~cMenu_Options_Controls( void );

	virtual void Init( void );
	virtual void Init_GUI( void );
	virtual void Update( void );
	virtual void Draw( void );

	void Joy_Default( unsigned int index );
	void Joy_Disable( void );

	// CEGUI events
	bool joy_name_Select( const EventArgs &event ); // joystick name option selected
	bool joy_analog_jump_Select( const EventArgs &event ); // joystick analog jump option selected

	// Gamepad Image
	cHudSprite *gamepad;
	// input names
	// Keyboard
	cHudSprite *keyboard_up_keyname;
	cHudSprite *keyboard_down_keyname;
	cHudSprite *keyboard_left_keyname;
	cHudSprite *keyboard_right_keyname;
	cHudSprite *keyboard_jump_keyname;
	cHudSprite *keyboard_shoot_keyname;
	cHudSprite *keyboard_action_keyname;
	// Joypad
	cHudSprite *joypad_jump_keyname;
	cHudSprite *joypad_item_keyname;
	cHudSprite *joypad_shoot_keyname;
	cHudSprite *joypad_action_keyname;
	cHudSprite *joypad_exit_keyname;
};

/* *** *** *** *** *** *** *** cMenu_Savegames *** *** *** *** *** *** *** *** *** *** */

class cMenu_Savegames : public cMenu_Base
{
public:
	cMenu_Savegames( bool ntype_save );
	virtual ~cMenu_Savegames( void );

	virtual void Init( void );
	virtual void Init_GUI( void );
	virtual void Update( void );
	virtual void Draw( void );

	// Exit
	virtual void Exit( void );

	void Update_Load( void );
	void Update_Save( void );

	// Set Savegame Description
	string Set_SaveDescription( unsigned int save_slot );
	// Update Savegame Descriptions
	void Update_SavedGames_Text( void );

	// Savegame images
	HudSpriteList savegame_temp;

	// if save menu
	bool type_save;
};

/* *** *** *** *** *** *** *** cMenu_Credits *** *** *** *** *** *** *** *** *** *** */

class cMenu_Credits : public cMenu_Base
{
public:
	cMenu_Credits( void );
	virtual ~cMenu_Credits( void );

	virtual void Init( void );
	virtual void Init_GUI( void );
	virtual void Update( void );
	virtual void Draw( void );

	// Exit
	virtual void Exit( void );
};

#endif

