/***************************************************************************
			globals.h  -  global header
                             -------------------
    copyright            :	(C) 2003-2004 Artur Hallmann
							(C) 2003-2006 FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

/* *** *** *** *** *** *** *** Maryo ! *** *** *** *** *** *** *** *** *** *** */

#define CAPTION "Secret Maryo Chronicles"
#if !defined( VERSION ) && !defined( HAVE_CONFIG_H )
#define VERSION "0.98 Beta 1"
#endif

// used for animations and the framerate speedfactor calculation
#define DESIRED_FPS 32
// ingame default resolution for scaling
#define GAME_RES_W 800
#define GAME_RES_H 600


// default files for levels
#define DEFAULT_MUSIC "land/land_1.ogg"
#define DEFAULT_BACKGROUND "game/background/wood_1.png"

/* *** *** *** *** *** *** *** Object Directions *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	DIR_NOTHING	= -1,
	DIR_LEFT	= 0,
	DIR_RIGHT	= 1,
	DIR_UP		= 2,
	DIR_DOWN	= 3,
	DIR_HORIZONTAL	= 10,	// left or right
	DIR_VERTICAL	= 11,	// up or down
	DIR_ALL			= 20,	// all directions
	DIR_FIRST		= 100,	// Overworld first waypoint
	DIR_LAST		= 101	// Overworld last waypoint
} ObjectDirection;

/* *** *** *** *** *** *** *** Default Colors *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	COL_DEFAULT	= -1,
	COL_WHITE	= 0,
	COL_BLACK	= 1,
	COL_RED		= 2,
	COL_ORANGE	= 3,
	COL_YELLOW	= 4,
	COL_GREEN	= 5,
	COL_BLUE	= 6,
	COL_BROWN	= 7
} DefaultColor;

/* *** *** *** *** *** *** *** Sound Resource ID's  *** *** *** *** *** *** *** *** *** *** */

// ingame sounds which shouldn't be played twice at the same time
typedef enum
{
	RID_MARYO_JUMP		= 1,
	RID_MARYO_TOCK		= 2,
	RID_MARYO_POWERDOWN = 3,
	RID_MARYO_DEATH		= 1,
	RID_MARYO_FIREBALL	= 2,

	RID_FIREPLANT		= 3,
	RID_MUSHROOM		= 3,
	RID_1UP_MUSHROOM	= 3
} AudioChannel;

/* *** *** *** *** *** *** *** Game Modes *** *** *** *** *** *** *** *** *** *** */

typedef enum
{
	MODE_NOTHING	= 0,
	MODE_LEVEL		= 1,
	MODE_OVERWORLD	= 2,
	MODE_MENU		= 3,
	MODE_LEVEL_SETTINGS	= 4
} GameMode;

/* *** *** *** *** *** *** *** Debugging *** *** *** *** *** *** *** *** *** *** */

#ifdef _DEBUG
	#define _STLP_DEBUG 1
#else
	#undef _STLP_DEBUG
#endif

#ifdef _WIN32
	#ifndef __WIN32__
		#define __WIN32__
	#endif
	#ifdef _DEBUG
		//#pragma warning ( disable : 4786 )
		#pragma warning ( disable : 4244 ) // possible loss of data
		#pragma warning ( disable : 4267 ) // possible loss of data
	#endif
	

	#if _MSC_VER >= 1400 // disable Visual Studio 8 warnings
		#ifndef _CRT_SECURE_NO_DEPRECATE
			#define _CRT_SECURE_NO_DEPRECATE
		#endif
		#ifndef _CRT_NON_CONFORMING_SWPRINTFS
			#define _CRT_NON_CONFORMING_SWPRINTFS
		#endif
	#endif
#endif

/* *** *** *** *** *** *** *** Paths *** *** *** *** *** *** *** *** *** *** */

#ifdef HAVE_CONFIG_H // For non-Windows platforms
	#include "config.h"
#else
	#ifdef __WIN32__
		#define SAVE_DIR "savegames"
		#define OVERWORLD_DIR "data/world"
		#define PIXMAPS_DIR "data/pixmaps"
		#define SOUNDS_DIR "data/sounds"
		#define MUSIC_DIR "data/music"
		#define FONT_DIR "data/font"
//	#else
//		#error HAVE_CONFIG_H is required on non-windows platforms
	#endif
#endif

/* *** *** *** *** *** *** *** Standard setup *** *** *** *** *** *** *** *** *** *** */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <time.h>
#include <math.h>

using std::string;
using std::vector;
using std::flush;
using std::ifstream;
using std::fstream;
using std::ofstream;
using std::stringstream;
using std::ios;
#if defined( _MSC_VER ) && _MSC_VER < 1400 // no Visual Studio 8 and higher
//	using std::find_if;
#endif

/* *** *** *** *** *** *** *** *** SDL *** *** *** *** *** *** *** *** *** */

#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#if defined( __WIN32__ ) && defined( _DEBUG )
	#undef main
#endif

/* *** *** *** *** *** *** *** GUI System *** *** *** *** *** *** *** *** *** *** */

#include "CEGUI.h"
#include "renderers/OpenGLGUIRenderer/openglrenderer.h"

/* *** *** *** *** *** *** *** XML *** *** *** *** *** *** *** *** *** *** */

#include "CEGUIXMLParser.h"
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

using namespace CEGUI;

/* *** *** *** *** *** *** Game setup *** *** *** *** *** *** *** *** *** *** *** */

#include "video/img_manager.h"
#include "obj_manager.h"
#include "framerate.h"
#include "collision.h"

extern cFramerate Framerate;

#include "file_parser.h"
#include "video/img_settings.h"
#include "video/video.h"
#include "user/savegame.h"
#include "audio.h"
#include "video/animation.h"
#include "input/joystick.h"
#include "gui/menu.h"
#include "objects/sprite.h"
#include "objects/active.h"
#include "input/mouse.h"
// level
#include "level/level_settings.h"
#include "level/leveleditor.h"
#include "level/level.h"
// objects
#include "objects/goldpiece.h"
#include "objects/star.h"
#include "objects/moving_platform.h"
#include "objects/cloud.h"
#include "objects/levelexit.h"
#include "objects/powerup.h"
#include "objects/fireball.h"
#include "objects/box.h"
#include "player.h"
// enemies
#include "enemies/enemy.h"
#include "enemies/turtle.h"
#include "enemies/goomba.h"
#include "enemies/jpiranha.h"
#include "enemies/banzai_bill.h"
#include "enemies/rex.h"
#include "user/preferences.h"
// overworld
#include "overworld/waypoint.h"
#include "overworld/layer.h"
#include "overworld/player.h"
#include "overworld/overworld.h"
#include "gui/dialog.h"
#include "gui/hud.h"

/* *** *** *** *** *** *** *** *** Variables *** *** *** *** *** *** *** *** *** */

// Image Manager
extern cImageManager *pImageManager;
// Sprite Manager
extern cObjectManager *pObjManager;
// The HUD Manager
extern cHudManager *pHudManager;
// The Animation Manager
extern cAnimationManager *pAnimationManager;
// The Dialog Manager
extern cDialogManager *pDialogManager;
// The Joystick
extern cJoystick *pJoystick;
// GUI System
extern OpenGLRenderer *pGuiRenderer;
extern System *pGuiSystem;

// The Player
extern cPlayer *pPlayer;
// The Level
extern cLevel *pLevel;
// The Overworld
extern cOverWorld *pOverWorld;
// The Menu
extern cMainMenu *pMenu;
// The Mouse
extern cMouseCursor *pMouseCursor;
// The HUD
extern cPlayerPoints *pointsdisplay;
extern cDebugDisplay *debugdisplay;
extern cGoldDisplay *golddisplay;
extern cLiveDisplay *livedisplay;
extern cTimeDisplay *timedisplay;
extern cItemBox *Itembox;
// The Leveleditor
extern cLevelEditor *pLeveleditor;
// Audio Handler
extern cAudio *pAudio;
// The Preferences
extern cPreferences *pPreferences;
// The Savegame Handler
extern cSavegame *pSavegame;

extern bool done;
extern SDL_Surface *screen;
extern Color std_bgcolor, blue, darkblue, lightblue, black, white, grey, lightgrey, lightgreyalpha, green, lightgreenalpha, greenyellow, darkgreen, red, lightred, lila, orange;
extern TTF_Font *font_normal, *font_small;
// Pressed keys
extern Uint8 *keys;
// Event system
extern SDL_Event event;
/* 0 = disabled
 * 1 = Level Editor
 * 2 = Object Editor
 */
extern int Leveleditor_Mode;
/* 0 = nothing
 * 1 = Level
 * 2 = Overworld
 * 3 = Menu
 */
extern GameMode Game_Mode;
extern bool Game_debug;

// The Camera Position
extern double cameraposx, cameraposy;
// global image scale
extern double global_scalex, global_scaley;

/* *** *** *** *** *** *** *** Functions *** *** *** *** *** *** *** *** *** *** */

// Function: is_valid_number
// Returns true if the number is valid
bool is_valid_number( string num );

/* Function: Copy_Object
 * Returns the Copied Object
 * with the given position
 * if no position is given default is 0
 * returns NULL if a error occured
 */
cSprite *Copy_Object( cSprite *CopyObject, double x = 0, double y = 0 );

// Function: Get_current_time
// Returns the current computer time
string Get_current_time( void );

// Function: ClearEvents
// Clears the complete input event queue
void ClearEvents( void ); 

// Function: Gui_handle_Mouse_down
// GUI generic function
void Gui_handle_Mouse_down( Uint8 button );
// Function: Gui_handle_Mouse_down
// GUI generic function
void Gui_handle_Mouse_up( Uint8 button );
// Function: Gui_handle_Mouse_down
// GUI generic function
void Gui_handle_Time( void );

/* Function: EditMessageBox
 * an EditBox
 *
 * Parameters:
 * default_text = the Default Text
 * auto_no_text = if a key is pressed the default text dissapers
 */
string EditMessageBox( string default_text, string title_text, double pos_x, double pos_y, bool auto_no_text = 1 );

/* Function: Preload_images
 * Preloads the common images
 * into the image manager
 */
void Preload_images( void );

/* Function: Delete_file
 * Deletes the given file
 * Use with Caution
 *
 * Returns:
 * true on success
*/
bool Delete_file( string filename );

// Function: valid_file
// Checks if the file exists
bool valid_file( string filename );

// Function: int_to_string
// Returns the number as a string
string int_to_string( int number );
// Function: double_to_string
// Returns the number as a string
string double_to_string( double number );
// Returns the string as a float
float string_to_float( const string &str );
// Function: string_to_int
// Returns the string as a number
int string_to_int( string str );
// Function: string_to_double
// Returns the string as a double
double string_to_double( string str );

#endif
