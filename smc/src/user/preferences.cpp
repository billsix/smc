/***************************************************************************
           preferences.cpp  -  Game Settings Handler :)
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../user/preferences.h"
#include "../audio/audio.h"
#include "../video/video.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../input/joystick.h"
#include "../gui/hud.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
namespace fs = boost::filesystem;


/* *** *** *** *** *** *** *** cPreferences *** *** *** *** *** *** *** *** *** *** */

cPreferences :: cPreferences( void )
{
	// Get user directory
	user_data_dir = Get_User_Directory();

	// Create user directory
	if( !fs::exists( fs::path( user_data_dir, fs::native ) ) )
	{
		// first run if not available
		fs::create_directory( fs::path( user_data_dir, fs::native ) );
	}
	// Create savegame directory
	if( !fs::exists( fs::path( user_data_dir + USER_SAVEGAME_DIR, fs::native ) ) )
	{
		fs::create_directory( fs::path( user_data_dir + USER_SAVEGAME_DIR, fs::native ) );
	}
	// Create screenshot directory
	if( !fs::exists( fs::path( user_data_dir + USER_SCREENSHOT_DIR, fs::native ) ) )
	{
		fs::create_directory( fs::path( user_data_dir + USER_SCREENSHOT_DIR, fs::native ) );
	}
	// Create level directory
	if( !fs::exists( fs::path( user_data_dir + USER_LEVEL_DIR, fs::native ) ) )
	{
		fs::create_directory( fs::path( user_data_dir + USER_LEVEL_DIR, fs::native ) );
	}
	// Create world directory
	if( !fs::exists( fs::path( user_data_dir + USER_WORLD_DIR, fs::native ) ) )
	{
		fs::create_directory( fs::path( user_data_dir + USER_WORLD_DIR, fs::native ) );
	}
	// Create cache directory
	if( !fs::exists( fs::path( user_data_dir + USER_IMGCACHE_DIR, fs::native ) ) )
	{
		fs::create_directory( fs::path( user_data_dir + USER_IMGCACHE_DIR, fs::native ) );
	}

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
	
	if( !file_exists( config_filename ) )
	{
		printf( "Couldn't open preferences file : %s\n", config_filename.c_str() );
		return 0;
	}

	try
	{
		System::getSingleton().getXMLParser()->parseXMLFile( *this, config_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Config.xsd", "" );
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
	file << "\t<Item Name=\"video_fullscreen\" Value=\"" << video_fullscreen << "\" />" << std::endl;
	file << "\t<Item Name=\"video_screen_w\" Value=\"" << video_screen_w << "\" />" << std::endl;
	file << "\t<Item Name=\"video_screen_h\" Value=\"" << video_screen_h << "\" />" << std::endl;
	file << "\t<Item Name=\"video_screen_bpp\" Value=\"" << static_cast<int>(video_screen_bpp) << "\" />" << std::endl;
	file << "\t<Item Name=\"video_vsync\" Value=\"" << video_vsync << "\" />" << std::endl;
	// Audio
	file << "\t<Item Name=\"audio_music\" Value=\"" << audio_music << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_sound\" Value=\"" << audio_sound << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_music_volume\" Value=\"" << static_cast<int>(audio_music_volume) << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_sound_volume\" Value=\"" << static_cast<int>(audio_sound_volume) << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_hz\" Value=\"" << audio_hz << "\" />" << std::endl;
	// Keyboard
	file << "\t<Item Name=\"Key_up\" Value=\"" << key_up << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_down\" Value=\"" << key_down << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_left\" Value=\"" << key_left << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_right\" Value=\"" << key_right << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_jump\" Value=\"" << key_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_shoot\" Value=\"" << key_shoot << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_action\" Value=\"" << key_action << "\" />" << std::endl;
	// Joystick/Gamepad
	file << "\t<Item Name=\"joy_enabled\" Value=\"" << joy_enabled << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_name\" Value=\"" << joy_name << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_analog_jump\" Value=\"" << joy_analog_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_axis_hor\" Value=\"" << joy_axis_hor << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_axis_ver\" Value=\"" << joy_axis_ver << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_axis_threshold\" Value=\"" << joy_axis_threshold << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_jump\" Value=\"" << joy_button_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_item\" Value=\"" << joy_button_item << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_shoot\" Value=\"" << joy_button_shoot << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_action\" Value=\"" << joy_button_action << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_exit\" Value=\"" << joy_button_exit << "\" />" << std::endl;
	// Special
	file << "\t<Item Name=\"game_version\" Value=\"" << VERSION << "\" />" << std::endl;
	file << "\t<Item Name=\"always_run\" Value=\"" << always_run << "\" />" << std::endl;
	file << "\t<Item Name=\"level_background_images\" Value=\"" << level_background_images << "\" />" << std::endl;
	// Leveleditor
	file << "\t<Item Name=\"editor_mouse_auto_hide\" Value=\"" << editor_mouse_auto_hide << "\" />" << std::endl;
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
	video_screen_w = 1024;
	video_screen_h = 768;
	video_screen_bpp = 32;
	/* disable by default because of possible bad drivers
	 * which can't handle visual sync
	*/
	video_vsync = 0;

#ifdef _DEBUG
	video_fullscreen = 0;
#else
	video_fullscreen = 1;
#endif

	// Audio
	audio_music = 1;
	audio_sound = 1;
	audio_music_volume = 100;
	audio_sound_volume = 120;
	audio_hz = 44100;

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
	// axes
	joy_axis_hor = 0;
	joy_axis_ver = 1;
	// axis threshold
	joy_axis_threshold = 10000;
	// buttons
	joy_button_jump = 0;
	joy_button_shoot = 1;
	joy_button_item = 3;
	joy_button_action = 2;
	joy_button_exit = 4;

	// Special
	game_version = VERSION;
	always_run = 0;
	joy_name.clear();
	joy_analog_jump = 0;
	level_background_images = 1;

	// editor
	editor_mouse_auto_hide = 0;

	config_filename = user_data_dir + "config.xml";
}

void cPreferences :: Update( void )
{
	camera_hor_speed = pCamera->hor_offset_speed;
	camera_ver_speed = pCamera->ver_offset_speed;

	audio_music = pAudio->music_enabled;
	audio_sound = pAudio->sound_enabled;
	audio_music_volume = pAudio->music_volume;
	audio_sound_volume = pAudio->sound_volume;

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

	pAudio->music_volume = audio_music_volume;
	pAudio->sound_volume = audio_sound_volume;

	// disable sound and music if the audio initialization failed
	if( pVideo->audio_init_failed )
	{
		audio_sound = 0;
		audio_music = 0;
	}
	
	// disable joystick if the joystick initialization failed
	if( pVideo->joy_init_failed )
	{
		joy_enabled = 0;
	}

	// init audio settings
	pAudio->Init( audio_sound, audio_music );

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
	else if( name.compare( "Screen_H" ) == 0 || name.compare( "video_screen_h" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 200 || val <= 2560 )
		{
			video_screen_h = val;
		}
	}
	else if( name.compare( "Screen_W" ) == 0 || name.compare( "video_screen_w" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 200 && val <= 2560 )
		{
			video_screen_w = val;
		}
	}
	else if( name.compare( "Screen_Bpp" ) == 0 || name.compare( "video_screen_bpp" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 8 && val <= 32 )
		{
			video_screen_bpp = val;
		}
	}
	else if( name.compare( "vsync" ) == 0 || name.compare( "video_vsync" ) == 0 )
	{
		video_vsync = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "Fullscreen" ) == 0 || name.compare( "video_fullscreen" ) == 0 )
	{
		video_fullscreen = attributes.getValueAsBool( "Value" );
	}
	// Audio
	else if( name.compare( "Music" ) == 0 || name.compare( "audio_music" ) == 0 )
	{
		audio_music = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "Sounds" ) == 0 || name.compare( "audio_sound" ) == 0 )
	{
		audio_sound = attributes.getValueAsBool( "Value" );
	}
	if( name.compare( "music_volume" ) == 0 || name.compare( "audio_music_volume" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= MIX_MAX_VOLUME )
		{
			audio_music_volume =  val;
		}
	}
	else if( name.compare( "sound_volume" ) == 0 || name.compare( "audio_sound_volume" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= MIX_MAX_VOLUME )
		{
			audio_sound_volume = val;
		}
	}
	else if( name.compare( "audio_hz" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 44800 )
		{
			audio_hz = val;
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
	else if( name.compare( "Joypad_enabled" ) == 0 || name.compare( "joy_enabled" ) == 0 )
	{
		joy_enabled = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "joy_name" ) == 0 )
	{
		joy_name = attributes.getValueAsString( "Value" ).c_str();
	}
	else if( name.compare( "joy_analog_jump" ) == 0 )
	{
		joy_analog_jump = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "joy_axis_hor" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 15 )
		{
			joy_axis_hor = val;
		}
	}
	else if( name.compare( "joy_axis_ver" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 15 )
		{
			joy_axis_ver = val;
		}
	}
	else if( name.compare( "joy_axis_threshold" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 32000 )
		{
			joy_axis_threshold = val;
		}
	}
	else if( name.compare( "Joypad_jump" ) == 0 || name.compare( "joy_button_jump" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 50 )
		{
			joy_button_jump = val;
		}
	}
	else if( name.compare( "Joypad_item" ) == 0 || name.compare( "joy_button_item" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 50 )
		{
			joy_button_item = val;
		}
	}
	else if( name.compare( "Joypad_shoot" ) == 0 || name.compare( "joy_button_shoot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 50 )
		{
			joy_button_shoot = val;
		}
	}
	else if( name.compare( "Joypad_action" ) == 0 || name.compare( "joy_button_action" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 50 )
		{
			joy_button_action = val;
		}
	}
	else if( name.compare( "Joypad_exit" ) == 0 || name.compare( "joy_button_exit" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 50 )
		{
			joy_button_exit = val;
		}
	}
	// Special
	else if( name.compare( "game_version" ) == 0 )
	{
		game_version = attributes.getValueAsString( "Value" ).c_str();
	}
	else if( name.compare( "always_run" ) == 0 )
	{
		always_run = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "background_images" ) == 0 || name.compare( "level_background_images" ) == 0 )
	{
		level_background_images = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "editor_mouse_auto_hide" ) == 0 )
	{
		editor_mouse_auto_hide = attributes.getValueAsBool( "Value" );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPreferences *pPreferences = NULL;
