/***************************************************************************
           preferences.cpp  -  Game Settings Handler :)
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../user/preferences.h"
#include "../audio/audio.h"
#include "../video/video.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../input/joystick.h"
#include "../gui/hud.h"
// boost filesystem
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
namespace fs = boost::filesystem;


/* *** *** *** *** *** *** *** cPreferences *** *** *** *** *** *** *** *** *** *** */

cPreferences :: cPreferences( void )
{
	Default();
}

cPreferences :: ~cPreferences( void )
{
	//
}

bool cPreferences :: Load( string filename /* = "" */ )
{
	Default();
	
	// if config file is given
	if( filename.length() )
	{
		config_filename = filename;
	}
	
	if( !valid_file( config_filename ) )
	{
		printf( "Couldn't open preferences file : %s\n", config_filename.c_str() );
		return 0;
	}

	try
	{
		System::getSingleton().getXMLParser()->parseXMLFile( *this, config_filename.c_str(), SCHEMA_DIR "/Config.xsd", "" );
	}
	// catch CEGUI Exceptions
	catch( Exception &ex )
	{
		printf( "Preferences Loading CEGUI Exception %s\n", ex.getMessage().c_str() );
		debugdisplay->Set_Text( "Preferences Loading failed : " + (string)ex.getMessage().c_str() );
	}

	return 1;
}

// XML element start
void cPreferences :: elementStart( const String &element, const XMLAttributes &attributes )
{
    if( element == "Item" )
    {
        handle_item( attributes );
    }
}

// XML element end
void cPreferences :: elementEnd( const String &element )
{
	
}

void cPreferences :: Save( void )
{
	Update(); 

	ofstream file( config_filename.c_str(), ios::out );

	if( !file.is_open() )
	{
		printf( "Error : couldn't open config file for saving. Is the file read-only ?" );
		return;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin preferences
	file << "<Preferences>" << std::endl;
	// Game 
	file << "\t<Item Name=\"camera_hor_speed\" Value=\"" << camera_hor_speed << "\" />" << std::endl;
	file << "\t<Item Name=\"camera_ver_speed\" Value=\"" << camera_ver_speed << "\" />" << std::endl;
	// Video
	file << "\t<Item Name=\"Fullscreen\" Value=\"" << Fullscreen << "\" />" << std::endl;
	file << "\t<Item Name=\"Screen_W\" Value=\"" << Screen_W << "\" />" << std::endl;
	file << "\t<Item Name=\"Screen_H\" Value=\"" << Screen_H << "\" />" << std::endl;
	file << "\t<Item Name=\"Screen_Bpp\" Value=\"" << (int)Screen_Bpp << "\" />" << std::endl;
	file << "\t<Item Name=\"vsync\" Value=\"" << vsync << "\" />" << std::endl;
	// Audio
	file << "\t<Item Name=\"Music\" Value=\"" << Music << "\" />" << std::endl;
	file << "\t<Item Name=\"Sounds\" Value=\"" << Sounds << "\" />" << std::endl;
	file << "\t<Item Name=\"music_volume\" Value=\"" << (int)music_volume << "\" />" << std::endl;
	file << "\t<Item Name=\"sound_volume\" Value=\"" << (int)sound_volume << "\" />" << std::endl;
	// Keyboard
	file << "\t<Item Name=\"Key_up\" Value=\"" << key_up << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_down\" Value=\"" << key_down << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_left\" Value=\"" << key_left << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_right\" Value=\"" << key_right << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_jump\" Value=\"" << key_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_shoot\" Value=\"" << key_shoot << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_action\" Value=\"" << key_action << "\" />" << std::endl;
	// Joystick
	file << "\t<Item Name=\"Joypad_enabled\" Value=\"" << joy_enabled << "\" />" << std::endl;
	file << "\t<Item Name=\"Joypad_jump\" Value=\"" << joy_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"Joypad_item\" Value=\"" << joy_item << "\" />" << std::endl;
	file << "\t<Item Name=\"Joypad_shoot\" Value=\"" << joy_shoot << "\" />" << std::endl;
	file << "\t<Item Name=\"Joypad_action\" Value=\"" << joy_action << "\" />" << std::endl;
	file << "\t<Item Name=\"Joypad_exit\" Value=\"" << joy_exit << "\" />" << std::endl;
	// Special
	file << "\t<Item Name=\"always_run\" Value=\"" << always_run << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_name\" Value=\"" << joy_name << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_analog_jump\" Value=\"" << joy_analog_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"background_images\" Value=\"" << background_images << "\" />" << std::endl;
	// Leveleditor
	file << "\t<Item Name=\"editor_mouse_auto_hide\" Value=\"" << editor_mouse_auto_hide << "\" />" << std::endl;
	// Directory
	file << "\t<Item Name=\"level_dir\" Value=\"" << level_dir << "\" />" << std::endl;
	// end preferences
	file << "</Preferences>" << std::endl;

	file.close();
}

void cPreferences :: Default( void )
{
	// Game
	camera_hor_speed = 0.3f;
	camera_ver_speed = 0.2f;

	// Video
	Screen_W = 1024;
	Screen_H = 768;
	Screen_Bpp = 32;
	/* disable by default because of possible bad drivers
	 * which can't handle visual sync
	*/
	vsync = 0;

#ifdef _DEBUG
	Fullscreen = 0;
#else
	Fullscreen = 1;
#endif

	// Audio
	Music = 1;
	Sounds = 1;
	music_volume = 100;
	sound_volume = 120;

	// Keyboard
	key_up = SDLK_UP;
	key_down = SDLK_DOWN;
	key_left = SDLK_LEFT;
	key_right = SDLK_RIGHT;
	key_jump = SDLK_LALT;
	key_shoot = SDLK_SPACE;
	key_action = SDLK_LCTRL;

	// Joypad
	joy_enabled = 1;
	joy_shoot = 1;
	joy_jump = 0;
	joy_item = 3;
	joy_action = 2;
	joy_exit = 4;

	// Special
	always_run = 0;
	joy_name.clear();
	joy_analog_jump = 0;
	background_images = 1;
	level_dir = LEVEL_DIR;

	// editor
	editor_mouse_auto_hide = 0;

	config_filename = user_data_dir + "config.xml";
}

void cPreferences :: Update( void )
{
	camera_hor_speed = pCamera->hor_offset_speed;
	camera_ver_speed = pCamera->ver_offset_speed;

	Music = pAudio->music_enabled;
	Sounds = pAudio->sound_enabled;
	music_volume = pAudio->music_volume;
	sound_volume = pAudio->sound_volume;

	// if not default joy used
	if( pJoystick->cur_stick > 0 )
	{
		joy_name = pJoystick->Get_Name();
	}
	// using default joy
	else
	{
		joy_name.clear();
	}
}

void cPreferences :: Apply( void )
{
	pCamera->hor_offset_speed = camera_hor_speed;
	pCamera->ver_offset_speed = camera_ver_speed;

	pAudio->music_volume = music_volume;
	pAudio->sound_volume = sound_volume;

	// disable sound and music if the audio initialization failed
	if( pVideo->audio_init_failed )
	{
		Sounds = 0;
		Music = 0;
	}
	
	// disable joystick if the joystick initialization failed
	if( pVideo->joy_init_failed )
	{
		joy_enabled = 0;
	}

	// init audio settings
	pAudio->Init( Sounds, Music );

	// reinit video
	pVideo->Init_Video();
}

void cPreferences :: handle_item( const XMLAttributes& attributes )
{
	string name = attributes.getValueAsString( "Name" ).c_str();

	// Game
	if( name.compare( "camera_hor_speed" ) == 0 )
	{
		camera_hor_speed = attributes.getValueAsFloat( "Value" );
	}
	else if( name.compare( "camera_ver_speed" ) == 0 )
	{
		camera_ver_speed = attributes.getValueAsFloat( "Value" );
	}
	// Video
	else if( name.compare( "Screen_H" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 200 || val <= 2560 )
		{
			Screen_H = val;
		}
	}
	else if( name.compare( "Screen_W" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 200 && val <= 2560 )
		{
			Screen_W = val;
		}
	}
	else if( name.compare( "Screen_Bpp" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 8 && val <= 32 )
		{
			Screen_Bpp = val;
		}
	}
	else if( name.compare( "vsync" ) == 0 )
	{
		vsync = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "Fullscreen" ) == 0 )
	{
		Fullscreen = attributes.getValueAsBool( "Value" );
	}
	// Audio
	else if( name.compare( "Music" ) == 0 )
	{
		Music = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "Sounds" ) == 0 )
	{
		Sounds = attributes.getValueAsBool( "Value" );
	}
	if( name.compare( "music_volume" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= MIX_MAX_VOLUME )
		{
			music_volume =  val;
		}
	}
	else if( name.compare( "sound_volume" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= MIX_MAX_VOLUME )
		{
			sound_volume = val;
		}
	}
	// Keyboard
	else if( name.compare( "Key_up" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_up = (SDLKey)val;
		}
	}
	else if( name.compare( "Key_down" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_down = (SDLKey)val;
		}
	}
	else if( name.compare( "Key_left" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_left = (SDLKey)val;
		}
	}
	else if( name.compare( "Key_right" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_right = (SDLKey)val;
		}
	}
	else if( name.compare( "Key_jump" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_jump = (SDLKey)val;
		}
	}
	else if( name.compare( "Key_shoot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_shoot = (SDLKey)val;
		}
	}
	else if( name.compare( "Key_action" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_action = (SDLKey)val;
		}
	}
	// Joypad
	else if( name.compare( "Joypad_enabled" ) == 0 )
	{
		joy_enabled = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "Joypad_jump" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 50 )
		{
			joy_jump = val;
		}
	}
	else if( name.compare( "Joypad_item" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 50 )
		{
			joy_item = val;
		}
	}
	else if( name.compare( "Joypad_shoot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 50 )
		{
			joy_shoot = val;
		}
	}
	else if( name.compare( "Joypad_action" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 50 )
		{
			joy_action = val;
		}
	}
	else if( name.compare( "Joypad_exit" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 50 )
		{
			joy_exit = val;
		}
	}
	// Special
	else if( name.compare( "always_run" ) == 0 )
	{
		always_run = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "joy_name" ) == 0 )
	{
		joy_name = attributes.getValueAsString( "Value" ).c_str();
	}
	else if( name.compare( "joy_analog_jump" ) == 0 )
	{
		joy_analog_jump = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "background_images" ) == 0 )
	{
		background_images = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "editor_mouse_auto_hide" ) == 0 )
	{
		editor_mouse_auto_hide = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "level_dir" ) == 0 )
	{
		string val = attributes.getValueAsString( "Value" ).c_str();

		// if directory is not valid
		if( !fs::exists( val ) )
		{
			printf( "Warning : Preferences level directory %s doesn't exists\n", val.c_str() );
		}
		
		level_dir = val;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPreferences *pPreferences = NULL;
