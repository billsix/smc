/***************************************************************************
           preferences.cpp  -  Preferences Loader :)
                             -------------------
    copyright            : (C) 2003 - 2006 by FluXy
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "../globals.h"


// ##################### Public #########################

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

	System::getSingleton().getXMLParser()->parseXMLFile( *this, config_filename, "data/schemes/ConfigScheme.xsd", "" );

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
	// Screen
	file << "\t<Item Name=\"Fullscreen\" Value=\"" << Fullscreen << "\" />" << std::endl;
	file << "\t<Item Name=\"Screen_W\" Value=\"" << Screen_W << "\" />" << std::endl;
	file << "\t<Item Name=\"Screen_H\" Value=\"" << Screen_H << "\" />" << std::endl;
	file << "\t<Item Name=\"Screen_Bpp\" Value=\"" << (int)Screen_Bpp << "\" />" << std::endl;
	// Audio
	file << "\t<Item Name=\"Music\" Value=\"" << Music << "\" />" << std::endl;
	file << "\t<Item Name=\"Sounds\" Value=\"" << Sounds << "\" />" << std::endl;
	file << "\t<Item Name=\"music_volume\" Value=\"" << (int)music_volume << "\" />" << std::endl;
	file << "\t<Item Name=\"sound_volume\" Value=\"" << (int)sound_volume << "\" />" << std::endl;
	// Keyboard
	file << "\t<Item Name=\"Key_up\" Value=\"" << Key_up << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_down\" Value=\"" << Key_down << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_left\" Value=\"" << Key_left << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_right\" Value=\"" << Key_right << "\" />" << std::endl;
	file << "\t<Item Name=\"Key_shoot\" Value=\"" << Key_shoot << "\" />" << std::endl;
	// Joystick
	file << "\t<Item Name=\"Joypad_enabled\" Value=\"" << Joy_enabled << "\" />" << std::endl;
	file << "\t<Item Name=\"Joypad_jump\" Value=\"" << Joy_jump << "\" />" << std::endl;
	file << "\t<Item Name=\"Joypad_item\" Value=\"" << Joy_item << "\" />" << std::endl;
	file << "\t<Item Name=\"Joypad_shoot\" Value=\"" << Joy_shoot << "\" />" << std::endl;
	file << "\t<Item Name=\"Joypad_exit\" Value=\"" << Joy_exit << "\" />" << std::endl;
	// Special
	file << "\t<Item Name=\"Background_images_disabled\" Value=\"" << Backgroundimages_disabled << "\" />" << std::endl;
	// Leveleditor
	file << "\t<Item Name=\"Hide_Mousecursor_automatically\" Value=\"" << Lvleditor_automousehide << "\" />" << std::endl;
	// Directory
	file << "\t<Item Name=\"level_dir\" Value=\"" << level_dir << "\" />" << std::endl;
	// end preferences
	file << "</Preferences>" << std::endl;

	file.close();
}

void cPreferences :: Default( void )
{
	Screen_W = 1024;
	Screen_H = 768;
	Screen_Bpp = 32;

#ifdef _DEBUG
	Fullscreen = 0;
#else
	Fullscreen = 1;
#endif

	Music = 1;
	Sounds = 1;
	music_volume = 100;
	sound_volume = 120;

	Key_up = SDLK_UP;
	Key_down = SDLK_DOWN;
	Key_left = SDLK_LEFT;
	Key_right = SDLK_RIGHT;
	Key_shoot = SDLK_SPACE;

	Joy_enabled = 1;
	Joy_shoot = 1;
	Joy_jump = 0;
	Joy_item = 3;
	Joy_exit = 2;

	Lvleditor_automousehide = 0;
	Backgroundimages_disabled = 0;

	level_dir = "data/levels";

	config_filename = "config.xml";
}

void cPreferences :: Update( void )
{
	Music = pAudio->music_enabled;
	Sounds = pAudio->sound_enabled;
	music_volume = pAudio->music_volume;
	sound_volume = pAudio->sound_volume;
}

void cPreferences :: Apply( void )
{
	pAudio->music_enabled = Music;
	pAudio->sound_enabled = Sounds;
	pAudio->music_volume = music_volume;
	pAudio->sound_volume = sound_volume;
}

// ##################### Private #########################

void cPreferences :: handle_item( const XMLAttributes& attributes )
{
	string name = attributes.getValueAsString( "Name" ).c_str();
	string value = attributes.getValueAsString( "Value" ).c_str();

	if( name.compare( "Music" ) == 0 )
	{
		if( string_to_int( value ) == 0 )
		{
			Music = 0;
		}
		else
		{
			Music = 1;
		}
	}
	else if( name.compare( "Sounds" ) == 0 )
	{
		if( string_to_int( value ) == 0 )
		{
			Sounds = 0;
		}
		else
		{
			Sounds = 1;
		}
	}
	if( name.compare( "music_volume" ) == 0 )
	{
		if( string_to_int( value ) >= 0 && string_to_int( value ) <= MIX_MAX_VOLUME )
		{
			music_volume = string_to_int( value );
		}
	}
	else if( name.compare( "sound_volume" ) == 0 )
	{
		if( string_to_int( value ) >= 0 && string_to_int( value ) <= MIX_MAX_VOLUME )
		{
			sound_volume = string_to_int( value );
		}
	}
	else if( name.compare( "Fullscreen" ) == 0 )
	{
		if( string_to_int( value ) == 0 )
		{
			Fullscreen = 0;
		}
		else
		{
			Fullscreen = 1;
		}
	}
	else if( name.compare( "Joypad_enabled" ) == 0 )
	{
		if( string_to_int( value ) == 0 )
		{
			Joy_enabled = 0;
		}
		else
		{
			Joy_enabled = 1;
		}
	}
	else if( name.compare( "Screen_H" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 1600 )
		{
			//return 0;
		}
		else
		{
			Screen_H = string_to_int( value );
		}
	}
	else if( name.compare( "Screen_W" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 1600 )
		{
			//return 0;
		}
		else
		{
			Screen_W = string_to_int( value );
		}
	}
	else if( name.compare( "Screen_Bpp" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 32 )
		{
			//return 0;
		}
		else
		{
			Screen_Bpp = string_to_int( value );
		}
	}
	else if( name.compare( "Key_up" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 1000 )
		{
			//return 0;
		}
		else
		{
			Key_up = (SDLKey)string_to_int( value );
		}
	}
	else if( name.compare( "Key_down" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 1000 )
		{
			//return 0;
		}
		else
		{
			Key_down = (SDLKey)string_to_int( value );
		}
	}
	else if( name.compare( "Key_left" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 1000 )
		{
			//return 0;
		}
		else
		{
			Key_left = (SDLKey)string_to_int( value );
		}
	}
	else if( name.compare( "Key_right" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 1000 )
		{
			//return 0;
		}
		else
		{
			Key_right = (SDLKey)string_to_int( value );
		}
	}
	else if( name.compare( "Key_shoot" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 1000 )
		{
			//return 0;
		}
		else
		{
			Key_shoot = (SDLKey)string_to_int( value );
		}
	}
	else if( name.compare( "Joypad_jump" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 50 )
		{
			//return 0;
		}
		else
		{
			Joy_jump = string_to_int( value );
		}
	}
	else if( name.compare( "Joypad_item" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 50 )
		{
			//return 0;
		}
		else
		{
			Joy_item = string_to_int( value );
		}
	}
	else if( name.compare( "Joypad_shoot" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 50 )
		{
			//return 0;
		}
		else
		{
			Joy_shoot = string_to_int( value );
		}
	}
	else if( name.compare( "Joypad_exit" ) == 0 )
	{
		if( string_to_int( value ) < 0 || string_to_int( value ) > 50 )
		{
			//return 0;
		}
		else
		{
			Joy_exit = string_to_int( value );
		}
	}
	else if( name.compare( "Background_images_disabled" ) == 0 )
	{
		if( string_to_int( value ) == 0 )
		{
			Backgroundimages_disabled = 0;
		}
		else
		{
			Backgroundimages_disabled = 1;
		}
	}
	else if( name.compare( "Hide_Mousecursor_automatically" ) == 0 )
	{
		if( string_to_int( value ) == 0 )
		{
			Lvleditor_automousehide = 0;
		}
		else
		{
			Lvleditor_automousehide = 1;
		}
	}
	else if( name.compare( "level_dir" ) == 0 )
	{
		if( value.length() > 2 )
		{
			// todo : check if the directory exists
			level_dir = value;
		}
	}
}
