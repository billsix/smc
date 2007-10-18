/***************************************************************************
			globals.h  -  global header
                             -------------------
    copyright            :	(C) 2003 - 2007 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

/* *** *** *** *** *** *** *** Maryo ! *** *** *** *** *** *** *** *** *** *** */

// For non-Windows platforms
#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#define CAPTION "Secret Maryo Chronicles"
// Version
#ifndef VERSION
#define VERSION "1.2"
#endif

// framerate speedfactor calculation is based on this
#define DESIRED_FPS 32
// game resolution for global scaling
#define GAME_RES_W 800
#define GAME_RES_H 600

/* *** *** *** *** *** *** *** Object Direction *** *** *** *** *** *** *** *** *** *** */

enum ObjectDirection
{
	// undefined
	DIR_UNDEFINED	= -1,

	// default
	DIR_LEFT	= 0,
	DIR_RIGHT	= 1,
	DIR_UP		= 2,
	DIR_DOWN	= 3,

	DIR_TOP		= 2,
	DIR_BOTTOM	= 3,

	// multi
	DIR_TOP_LEFT = 4,
	DIR_TOP_RIGHT = 5,
	DIR_BOTTOM_LEFT = 6,
	DIR_BOTTOM_RIGHT = 7,
	DIR_LEFT_TOP = 21,
	DIR_LEFT_BOTTOM = 22,
	DIR_RIGHT_TOP = 23,
	DIR_RIGHT_BOTTOM = 24,

	DIR_UP_LEFT = 4,
	DIR_UP_RIGHT = 5,
	DIR_DOWN_LEFT = 6,
	DIR_DOWN_RIGHT = 7,
	DIR_LEFT_UP = 21,
	DIR_LEFT_DOWN = 22,
	DIR_RIGHT_UP = 23,
	DIR_RIGHT_DOWN = 24,

	// extra
	DIR_HORIZONTAL	= 10,	// left or right
	DIR_VERTICAL	= 11,	// up or down
	DIR_ALL			= 20,	// all directions

	// special
	DIR_FIRST		= 100,	// Overworld first waypoint
	DIR_LAST		= 101	// Overworld last waypoint
};

/* *** *** *** *** *** *** *** Default Color *** *** *** *** *** *** *** *** *** *** */

enum DefaultColor
{
	COL_DEFAULT	= -1,
	COL_WHITE	= 0,
	COL_BLACK	= 1,
	COL_RED		= 2,
	COL_ORANGE	= 3,
	COL_YELLOW	= 4,
	COL_GREEN	= 5,
	COL_BLUE	= 6,
	COL_BROWN	= 7,
	COL_GREY	= 8
};

/* *** *** *** *** *** *** *** Game Mode *** *** *** *** *** *** *** *** *** *** */

enum GameMode
{
	MODE_NOTHING		= 0,
	MODE_LEVEL			= 1,
	MODE_OVERWORLD		= 2,
	MODE_MENU			= 3,
	MODE_LEVEL_SETTINGS	= 4
};

enum GameModeType
{
	MODE_TYPE_DEFAULT		= 0,
	MODE_TYPE_LEVEL_CUSTOM	= 1,
	MODE_TYPE_LEVEL_CUSTOM_EDITOR	= 2
};

/* *** *** *** *** *** *** *** Game Action *** *** *** *** *** *** *** *** *** *** */

enum GameAction
{
	GA_NONE				= 0,
	GA_DOWNGRADE_PLAYER	= 1,
	GA_ENTER_MENU		= 2,
	GA_ENTER_MENU_CREDITS = 3,
	GA_ENTER_WORLD		= 4,
	GA_ENTER_LEVEL		= 5,
	GA_ACTIVATE_LEVEL_EXIT	= 6
};

/* *** *** *** *** *** *** *** Debugging *** *** *** *** *** *** *** *** *** *** */

// Enable STLPort Debugging
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

	// disable Visual Studio 8 warnings
	#if _MSC_VER >= 1400
		#ifndef _CRT_SECURE_NO_DEPRECATE
			#define _CRT_SECURE_NO_DEPRECATE
		#endif
		#ifndef _CRT_NON_CONFORMING_SWPRINTFS
			#define _CRT_NON_CONFORMING_SWPRINTFS
		#endif
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
#include <algorithm>

using std::sort;
using std::string;
using std::vector;
using std::flush;
using std::ifstream;
using std::fstream;
using std::ofstream;
using std::stringstream;
using std::ios;


/* *** *** *** *** *** *** *** Paths *** *** *** *** *** *** *** *** *** *** */

// user data directory
extern string user_data_dir;

#ifndef HAVE_CONFIG_H
	#define DATA_DIR "data"
#endif

// Core
#define GAME_OVERWORLD_DIR "world"
#define GAME_LEVEL_DIR "levels"
#define GAME_PIXMAPS_DIR "pixmaps"
#define GAME_SOUNDS_DIR "sounds"
#define GAME_MUSIC_DIR "music"
#define GAME_EDITOR_DIR "editor"
#define GAME_ICON_DIR "icon"
#define GAME_SCHEMA_DIR "schema"
// GUI
#define GUI_SCHEME_DIR "gui/schemes"
#define GUI_IMAGESET_DIR "gui/imagesets"
#define GUI_FONT_DIR "gui/font"
#define GUI_LAYOUT_DIR "gui/layout"
#define GUI_LOOKNFEEL_DIR "gui/looknfeel"
// User
#define USER_SAVEGAME_DIR "savegames"
#define USER_SCREENSHOT_DIR "screenshots"
#define USER_LEVEL_DIR "levels"
#define USER_WORLD_DIR "worlds"

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
#include "RendererModules/OpenGLGUIRenderer/openglrenderer.h"

/* *** *** *** *** *** *** *** XML *** *** *** *** *** *** *** *** *** *** */

#include "CEGUIXMLParser.h"
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

using namespace CEGUI;

/* *** *** *** *** *** *** *** Pre-declare *** *** *** *** *** *** *** *** *** *** */

// Allows use of pointers in header files without including individual headers
// and decreases dependencies between files

/* *** *** *** *** *** *** *** Sprite Types *** *** *** *** *** *** *** *** *** *** */

enum SpriteType
{
	TYPE_UNDEFINED = 0,
	// global
	TYPE_SPRITE = 1,
	TYPE_PASSIVE = 44,
	TYPE_FRONT_PASSIVE = 45,
	TYPE_MASSIVE = 46,
	TYPE_HALFMASSIVE = 5,
	TYPE_CLIMBABLE = 47,
	TYPE_ENEMY = 2,
	TYPE_PLAYER = 3,
	TYPE_ACTIVESPRITE = 4,
	// game
	TYPE_MOUSECURSOR = 100,
	// overworld
	TYPE_OW_WAYPOINT = 55,
	TYPE_OW_LINE_START = 57,
	TYPE_OW_LINE_END = 58,
	// level
	TYPE_LEVELEXIT = 18,
	TYPE_LEVEL_ENTRY = 54,
	TYPE_ENEMYSTOPPER = 20,
	TYPE_BONUSBOX = 26,
	TYPE_SPINBOX = 27,
	TYPE_TEXT_BOX = 59,
	TYPE_MOVING_PLATFORM = 38,
	// enemy
	TYPE_GUMBA = 10,
	TYPE_TURTLE = 19,
	TYPE_TURTLE_BOSS = 56,
	TYPE_JPIRANHA = 29,
	TYPE_ROKKO = 30,
	TYPE_REX = 36,
	TYPE_THROMP = 41,
	TYPE_EATO = 42,
	TYPE_GEE = 43,
	TYPE_SPIKA = 31,
	TYPE_STATIC_ENEMY = 50,
	// items
	TYPE_POWERUP = 23,
	TYPE_MUSHROOM_DEFAULT = 25,
	TYPE_MUSHROOM_LIVE_1 = 35,
	TYPE_MUSHROOM_POISON = 49,
	TYPE_MUSHROOM_BLUE = 51,
	TYPE_MUSHROOM_GHOST = 52,
	TYPE_FIREPLANT = 24,
	TYPE_JGOLDPIECE = 22, // Jumping Goldpiece
	TYPE_FGOLDPIECE = 48, // Falling Goldpiece
	TYPE_GOLDPIECE = 8,
	TYPE_MOON = 37,
	TYPE_JSTAR = 39, // Jumping Star
	// special
	TYPE_BALL = 28,
	// HUD
	TYPE_STATUSTEXT = 9,
	TYPE_POINTDISPLAY = 12,
	TYPE_GAMETIMEDISPLAY = 13,
	TYPE_DEBUGDISPLAY = 14,
	TYPE_LIFEDISPLAY = 15,
	TYPE_GOLDDISPLAY = 16,
	TYPE_MENUBG = 17,
	TYPE_ITEMBOXDISPLAY = 32
};

/* *** *** *** *** *** *** *** Massive Types *** *** *** *** *** *** *** *** *** *** */

enum MassiveType
{
	MASS_PASSIVE = 0,
	MASS_MASSIVE = 1,
	MASS_HALFMASSIVE = 2,
	MASS_CLIMBABLE = 3
};

/* *** Array Types *** */

enum ArrayType
{
	ARRAY_UNDEFINED = 0,
	// level default
	ARRAY_MASSIVE = 1,
	ARRAY_PASSIVE = 2,
	ARRAY_FRONT_PASSIVE = 8,
	ARRAY_ENEMY = 3,
	ARRAY_ACTIVE = 4,
	// hud
	ARRAY_HUD = 5,
	// animation
	ARRAY_ANIM = 6
};

/* *** Ball Effect types ***  */

enum ball_effect
{
	FIREBALL_DEFAULT = 1,
	FIREBALL_EXPLOSION = 2,
	ICEBALL_DEFAULT = 3,
	ICEBALL_EXPLOSION = 4
};

/* *** other ***  */

class cGradientRequest;
class cLineRequest;
class cMenu_Base;
class cObjectCollisionType;
class cObjectCollision;
class cParticleAnimation;
class cRectRequest;
class cSoftware_texture;
class cSurfaceRequest;
class Color;
class cSprite;
class GL_rect;
class GL_Surface;
class GL_line;
class GL_point;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
