/***************************************************************************
			globals.h  -  global header
                             -------------------
    copyright            :	(C) 2003 - 2007 Florian Richter
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

#ifdef HAVE_CONFIG_H // For non-Windows platforms
	#include "config.h"
#endif

#define CAPTION "Secret Maryo Chronicles"
#ifndef VERSION
#define VERSION "0.99.6"
#endif

// used for animations and the framerate speedfactor calculation
#define DESIRED_FPS 32
// ingame default resolution for scaling
#define GAME_RES_W 800
#define GAME_RES_H 600

/* *** *** *** *** *** *** *** Object Directions *** *** *** *** *** *** *** *** *** *** */

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

/* *** *** *** *** *** *** *** Default Colors *** *** *** *** *** *** *** *** *** *** */

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

/* *** *** *** *** *** *** *** Game Modes *** *** *** *** *** *** *** *** *** *** */

enum GameMode
{
	MODE_NOTHING	= 0,
	MODE_LEVEL		= 1,
	MODE_OVERWORLD	= 2,
	MODE_MENU		= 3,
	MODE_LEVEL_SETTINGS	= 4
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

// no Visual Studio 8 and higher
#if defined( _MSC_VER ) && _MSC_VER < 1400
//	using std::find_if;
#endif

/* *** *** *** *** *** *** *** Paths *** *** *** *** *** *** *** *** *** *** */

// user data directory
extern string user_data_dir;

#ifndef HAVE_CONFIG_H
	// Core
	#define SAVE_DIR "savegames"
	#define SCREENSHOT_DIR "screenshots"
	#define OVERWORLD_DIR "data/world"
	#define LEVEL_DIR "data/levels"
	#define PIXMAPS_DIR "data/pixmaps"
	#define SOUNDS_DIR "data/sounds"
	#define MUSIC_DIR "data/music"
	#define EDITOR_DIR "data/editor"
	#define ICON_DIR "data/icon"
	#define SCHEMA_DIR "data/schema"
	// CEGUI
	#define GUI_SCHEME_DIR "data/gui/schemes"
	#define GUI_IMAGESET_DIR "data/gui/imagesets"
	#define GUI_FONT_DIR "data/gui/font"
	#define GUI_LAYOUT_DIR "data/gui/layout"
	#define GUI_LOOKNFEEL_DIR "data/gui/looknfeel"
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
#include "RendererModules/OpenGLGUIRenderer/openglrenderer.h"

/* *** *** *** *** *** *** *** XML *** *** *** *** *** *** *** *** *** *** */

#include "CEGUIXMLParser.h"
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

using namespace CEGUI;

/* *** *** *** *** *** *** *** Pre-declare *** *** *** *** *** *** *** *** *** *** */

// Allows use of pointers in header files without including individual headers
// and decreases dependencies between files

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
