/***************************************************************************
 * preferences.cpp  -  Game settings handler
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

#include "../user/preferences.h"
#include "../audio/audio.h"
#include "../video/video.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../input/joystick.h"
#include "../gui/hud.h"
#include "../level/level_manager.h"
#include "../core/i18n.h"
// boost filesystem
#include "boost/filesystem/convenience.hpp"
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
	
	// prefer local config file
	if( File_Exists( config_filename ) )
	{
		printf( "Using local preferences file : %s\n", config_filename.c_str() );
	}
	// user dir
	else
	{
		config_filename.insert( 0, user_data_dir );

		// does not exist in user dir
		if( !File_Exists( config_filename ) )
		{
			// only print warning if file is given
			if( !filename.empty() )
			{
				printf( "Couldn't open preferences file : %s\n", config_filename.c_str() );
			}
			return 0;
		}
	}

	try
	{
		CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, config_filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Config.xsd", "" );
	}
	// catch CEGUI Exceptions
	catch( CEGUI::Exception &ex )
	{
		printf( "Preferences Loading CEGUI Exception %s\n", ex.getMessage().c_str() );
		debugdisplay->Set_Text( _("Preferences Loading failed : ") + (string)ex.getMessage().c_str() );
	}

	// if user data dir is set
	if( !force_user_data_dir.empty() )
	{
		user_data_dir = force_user_data_dir;
	}

	return 1;
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
	file << "\t<Item Name=\"game_language\" Value=\"" << language << "\" />" << std::endl;
	file << "\t<Item Name=\"game_version\" Value=\"" << smc_version << "\" />" << std::endl;
	file << "\t<Item Name=\"user_data_dir\" Value=\"" << force_user_data_dir << "\" />" << std::endl;
	file << "\t<Item Name=\"camera_hor_speed\" Value=\"" << camera_hor_speed << "\" />" << std::endl;
	file << "\t<Item Name=\"camera_ver_speed\" Value=\"" << camera_ver_speed << "\" />" << std::endl;
	// Video
	file << "\t<Item Name=\"video_fullscreen\" Value=\"" << video_fullscreen << "\" />" << std::endl;
	file << "\t<Item Name=\"video_screen_w\" Value=\"" << video_screen_w << "\" />" << std::endl;
	file << "\t<Item Name=\"video_screen_h\" Value=\"" << video_screen_h << "\" />" << std::endl;
	file << "\t<Item Name=\"video_screen_bpp\" Value=\"" << static_cast<int>(video_screen_bpp) << "\" />" << std::endl;
	file << "\t<Item Name=\"video_vsync\" Value=\"" << video_vsync << "\" />" << std::endl;
	file << "\t<Item Name=\"video_geometry_detail\" Value=\"" << pVideo->geometry_detail << "\" />" << std::endl;
	file << "\t<Item Name=\"video_texture_detail\" Value=\"" << pVideo->texture_detail << "\" />" << std::endl;
	// Audio
	file << "\t<Item Name=\"audio_music\" Value=\"" << audio_music << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_sound\" Value=\"" << audio_sound << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_music_volume\" Value=\"" << static_cast<int>(pAudio->music_volume) << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_sound_volume\" Value=\"" << static_cast<int>(pAudio->sound_volume) << "\" />" << std::endl;
	file << "\t<Item Name=\"audio_hz\" Value=\"" << audio_hz << "\" />" << std::endl;
	// Keyboard
	file << "\t<Item Name=\"keyboard_key_up\" Value=\"" << key_up << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_down\" Value=\"" << key_down << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_left\" Value=\"" << key_left << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_right\" Value=\"" << key_right << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_jump\" Value=\"" << key_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_shoot\" Value=\"" << key_shoot << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_key_action\" Value=\"" << key_action << "\" />" << std::endl;
	file << "\t<Item Name=\"keyboard_scroll_speed\" Value=\"" << scroll_speed << "\" />" << std::endl;
	// Joystick/Gamepad
	file << "\t<Item Name=\"joy_enabled\" Value=\"" << joy_enabled << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_name\" Value=\"" << string_to_xml_string( joy_name ) << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_analog_jump\" Value=\"" << joy_analog_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_axis_hor\" Value=\"" << joy_axis_hor << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_axis_ver\" Value=\"" << joy_axis_ver << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_axis_threshold\" Value=\"" << joy_axis_threshold << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_jump\" Value=\"" << static_cast<int>(joy_button_jump) << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_item\" Value=\"" << static_cast<int>(joy_button_item) << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_shoot\" Value=\"" << static_cast<int>(joy_button_shoot) << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_action\" Value=\"" << static_cast<int>(joy_button_action) << "\" />" << std::endl;
	file << "\t<Item Name=\"joy_button_exit\" Value=\"" << static_cast<int>(joy_button_exit) << "\" />" << std::endl;
	// Special
	file << "\t<Item Name=\"always_run\" Value=\"" << always_run << "\" />" << std::endl;
	file << "\t<Item Name=\"level_background_images\" Value=\"" << level_background_images << "\" />" << std::endl;
	file << "\t<Item Name=\"image_cache_enabled\" Value=\"" << image_cache_enabled << "\" />" << std::endl;
	// Leveleditor
	file << "\t<Item Name=\"editor_mouse_auto_hide\" Value=\"" << editor_mouse_auto_hide << "\" />" << std::endl;
	// end preferences
	file << "</Preferences>" << std::endl;

	file.close();
}

void cPreferences :: Default( void )
{
	// Game
	language = "";
	force_user_data_dir.clear();
	game_version = smc_version;
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
	audio_hz = 44100;

	// Keyboard
	key_up = SDLK_UP;
	key_down = SDLK_DOWN;
	key_left = SDLK_LEFT;
	key_right = SDLK_RIGHT;
	key_jump = SDLK_LALT;
	key_shoot = SDLK_SPACE;
	key_action = SDLK_LCTRL;
	scroll_speed = 1;

	// Joystick
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
	always_run = 0;
	joy_name.clear();
	joy_analog_jump = 0;
	level_background_images = 1;
	image_cache_enabled = 1;

	// editor
	editor_mouse_auto_hide = 0;

	// filename
	config_filename = "config.xml";
}

void cPreferences :: Update( void )
{
	camera_hor_speed = pLevel_Manager->camera->hor_offset_speed;
	camera_ver_speed = pLevel_Manager->camera->ver_offset_speed;

	audio_music = pAudio->music_enabled;
	audio_sound = pAudio->sound_enabled;

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
	pLevel_Manager->camera->hor_offset_speed = camera_hor_speed;
	pLevel_Manager->camera->ver_offset_speed = camera_ver_speed;
	
	// disable joystick if the joystick initialization failed
	if( pVideo->joy_init_failed )
	{
		joy_enabled = 0;
	}
}

void cPreferences :: Apply_Video( Uint16 screen_w, Uint16 screen_h, Uint8 screen_bpp, bool fullscreen, bool vsync, float geometry_detail, float texture_detail )
{
	/* if resolution, bpp, vsync or texture detail changed
	 * a texture reload is necessary
	*/
	if( video_screen_w != screen_w || video_screen_h != screen_h || video_screen_bpp != screen_bpp || video_vsync != vsync || pVideo->texture_detail != texture_detail )
	{
		// new settings
		video_screen_w = screen_w;
		video_screen_h = screen_h;
		video_screen_bpp = screen_bpp;
		video_vsync = vsync;
		video_fullscreen = fullscreen;
		pVideo->texture_detail = texture_detail;
		pVideo->geometry_detail = geometry_detail;

		// reinitialize video and reload textures from file
		pVideo->Init_Video( 1 );
	}
	// no texture reload necessary
	else
	{
		// geometry detail changed
		if( pVideo->geometry_detail != geometry_detail )
		{
			pVideo->geometry_detail = geometry_detail;
			pVideo->Init_Geometry();
		}

		// fullscreen changed
		if( video_fullscreen != fullscreen )
		{
			// toggle fullscreen and switches video_fullscreen itself
			pVideo->Toggle_Fullscreen();
		}
	}
}

void cPreferences :: Apply_Audio( bool sound, bool music )
{
	// disable sound and music if the audio initialization failed
	if( pVideo->audio_init_failed )
	{
		audio_sound = 0;
		audio_music = 0;
		return;
	}

	audio_sound = sound;
	audio_music = music;

	// init audio settings
	pAudio->Init();
}

// XML element start
void cPreferences :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
    if( element == "Item" )
    {
        handle_item( attributes );
    }
}

// XML element end
void cPreferences :: elementEnd( const CEGUI::String &element )
{
	
}

void cPreferences :: handle_item( const CEGUI::XMLAttributes& attributes )
{
	string name = attributes.getValueAsString( "Name" ).c_str();

	// Game
	if( name.compare( "game_language" ) == 0 )
	{
		language = attributes.getValueAsString( "Value" ).c_str();
	}
	else if( name.compare( "game_version" ) == 0 )
	{
		game_version = attributes.getValueAsFloat( "Value" );
	}
	else if( name.compare( "user_data_dir" ) == 0 )
	{
		force_user_data_dir = attributes.getValueAsString( "Value" ).c_str();

		// if user data dir is set
		if( !force_user_data_dir.empty() ) 
		{
			Convert_Path_Separators( force_user_data_dir );

			// add trailing slash if missing
			if( *(force_user_data_dir.end() - 1) != '/' )
			{
				force_user_data_dir.insert( force_user_data_dir.length(), "/" );
			}
		}
	}
	else if( name.compare( "camera_hor_speed" ) == 0 )
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

		if( val < 200 )
		{
			val = 200;
		}
		else if( val > 2560 )
		{
			val = 2560;
		}
		
		video_screen_h = val;
	}
	else if( name.compare( "Screen_W" ) == 0 || name.compare( "video_screen_w" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val < 200 )
		{
			val = 200;
		}
		else if( val > 2560 )
		{
			val = 2560;
		}

		video_screen_w = val;
	}
	else if( name.compare( "Screen_Bpp" ) == 0 || name.compare( "video_screen_bpp" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val < 8 )
		{
			val = 8;
		}
		else if( val > 32 )
		{
			val = 32;
		}

		video_screen_bpp = val;
	}
	else if( name.compare( "vsync" ) == 0 || name.compare( "video_vsync" ) == 0 )
	{
		video_vsync = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "Fullscreen" ) == 0 || name.compare( "video_fullscreen" ) == 0 )
	{
		video_fullscreen = attributes.getValueAsBool( "Value" );
	}
	else if( name.compare( "video_geometry_detail" ) == 0 )
	{
		pVideo->geometry_detail = attributes.getValueAsFloat( "Value" );
	}
	else if( name.compare( "video_texture_detail" ) == 0 )
	{
		pVideo->texture_detail = attributes.getValueAsFloat( "Value" );
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
			pAudio->music_volume = val;
		}
	}
	else if( name.compare( "sound_volume" ) == 0 || name.compare( "audio_sound_volume" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= MIX_MAX_VOLUME )
		{
			pAudio->sound_volume = val;
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
	else if( name.compare( "Key_up" ) == 0 || name.compare( "keyboard_key_up" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_up = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "Key_down" ) == 0 || name.compare( "keyboard_key_down" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_down = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "Key_left" ) == 0 || name.compare( "keyboard_key_left" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_left = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "Key_right" ) == 0 || name.compare( "keyboard_key_right" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_right = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "Key_jump" ) == 0 || name.compare( "keyboard_key_jump" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_jump = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "Key_shoot" ) == 0 || name.compare( "keyboard_key_shoot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_shoot = static_cast<SDLKey>(val);
		}
	}
	else if( name.compare( "Key_action" ) == 0 || name.compare( "keyboard_key_action" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 1000 )
		{
			key_action = static_cast<SDLKey>(val);
		}
	}
	// scroll speed
	else if( name.compare( "keyboard_scroll_speed" ) == 0 )
	{
		scroll_speed = attributes.getValueAsFloat( "Value" );
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

		if( val >= 0 && val <= 256 )
		{
			joy_button_jump = val;
		}
	}
	else if( name.compare( "Joypad_item" ) == 0 || name.compare( "joy_button_item" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			joy_button_item = val;
		}
	}
	else if( name.compare( "Joypad_shoot" ) == 0 || name.compare( "joy_button_shoot" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			joy_button_shoot = val;
		}
	}
	else if( name.compare( "Joypad_action" ) == 0 || name.compare( "joy_button_action" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			joy_button_action = val;
		}
	}
	else if( name.compare( "Joypad_exit" ) == 0 || name.compare( "joy_button_exit" ) == 0 )
	{
		int val = attributes.getValueAsInteger( "Value" );

		if( val >= 0 && val <= 256 )
		{
			joy_button_exit = val;
		}
	}
	// Special
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
	else if( name.compare( "image_cache_enabled" ) == 0 )
	{
		image_cache_enabled = attributes.getValueAsBool( "Value" );
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cPreferences *pPreferences = NULL;
