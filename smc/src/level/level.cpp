/***************************************************************************
 * level.cpp  -  level handling class
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

#include "../level/level.h"
#include "../level/level_editor.h"
#include "../core/game_core.h"
#include "../core/main.h"
#include "../core/camera.h"
#include "../gui/hud.h"
#include "../gui/menu.h"
#include "../user/preferences.h"
#include "../audio/audio.h"
#include "../audio/random_sound.h"
#include "../core/framerate.h"
#include "../player/player.h"
#include "../core/obj_manager.h"
#include "../objects/goldpiece.h"
#include "../objects/level_exit.h"
#include "../objects/level_entry.h"
#include "../video/font.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../user/savegame.h"
#include "../overworld/world_manager.h"
#include "../overworld/overworld.h"
#include "../enemies/turtle.h"
#include "../enemies/bosses/turtle_boss.h"
#include "../enemies/rokko.h"
#include "../enemies/krush.h"
#include "../enemies/furball.h"
#include "../enemies/jpiranha.h"
#include "../enemies/thromp.h"
#include "../enemies/eato.h"
#include "../enemies/gee.h"
#include "../enemies/spika.h"
#include "../enemies/static.h"
#include "../objects/powerup.h"
#include "../objects/enemystopper.h"
#include "../objects/spinbox.h"
#include "../objects/bonusbox.h"
#include "../objects/text_box.h"
#include "../objects/moving_platform.h"
#include "../video/renderer.h"
#include "../video/gl_surface.h"
#include "../core/sprite_manager.h"
#include "../video/animation.h"
#include "../core/math/utilities.h"
#include "../core/i18n.h"


/* *** *** *** *** *** cLevel *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel :: cLevel( void )
: cFile_parser()
{
	engine_version = -1; // no version
	last_saved = 0;

	musicfile.clear();
	valid_music = 0;

	levelfile.clear();
	next_levelfile.clear();

	author.clear();
	version.clear();
	delayed_unload = 0;

	background_manager = new cBackground_Manager();
	pGlobal_effect = new cGlobal_effect();
	pSprite_Manager = new cSprite_Manager();

	// add default gradient layer
	cBackground *gradient_background = new cBackground();
	gradient_background->Set_Type( BG_GR_VER );
	gradient_background->posz = 0.0001f;
	background_manager->Add( gradient_background );
}

cLevel :: ~cLevel( void )
{
	Unload();

	// delete
	delete background_manager;
	delete pGlobal_effect;
	delete pSprite_Manager;
}

bool cLevel :: New( string filename )
{
	Unload();

	ifstream ifs;

	// if no name is given create name
	if( filename.empty() )
	{
		unsigned int i = 1;

		// search for a not existing file
		while( 1 )
		{
			// set name
			filename = user_data_dir + USER_LEVEL_DIR + "/new_" + int_to_string( i ) + ".smclvl";
			// try to open the file
			ifs.open( filename.c_str(), ios::in );

			// found unused name
			if( !ifs.is_open() )
			{
				break;
			}

			// stop on 99
			if( i > 99 )
			{
				return 0;
			}
			
			ifs.close();
			i++;
		}
	}
	else
	{
		// set file type
		if( filename.find( ".smclvl" ) == string::npos )
		{
			filename.insert( filename.length(), ".smclvl" );
		}

		// set user directory
		if( filename.find( user_data_dir + USER_LEVEL_DIR + "/" ) == string::npos )
		{
			filename.insert( 0, user_data_dir + USER_LEVEL_DIR + "/" );
		}
	}

	// open file
	ifs.open( filename.c_str(), ios::in );

	// Level doesn't exist
	if( !ifs )
	{
		// set filename
		levelfile = filename;
		data_file = Get_Filename( filename, 0, 1 );

		engine_version = level_engine_version;

		// set default music
		musicfile = DATA_DIR "/" GAME_MUSIC_DIR "/" LEVEL_DEFAULT_MUSIC;
		valid_music = File_Exists( musicfile );

		return 1;
	}

	// Level already exists
	ifs.close();
	return 0;
}

bool cLevel :: Load( string filename, bool delayed /* = 0 */ )
{
	next_levelfile.clear();

	if( !Get_Path( filename ) )
	{
		// show error without directory and file type
		printf( "Couldn't load level : %s\n", Get_Filename( filename, 0, 0 ).c_str() );
		return 0;
	}

	if( delayed )
	{
		next_levelfile = filename;
		return 1;
	}

	Unload();

	levelfile = filename;

	// set default music
	musicfile = DATA_DIR "/" GAME_MUSIC_DIR "/" LEVEL_DEFAULT_MUSIC;
	valid_music = File_Exists( musicfile );

	// new level format
	if( filename.rfind( ".smclvl" ) != string::npos )
	{
		try
		{
			CEGUI::System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/Level.xsd", "" );
		}
		// catch CEGUI Exceptions
		catch( CEGUI::Exception &ex )
		{
			printf( "Loading Level %s CEGUI Exception %s\n", filename.c_str(), ex.getMessage().c_str() );
			debugdisplay->Set_Text( _("Loading Level failed : ") + (string)ex.getMessage().c_str() );
			return 0;
		}

		// set parser filename for compatibility
		data_file = filename;
	}
	// parse old level format
	else
	{
		Parse( filename );

		// always use default direction
		pPlayer->Set_Direction( DIR_RIGHT, 1 );
	}

	// hack : save current camera position
	float cam_x = pActive_Camera->x;
	float cam_y = pActive_Camera->y;

	// hack : use level camera position for global effect initialization
	pActive_Camera->Center();

	// init global effect
	pGlobal_effect->Init_Anim();
	
	// hack : set back old position
	pActive_Camera->Set_Pos( cam_x, cam_y );

	// engine version entry not set
	if( engine_version < 0 )
	{
		engine_version = 0;
	}

	return 1;
}

void cLevel :: Unload( bool delayed /* = 0 */ )
{
	if( delayed )
	{
		delayed_unload = 1;
		return;
	}
	else
	{
		delayed_unload = 0;
	}

	// not loaded
	if( !Is_Loaded() )
	{
		return;
	}

	// delete backgrounds
	background_manager->Delete_All();

	// add default gradient layer
	cBackground *gradient_background = new cBackground();
	gradient_background->Set_Type( BG_GR_VER );
	gradient_background->posz = 0.0001f;
	background_manager->Add( gradient_background );

	// reset music
	musicfile.clear();

	valid_music = 0;

	author.clear();
	version.clear();

	// reset camera limits
	pLevel_Manager->camera->Reset_Limits();

	// clear global effect
	pGlobal_effect->Clear();

	// reset time display
	if( timedisplay )
	{
		timedisplay->counter = -1;
	}
	// reset mouse
	pMouseCursor->Reset( 0 );
	// reset player
	pPlayer->Reset( 0 );

	// no version
	engine_version = -1;

	levelfile.clear();

	// delete sprites at last
	pSprite_Manager->Delete_All();
}

void cLevel :: Save( void )
{
	pAudio->Play_Sound( "editor/save.ogg" );

	// check if old filename ending
	if( levelfile.rfind( ".txt" ) != string::npos )
	{
		levelfile.erase( levelfile.rfind( ".txt" ) );
		levelfile.insert( levelfile.length(), ".smclvl" );
	}

	// use user level dir
	if( levelfile.find( user_data_dir + USER_LEVEL_DIR + "/" ) == string::npos )
	{
		// erase old directory
		levelfile = Get_Filename( levelfile, 0, 1 );
		// set user directory
		levelfile.insert( 0, user_data_dir + USER_LEVEL_DIR + "/" );
	}

	ofstream file( levelfile.c_str(), ios::out | ios::trunc );

	if( !file )
	{
		debugdisplay->Set_Text( _("Couldn't save level ") + levelfile );
		return;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin level
	file << "<level>" << std::endl;

	// begin info
	file << "\t<information>" << std::endl;
		// game version
		file << "\t\t<Property name=\"game_version\" value=\"" << smc_version << "\" />" << std::endl;
		// engine version
		file << "\t\t<Property name=\"engine_version\" value=\"" << level_engine_version << "\" />" << std::endl;
		// time ( seconds since 1970 )
		file << "\t\t<Property name=\"save_time\" value=\"" << time( NULL ) << "\" />" << std::endl;
	// end info
	file << "\t</information>" << std::endl;

	// begin settings
	file << "\t<settings>" << std::endl;
		// level author
		file << "\t\t<Property name=\"lvl_author\" value=\"" << string_to_xml_string( author ) << "\" />" << std::endl;
		// level version
		file << "\t\t<Property name=\"lvl_version\" value=\"" << string_to_xml_string( version ) << "\" />" << std::endl;
		// music
		string music_file = Get_Musicfile( 1 );
		file << "\t\t<Property name=\"lvl_music\" value=\"" << string_to_xml_string( music_file ) << "\" />" << std::endl;
		// camera limits
		file << "\t\t<Property name=\"cam_limit_x\" value=\"" << static_cast<int>(pLevel_Manager->camera->limit_rect.x) << "\" />" << std::endl;
		file << "\t\t<Property name=\"cam_limit_y\" value=\"" << static_cast<int>(pLevel_Manager->camera->limit_rect.y) << "\" />" << std::endl;
		file << "\t\t<Property name=\"cam_limit_w\" value=\"" << static_cast<int>(pLevel_Manager->camera->limit_rect.w) << "\" />" << std::endl;
		file << "\t\t<Property name=\"cam_limit_h\" value=\"" << static_cast<int>(pLevel_Manager->camera->limit_rect.h) << "\" />" << std::endl;
		// fixed camera horizontal velocity
		file << "\t\t<Property name=\"cam_fixed_hor_vel\" value=\"" << pLevel_Manager->camera->fixed_hor_vel << "\" />" << std::endl;
	// end settings
	file << "\t</settings>" << std::endl;

	// backgrounds
	for( vector<cBackground *>::iterator itr = background_manager->objects.begin(), itr_end = background_manager->objects.end(); itr != itr_end; ++itr )
	{
		(*itr)->Save_to_Stream( file );
	}
	// global effect
	pGlobal_effect->Save_to_Stream( file );
	// player
	pPlayer->Save_to_Stream( file );
	// objects
	for( SpriteList::iterator itr = pSprite_Manager->objects.begin(), itr_end = pSprite_Manager->objects.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		// skip spawned and destroyed objects
		if( obj->spawned || obj->destroy )
		{
			continue;
		}

		// save to file stream
		obj->Save_to_Stream( file );
	}

	// end level
	file << "</level>" << std::endl;
	file.close();

	debugdisplay->Set_Text( _("Level ") + Get_Filename( data_file, 0, 0 ) + _(" saved") );
}

void cLevel :: Delete( void )
{
	Delete_File( levelfile );

	Unload();
}

void cLevel :: Update( void )
{
	if( delayed_unload )
	{
		Unload();
		return;
	}

	if( !next_levelfile.empty() )
	{
		Load( next_levelfile );
	}

	// if leveleditor not active
	if( !editor_level_enabled )
	{
		// backgrounds
		for( vector<cBackground *>::iterator itr = background_manager->objects.begin(), itr_end = background_manager->objects.end(); itr != itr_end; ++itr )
		{
			(*itr)->Update();
		}

		// objects
		pSprite_Manager->Update_Items();

		// animations
		pAnimation_Manager->Update();

		// global effect
		pGlobal_effect->Update();
	}
}

void cLevel :: Draw_Layer_1( LevelDrawType type /* = LVL_DRAW */ )
{
	// with background
	if( type != LVL_DRAW_NO_BG )
	{
		for( vector<cBackground *>::iterator itr = background_manager->objects.begin(), itr_end = background_manager->objects.end(); itr != itr_end; ++itr )
		{
			(*itr)->Draw();
		}
	}

	// only background
	if( type == LVL_DRAW_BG )
	{
		return;
	}

	// Objects
	pSprite_Manager->Draw_Items();
	// Animations
	pAnimation_Manager->Draw();
}

void cLevel :: Draw_Layer_2( LevelDrawType type /* = LVL_DRAW */ )
{
	// only background
	if( type == LVL_DRAW_BG )
	{
		return;
	}

	// global effect
	if( !editor_level_enabled )
	{
		pGlobal_effect->Draw();
	}

	// ghost
	if( pPlayer->maryo_type == MARYO_GHOST )
	{
		// create request
		cRectRequest *request = new cRectRequest();

		Color color = Color( 0.5f, 0.5f, 0.5f, 0.3f );
		pVideo->Draw_Rect( 0, 0, static_cast<float>(game_res_w), static_cast<float>(game_res_h), 0.12f, &color, request );

		request->combine_type = GL_MODULATE;

		request->combine_col[0] = 0.9f;
		request->combine_col[1] = 0.6f;
		request->combine_col[2] = 0.8f;


		// add request
		pRenderer->Add( request );
	}
}

void cLevel :: Process_Input( void )
{
	// only non editor
	if( !editor_level_enabled )
	{
		// none
	}
}

bool cLevel :: Key_Down( SDLKey key )
{
	// debug key F2
	if( key == SDLK_F2 && game_debug && !editor_level_enabled )
	{
		pPlayer->Set_Type( MARYO_CAPE, 0 );
	}
	// debug key F3
	else if( key == SDLK_F3 && !editor_level_enabled )
	{
		//pPlayer->GotoNextLevel();
		//DrawEffect( HORIZONTAL_VERTICAL_FADE );
		//pPlayer->Draw_Animation( MARYO_FIRE );

		cParticle_Emitter *anim = new cParticle_Emitter();
		anim->Set_Emitter_Rect( pPlayer->posx + static_cast<float>( pPlayer->col_rect.w / 2 ), pPlayer->posy - 100, 10, 10 );
		anim->Set_Emitter_Time_to_Live( -1 );
		anim->Set_Emitter_Iteration_Interval( 5 );
		anim->Set_Quota( 200 );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/star.png" ) );
		anim->Set_Time_to_Live( 3, 3 );
		anim->Set_Fading_Alpha( 1 );
		anim->Set_Speed( 1, 4 );
		anim->Set_Scale( 0.5f );
		anim->Set_Const_Rotation_Z( -5, 10 );
		pAnimation_Manager->Add( anim );
	}
	// debug key F4
	else if( key == SDLK_F4 )
	{
		Draw_Effect_Out( EFFECT_OUT_RANDOM_COLOR_BOOST );
		//pPlayer->Get_Item( TYPE_MUSHROOM_DEFAULT );

		/*if( pGlobal_effect->type != GL_EFF_FALLING )
		{
			pGlobal_effect->Set_image( "animation/particles/rain.png" );
			pGlobal_effect->Set_Type( GL_EFF_FALLING );
			pGlobal_effect->Set_Lifetime_mod( 4 );
			pGlobal_effect->Set_Scale( 0.5f, 0.5f );
			pGlobal_effect->Set_Creation_Speed( 2 );
			pGlobal_effect->Set_Speed( 13, 5 );
			pGlobal_effect->Set_Direction( 90, 0 );
			pGlobal_effect->Set_ConstRotationZ( 0, 0 );
			pGlobal_effect->Init_Anim();
		}*/
	}
	// Toggle leveleditor
	else if( key == SDLK_F8 )
	{
		pLevel_Editor->Toggle();
	}
	// ## Game
	// Shoot
	else if( key == pPreferences->key_shoot && !editor_enabled )
	{
		pPlayer->Action_Shoot();
	}
	// Jump
	else if( key == pPreferences->key_jump && !editor_enabled )
	{
		pPlayer->Action_Jump();
	}
	// Action
	else if( key == pPreferences->key_action && !editor_enabled )
	{
		pPlayer->Action_Interact( INP_ACTION );
	}
	// Up
	else if( key == pPreferences->key_up && !editor_enabled )
	{
		pPlayer->Action_Interact( INP_UP );
	}
	// Down
	else if( key == pPreferences->key_down && !editor_enabled )
	{
		pPlayer->Action_Interact( INP_DOWN );
	}
	// Left
	else if( key == pPreferences->key_left && !editor_enabled )
	{
		pPlayer->Action_Interact( INP_LEFT );
	}
	// Right
	else if( key == pPreferences->key_right && !editor_enabled )
	{
		pPlayer->Action_Interact( INP_RIGHT );
	}
	// Request Item
	else if( key == SDLK_RETURN && !editor_enabled )
	{
		pPlayer->Action_Interact( INP_ITEM );
	}
	// God Mode
	else if( pKeyboard->keys[SDLK_g] && pKeyboard->keys[SDLK_o] && pKeyboard->keys[SDLK_d] && !editor_enabled )
	{
		if( pPlayer->godmode )
		{
			debugdisplay->Set_Text( "Funky God Mode disabled" );
		}
		else
		{
			debugdisplay->Set_Text( "Funky God Mode enabled" );
		}

		pPlayer->godmode = !pPlayer->godmode;
	}
	// Set Small state
	else if( pKeyboard->keys[SDLK_k] && pKeyboard->keys[SDLK_i] && pKeyboard->keys[SDLK_d] && !editor_enabled )
	{
		pPlayer->Set_Type( MARYO_SMALL, 0 );
	}
	// Exit
	else if( key == SDLK_ESCAPE )
	{
		pPlayer->Action_Interact( INP_EXIT );
	}
	// ## editor
	else if( pLevel_Editor->Key_Down( key ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cLevel :: Key_Up( SDLKey key )
{
	// only if not in Editor
	if( editor_level_enabled )
	{
		return 0;
	}

	// Interaction keys
	if( key == pPreferences->key_right )
	{
		pPlayer->Action_Stop_Interact( INP_RIGHT );
	}
	else if( key == pPreferences->key_left )
	{
		pPlayer->Action_Stop_Interact( INP_LEFT );
	}
	else if( key == pPreferences->key_down )
	{
		pPlayer->Action_Stop_Interact( INP_DOWN );
	}
	else if( key == pPreferences->key_jump )
	{
		pPlayer->Action_Stop_Interact( INP_JUMP );
	}
	else if( key == pPreferences->key_shoot )
	{
		pPlayer->Action_Stop_Interact( INP_SHOOT );
	}
	else if( key == pPreferences->key_action )
	{
		pPlayer->Action_Stop_Interact( INP_ACTION );
	}
	else
	{
		// not processed
		return 0;
	}


	// key got processed
	return 1;
}

bool cLevel :: Mouse_Down( Uint8 button )
{
	// ## editor
	if( pLevel_Editor->Mouse_Down( button ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

bool cLevel :: Mouse_Up( Uint8 button )
{
	// ## editor
	if( pLevel_Editor->Mouse_Up( button ) )
	{
		// processed by the editor
		return 1;
	}
	else
	{
		// not processed
		return 0;
	}

	// button got processed
	return 1;
}

bool cLevel :: Joy_Button_Down( Uint8 button )
{
	// Shoot
	if( button == pPreferences->joy_button_shoot && !editor_enabled )
	{
		pPlayer->Action_Interact( INP_SHOOT );
	}
	// Jump
	else if( button == pPreferences->joy_button_jump && !editor_enabled )
	{
		pPlayer->Action_Interact( INP_JUMP );
	}
	// Interaction keys
	else if( button == pPreferences->joy_button_action && !editor_enabled )
	{
		pPlayer->Action_Interact( INP_ACTION );
	}
	// Request Itembox Item
	else if( button == pPreferences->joy_button_item && !editor_enabled )
	{
		pPlayer->Action_Interact( INP_ITEM );
	}
	// Enter menu
	else if( button == pPreferences->joy_button_exit )
	{
		pPlayer->Action_Interact( INP_EXIT );
	}
	else
	{
		// not processed
		return 0;
	}

	// key got processed
	return 1;
}

bool cLevel :: Joy_Button_Up( Uint8 button )
{
	// only if not in Editor
	if( editor_level_enabled )
	{
		return 0;
	}

	if( button == pPreferences->joy_button_jump )
	{
		pPlayer->Action_Stop_Interact( INP_JUMP );
	}
	else if( button == pPreferences->joy_button_shoot )
	{
		pPlayer->Action_Stop_Interact( INP_SHOOT );
	}
	else if( button == pPreferences->joy_button_action )
	{
		pPlayer->Action_Stop_Interact( INP_ACTION );
	}
	else
	{
		// not processed
		return 0;
	}


	// key got processed
	return 1;
}

string cLevel :: Get_Musicfile( int with_dir /* = 2 */, bool with_end /* = 1 */ )
{
	string filename = musicfile;

	// erase whole directory
	if( with_dir == 0 && filename.rfind( "/" ) != string::npos )
	{
		filename.erase( 0, filename.rfind( "/" ) + 1 );
	}
	// erase music directory
	else if( with_dir == 1 && filename.find( DATA_DIR "/" GAME_MUSIC_DIR "/" ) != string::npos )
	{
		filename.erase( 0, strlen( DATA_DIR "/" GAME_MUSIC_DIR "/" ) );
	}

	// erase file type
	if( !with_end && filename.rfind( "." ) != string::npos )
	{
		filename.erase( filename.rfind( "." ) );
	}

	return filename;
}

void cLevel :: Set_Musicfile( string filename )
{
	if( filename.length() < 4 )
	{
		return;
	}

	Convert_Path_Separators( filename );

	// add music dir
	if( filename.find( DATA_DIR "/" GAME_MUSIC_DIR "/" ) == string::npos )
	{
		filename.insert( 0, DATA_DIR "/" GAME_MUSIC_DIR "/" );
	}

	musicfile = filename;
	// check if music is available
	valid_music = File_Exists( filename );
}

void cLevel :: Set_Levelfile( string filename, bool delete_old /* = 1 */ )
{
	// erase file type and directory
	filename = Get_Filename( filename, 0, 0 );

	// if invalid
	if( filename.length() < 2 )
	{
		return;
	}

	// delete file with the old name
	if( delete_old )
	{
		Delete_File( levelfile );
	}

	Convert_Path_Separators( filename );

	// add level file type
	if( filename.find( ".smclvl" ) == string::npos )
	{
		filename.insert( filename.length(), ".smclvl" );
	}

	levelfile = filename;
	data_file = filename;

	// add level dir
	if( levelfile.find( user_data_dir + USER_LEVEL_DIR + "/" ) == string::npos )
	{
		levelfile.insert( 0, user_data_dir + USER_LEVEL_DIR + "/" );
	}

	// save with new filename
	Save();
}

bool cLevel :: Get_Path( string &filename, bool check_only_user_dir /* = 0 */ )
{
	filename = Get_Filename( filename, 0, 0 );

	// user level directory as default
	filename.insert( 0, user_data_dir + USER_LEVEL_DIR + "/" );
	// use new file type as default
	filename.insert( filename.length(), ".smclvl" );

	if( File_Exists( filename ) )
	{
		// found
		return 1;
	}

	// use old file type
	filename.erase( filename.rfind( "." ) );
	filename.insert( filename.length(), ".txt" );

	if( File_Exists( filename ) )
	{
		// found
		return 1;
	}

	if( !check_only_user_dir )
	{
		// use new file type
		filename.erase( filename.rfind( "." ) );
		filename.insert( filename.length(), ".smclvl" );

		// erase user level directory
		filename.erase( 0, user_data_dir.length() + strlen( USER_LEVEL_DIR "/" ) );

		// game level directory
		if( filename.find( DATA_DIR "/" GAME_LEVEL_DIR "/" ) == string::npos )
		{
			filename.insert( 0, DATA_DIR "/" GAME_LEVEL_DIR "/" );
		}

		if( File_Exists( filename ) )
		{
			// found
			return 1;
		}

		// use old file type
		filename.erase( filename.rfind( "." ) );
		filename.insert( filename.length(), ".txt" );

		if( File_Exists( filename ) )
		{
			// found
			return 1;
		}
	}

	// erase file type and directory
	filename = Get_Filename( filename, 0, 0 );

	// not found
	return 0;
}

void cLevel :: Set_Author( string name )
{
	author = name;
}

void cLevel :: Set_Version( string level_version )
{
	version = level_version;
}

cLevel_Entry* cLevel :: Get_Entry( string name )
{
	if( name.empty() )
	{
		return NULL;
	}

	// Search for entry
	for( SpriteList::iterator itr = pSprite_Manager->objects.begin(), itr_end = pSprite_Manager->objects.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		if( obj->type != TYPE_LEVEL_ENTRY )
		{
			continue;
		}

		cLevel_Entry *level_entry = static_cast<cLevel_Entry *>(obj);

		// found
		if( level_entry->entry_name.compare( name ) == 0 )
		{
			return level_entry;
		}
	}

	return NULL;
}

bool cLevel :: Is_Loaded( void )
{
	// if not loaded version is -1
	if( engine_version >= 0 )
	{
		return 1;
	}

	return 0;
}

bool cLevel :: HandleMessage( string *parts, unsigned int count, unsigned int line )
{
	if( parts[0].compare( "Player" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		pPlayer->Set_Pos( static_cast<float>(string_to_int( parts[1] )), static_cast<float>(game_res_h) - static_cast<float>( string_to_int( parts[2] ) + pPlayer->start_image->h ), 1 );
	}
	else if( parts[0].compare( "author" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		author = parts[1];
	}
	else if( parts[0].compare( "version" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		version = parts[1];
	}
	/***************************************************************************/
	else if( parts[0].compare( "Music" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( parts[1].length() < 3 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "No valid Music file\n" );
			return 0;  // error
		}

		Set_Musicfile( parts[1] );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Mod_Camera" ) == 0 )
	{
		// ignore
	}
	/***************************************************************************/
	else if( parts[0].compare( "Background_Color" ) == 0 ) // compatibility with old levels
	{
		Color color;

		if( Read_Color_Data( parts, count, line, color ) == 0 )
		{
			return 0;
		}

		background_manager->Get_Pointer(0)->Set_Color_1( color );
		background_manager->Get_Pointer(0)->Set_Color_2( color );
	}
	else if( parts[0].compare( "Background_color_1" ) == 0 ) // top background color
	{
		Color color;

		if( !Read_Color_Data( parts, count, line, color ) )
		{
			return 0;
		}

		background_manager->Get_Pointer(0)->Set_Color_1( color );
	}
	else if( parts[0].compare( "Background_color_2" ) == 0 ) // bottom background color
	{
		Color color;

		if( !Read_Color_Data( parts, count, line, color ) )
		{
			return 0;
		}

		background_manager->Get_Pointer(0)->Set_Color_2( color );
	}
	/***************************************************************************/
	else if( parts[0].find( "Background_image" ) == 0 && parts[0].length() <= 18 )
	{
		if( count < 2 || count > 6 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s %s\n", parts[0].c_str(), "needs 2-6 parameters" );
			return 0; // error
		}

		if( parts[1].length() < 3 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Not a valid %s file\n", parts[0].c_str() );
			return 0;  // error
		}

		string filename;

		filename.reserve( parts[1].length() + 50 );
		filename = parts[1];

		if( filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == string::npos )
		{
			filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
		}

		if( !File_Exists( filename ) )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s file not found : %s\n", parts[0].c_str(), filename.c_str() );

			return 0; // error
		}

		cBackground *background = new cBackground();
		background->Set_Image( parts[1] );

		// type is given
		if( count > 2 )
		{
			unsigned int bg_type = string_to_int( parts[2] );

			if( bg_type < 0 || bg_type > 200 )
			{
				printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "Not a valid Background image type ( %d )\n", bg_type );
				return 0;  // error
			}

			background->Set_Type( (BackgroundType)bg_type );


		}
		// default type
		else
		{
			background->Set_Type( BG_IMG_BOTTOM );
		}

		// x speed is given
		if( count > 3 )
		{
			float speedx = string_to_float( parts[3] );
			float speedy = 1;

			// y speed is given
			if( count > 4 )
			{
				speedy = string_to_float( parts[3] );
			}

			// set speed
			background->Set_Scroll_Speed( speedx, speedy );
		}

		// z position is given
		if( count > 5 )
		{
			background->Set_Pos_Z( string_to_float( parts[4] ) );
		}

		background_manager->Add( background );
	}
	/***************************************************************************/
	else if( parts[0].find( "Background_image_type" ) == 0 )
	{
		// ignore
	}
	/***************************************************************************/
	else if( parts[0].find( "global_effect" ) == 0 || parts[0].find( "ge_base" ) == 0 )
	{
		if( count < 3 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( parts[2].length() < 3 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Not a valid %s file\n", parts[0].c_str() );
			return 0;  // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		string filename;

		filename.reserve( parts[2].length() + 50 );
		filename = parts[2];

		if( filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == string::npos )
		{
			filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
		}

		if( !File_Exists( filename ) )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s file not found : %s\n", parts[0].c_str(), filename.c_str() );

			return 0; // error
		}

		// remove pixmaps dir
		if( filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == 0 )
		{
			filename.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );
		}

		int effect_type = string_to_int( parts[1] );

		pGlobal_effect->Set_Type( (GlobalEffectType)effect_type );
		pGlobal_effect->Set_Image( filename );
	}
	/***************************************************************************/
	else if( parts[0].find( "ge_rect" ) == 0 )
	{
		pGlobal_effect->Set_Emitter_Rect( GL_rect( string_to_float( parts[1] ), string_to_float( parts[2] ), string_to_float( parts[3] ), string_to_float( parts[4] ) ) );
	}
	else if( parts[0].find( "ge_lifetime_mod" ) == 0 )
	{
		pGlobal_effect->Set_Time_to_Live( string_to_float( parts[1] ) * 0.3f );
	}
	else if( parts[0].find( "ge_creation_speed" ) == 0 )
	{
		pGlobal_effect->Set_Emitter_Iteration_Interval( ( 1 / string_to_float( parts[1] ) ) * 0.032f );
	}
	else if( parts[0].find( "ge_scale" ) == 0 )
	{
		pGlobal_effect->Set_Scale( string_to_float( parts[1] ), string_to_float( parts[2] ) );
	}
	else if( parts[0].find( "ge_speed" ) == 0 )
	{
		pGlobal_effect->Set_Speed( string_to_float( parts[1] ), string_to_float( parts[2] ) );
	}
	else if( parts[0].find( "ge_dir_range" ) == 0 )
	{
		pGlobal_effect->Set_Direction( string_to_float( parts[1] ), string_to_float( parts[2] ) );
	}
	else if( parts[0].find( "ge_const_rotz" ) == 0 )
	{
		pGlobal_effect->Set_Const_Rotation_Z( string_to_float( parts[1] ), string_to_float( parts[2] ) );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Levelengine_version" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		engine_version = static_cast<float>(string_to_int( parts[1] ));
	}
	/***************************************************************************/
	else if( parts[0].compare( "Sprite" ) == 0 || parts[0].compare( "ClimbSprite" ) == 0 )
	{
		if( count != 5 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 5 parameters" );
			return 0; // error
		}

		// check if the image file exists
		string filename = parts[1];

		while( filename.find( "/" ) == 0 )
		{
			filename.erase( 0, 1 );
		}

		if( filename.find( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) == string::npos )
		{
			filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
		}

		string settings_file = filename;
		settings_file.erase( settings_file.rfind( "." ) + 1 );
		settings_file.insert( settings_file.rfind( "." ) + 1, "settings" );

		if( !File_Exists( filename ) && !File_Exists( settings_file ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "file not found : %s\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[3] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[3].c_str() );
			return 0; // error
		}
		if( !parts[4].compare( "MASSIVE" ) == 0 && !parts[4].compare( "PASSIVE" ) == 0 && !parts[4].compare( "FRONT_PASSIVE" ) == 0 && !parts[4].compare( "HALFMASSIVE" ) == 0 )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "parameter five has to be MASSIVE, PASSIVE, or HALFMASSIVE\n" );
			return 0; // error
		}

		string getimage_path = filename;
		getimage_path.erase( 0, strlen( DATA_DIR "/" GAME_PIXMAPS_DIR "/" ) );

		cGL_Surface *surface = pVideo->Get_Surface( getimage_path );
		int posx = string_to_int( parts[2] );
		int posy = game_res_h - string_to_int( parts[3] );

		// if image is obsolete show a warning
		if( surface->obsolete )
		{
			printf( "%s : line %d Warning :\n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "using obsolete image : %s\nAt position x : %d, y : %d\n", getimage_path.c_str(), posx, posy );
		}

		cSprite *temp = new cSprite( surface, static_cast<float>(posx), static_cast<float>(posy) );

		if( parts[4].compare( "MASSIVE" ) == 0 )
		{
			temp->Set_Sprite_Type( TYPE_MASSIVE );
		}
		else if( parts[4].compare( "PASSIVE" ) == 0 )
		{
			temp->Set_Sprite_Type( TYPE_PASSIVE );
		}
		else if( parts[4].compare( "FRONT_PASSIVE" ) == 0 )
		{
			temp->Set_Sprite_Type( TYPE_FRONT_PASSIVE );
		}
		else if( parts[4].compare( "HALFMASSIVE" ) == 0 )
		{
			// Climbable
			if( parts[0].compare( "ClimbSprite" ) == 0 )
			{
				temp->Set_Sprite_Type( TYPE_CLIMBABLE );
			}
			// Halfmassive
			else
			{
				temp->Set_Sprite_Type( TYPE_HALFMASSIVE );
			}
		}

		pSprite_Manager->Add( temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Enemy" ) == 0 )
	{
		if( count < 3 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs more than 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[3] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[3].c_str() );
			return 0; // error
		}

		// Goomba changed to Gumba and since 1.5 changed to Furball
		if( parts[1].compare( "GOOMBA" ) == 0 )
		{
			/* 3 and 4 is a very old description
			 * 5 is lower than V.0.98
			*/
			if( !( count == 3 || count == 4 || count == 5 || count == 6 ) )
			{
				printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 5 parameters" );
				return 0; // error
			}

			if( count >= 5 ) // color support
			{
				cFurball *temp = NULL;
				DefaultColor color = static_cast<DefaultColor>(string_to_int( parts[4] ));

				// correct old color types
				if( count == 5 )
				{
					// Blue
					if( color == 1 )
					{
						color = COL_BLUE;
					}
					// Brown
					else
					{
						color = COL_BROWN;
					}
				}

				temp = new cFurball( (float)string_to_int( parts[2] ), (float)( game_res_h - string_to_int( parts[3] ) ) );
				temp->Set_Color( color );

				// direction support
				if( count >= 6 )
				{
					if( !is_valid_number( parts[5] ) )
					{
						printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
						printf( "%s is not a valid integer value\n", parts[5].c_str() );
						return 0; // error
					}

					ObjectDirection direction = DIR_RIGHT;
					direction = (ObjectDirection)string_to_int( parts[5] );

					if( direction == DIR_LEFT || direction == DIR_RIGHT )
					{
						temp->Set_Direction( direction );
					}
				}

				if( temp )
				{
					pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
				}
			}
			// support for old levels ( lower than V.0.8 )
			else
			{
				cFurball *temp = new cFurball( (float)string_to_int( parts[2] ), (float)( game_res_h - string_to_int( parts[3] ) ) );
				pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
			}
		}
		// Turtle
		// RTURTLE for old level support
		else if( parts[1].compare( "RTURTLE" ) == 0 || parts[1].compare( "TURTLE" ) == 0 )
		{
			if( count < 4 || count > 5 )
			{
				printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 4-5 parameters" );
				return 0; // error
			}

			ObjectDirection direction = DIR_RIGHT;

			// with direction
			if( count > 4 )
			{
				direction = (ObjectDirection)string_to_int( parts[4] );
			}

			cTurtle *temp = new cTurtle( (float)string_to_int( parts[2] ), (float)( game_res_h - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
		}
		// Jumping Piranha
		else if( parts[1].compare( "JPIRANHA" ) == 0 )
		{
			if( count < 4 || count > 7 )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 4-7 parameters" );
				return 0; // error
			}

			ObjectDirection direction = DIR_UP;
			float max_distance = 200;
			float speed = 5.8f;

			// with direction and max_distance
			if( count > 5 )
			{
				if( !is_valid_number( parts[4] ) )
				{
					printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
					printf( "%s is not a valid integer value\n", parts[4].c_str() );
					return 0; // error
				}

				if( !is_valid_number( parts[5] ) )
				{
					printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
					printf( "%s is not a valid integer value\n", parts[5].c_str() );
					return 0; // error
				}

				direction = (ObjectDirection)string_to_int( parts[4] );
				max_distance = (float)string_to_int( parts[5] );

				// fix wrong values
				if( max_distance <= 0 )
				{
					max_distance = 200;
				}
			}

			// with speed
			if( count > 6 )
			{
				speed = string_to_float( parts[6] );
			}

			cjPiranha *temp = new cjPiranha( (float)string_to_int( parts[2] ), (float)( game_res_h - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			temp->Set_Max_Distance( max_distance );
			temp->Set_Speed( speed );
			pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
		}
		// Rokko
		// BANZAI_BILL was the old name
		else if( parts[1].compare( "ROKKO" ) == 0 || parts[1].compare( "BANZAI_BILL" ) == 0 )
		{
			if( count < 5 || count > 6 )
			{
				printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "Line %d, error : %s %s\n", line, parts[1].c_str(), "needs 5-6 parameters" );
				return 0; // error
			}

			float speed = 8.5f;

			// with speed
			if( count > 5 )
			{
				speed = string_to_float( parts[5] );
			}

			cRokko *temp = new cRokko( (float)string_to_int( parts[2] ), (float)( game_res_h - string_to_int( parts[3] ) ) );
			temp->Set_Direction( (ObjectDirection)string_to_int( parts[4] ) );
			temp->Set_Speed( speed );
			pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
		}
		// Rex and since 1.5 changed to Krush
		else if( parts[1].compare( "REX" ) == 0 )
		{
			if( count < 4 || count > 5 )
			{
				printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "Line %d, error : %s %s\n", line, parts[1].c_str(), "needs 4-5 parameters" );
				return 0; // error
			}

			ObjectDirection direction = DIR_RIGHT;

			// with direction
			if( count > 4 )
			{
				direction = (ObjectDirection)string_to_int( parts[4] );
			}

			cKrush *temp = new cKrush( (float)string_to_int( parts[2] ), (float)( game_res_h - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
		}
		// Thromp
		// Thwomp was the old name
		else if( parts[1].compare( "THROMP" ) == 0 || parts[1].compare( "THWOMP" ) == 0 )
		{
			if( count < 6 || count > 7 )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 6-7 parameters" );
				return 0; // error
			}

			if( !is_valid_number( parts[4] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[4].c_str() );
				return 0; // error
			}

			if( !is_valid_number( parts[5] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[5].c_str() );
				return 0; // error
			}

			ObjectDirection direction = (ObjectDirection)string_to_int( parts[4] );
			int max_distance = string_to_int( parts[5] );
			float speed = 7;

			// fix wrong values
			if( max_distance <= 0 )
			{
				max_distance = 200;
			}

			// with speed
			if( count > 6 )
			{
				speed = string_to_float( parts[6] );
			}

			cThromp *temp = new cThromp( (float)string_to_int( parts[2] ), (float)( game_res_h - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			temp->Set_Max_Distance( static_cast<float>(max_distance) );
			temp->Set_Speed( speed );

			pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
		}
		// Eato
		else if( parts[1].compare( "EATO" ) == 0 )
		{
			if( count != 5 )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 5 parameters" );
				return 0; // error
			}

			if( !is_valid_number( parts[4] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[4].c_str() );
				return 0; // error
			}

			ObjectDirection direction = (ObjectDirection)string_to_int( parts[4] );

			cEato *temp = new cEato( (float)string_to_int( parts[2] ), (float)( game_res_h - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
		}
		// Gee
		else if( parts[1].compare( "GEE" ) == 0 )
		{
			if( count < 6 || count > 7 )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 6-7 parameters" );
				return 0; // error
			}

			if( !is_valid_number( parts[4] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[4].c_str() );
				return 0; // error
			}

			if( !is_valid_number( parts[5] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[5].c_str() );
				return 0; // error
			}

			ObjectDirection direction = (ObjectDirection)string_to_int( parts[4] );
			DefaultColor color = COL_YELLOW;
			unsigned int max_distance = 400;

			// convert old gee type to color
			unsigned int gtype = string_to_int( parts[5] );

			if( gtype == 1 )
			{
				color = COL_YELLOW;
			}
			else if( gtype == 2 )
			{
				color = COL_RED;
			}
			else if( gtype == 3 )
			{
				color = COL_GREEN;
			}

			// with max distance
			if( count > 6 )
			{
				max_distance = string_to_int( parts[6] );
			}

			cGee *temp = new cGee( (float)string_to_int( parts[2] ), (float)( game_res_h - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			temp->Set_Max_Distance( max_distance );
			temp->Set_Color( color );
			pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
		}
		else
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Line %d, error : %s %s\n", line, parts[1].c_str(), "Unknown Enemy type" );
			return 0; // error
		}
	}
	/***************************************************************************/
	else if( parts[0].compare( "Goldpiece" ) == 0 )
	{
		if( count < 3 || count > 4 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-4 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		DefaultColor color = COL_YELLOW;

		// with color information
		if( count > 3 )
		{
			if( !is_valid_number( parts[3] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[3].c_str() );
				return 0; // error
			}

			color = static_cast<DefaultColor>(string_to_int( parts[3] ));
		}

		cGoldpiece *temp = new cGoldpiece( (float)string_to_int( parts[1] ), (float)( game_res_h - string_to_int( parts[2] ) ) );
		temp->Set_Gold_Color( color );
		pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Moon" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		cMoon *temp = new cMoon( (float)string_to_int( parts[1] ), (float)( game_res_h - string_to_int( parts[2] ) ) );
		pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Levelexit" ) == 0 )
	{
		if( count < 3 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-5 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		cLevel_Exit *temp = NULL;

		// with type and level
		if( count == 4 || count == 5 )
		{
			if( !is_valid_number( parts[3] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[2].c_str() );
				return 0; // error
			}

			temp = new cLevel_Exit( (float)string_to_int( parts[1] ), (float)( game_res_h - string_to_int( parts[2] ) ) );

			// type
			temp->Set_Type( (Level_Exit_type)string_to_int( parts[3] ) );

			// with a given level
			if( count == 5 )
			{
				temp->Set_Level( parts[4] );
			}
		}
		// older level support
		else
		{
			// add a door sprite
			cMovingSprite *door_sprite = new cMovingSprite( pVideo->Get_Surface( "game/level/door_yellow_1.png" ), (float)string_to_int( parts[1] ), (float)( game_res_h - string_to_int( parts[2] ) ) );
			door_sprite->Set_Sprite_Type( TYPE_PASSIVE );
			pSprite_Manager->Add( door_sprite );
			// add the levelexit
			temp = new cLevel_Exit( (float)( string_to_int( parts[1] ) + 13 ), (float)( game_res_h - string_to_int( parts[2] ) + 60 ) );
		}

		pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
	}
	/***************************************************************************/
	else if( parts[0].compare( "BonusBox" ) == 0 )
	{
		if( !( count == 3 || count == 4 ) )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-4 parameters" );
			return 0; // error
		}

		// old level support
		if( count == 3 )
		{
			if( !is_valid_number( parts[1] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[1].c_str() );
				return 0; // error
			}
			if( !is_valid_number( parts[2] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[2].c_str() );
				return 0; // error
			}

			cBonusBox *temp = new cBonusBox( (float)string_to_int( parts[1] ), (float)( game_res_h - string_to_int( parts[2] ) ) );
			pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
		}
		else if( count == 4 )
		{
			if( !is_valid_number( parts[1] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[1].c_str() );
				return 0; // error
			}
			if ( !is_valid_number( parts[2] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[2].c_str() );
				return 0; // error
			}

			cBonusBox *temp = new cBonusBox( (float)string_to_int( parts[1] ), (float)( game_res_h - string_to_int( parts[2] ) ) );

			// Auto Mushroom or Fireplant
			if( parts[3].compare( "AUTO" ) == 0 )
			{
				temp->Set_Bonus_Type( TYPE_FIREPLANT );
			}
			// Life Mushroom
			else if( parts[3].compare( "LIFE" ) == 0 )
			{
				 temp->Set_Bonus_Type( TYPE_MUSHROOM_LIVE_1 );
			}
			// Jumping Star
			else if( parts[3].compare( "STAR" ) == 0 )
			{
				 temp->Set_Bonus_Type( TYPE_JSTAR );
			}
			// unknown Bonus item
			else
			{
				delete temp;
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "Bonusbox type Error %s\n", parts[3].c_str() );
				return 0; // error
			}

			pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
		}
		else
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Bonusbox count Error %d\n", count );
			return 0; // error
		}
	}
	/***************************************************************************/
	else if( parts[0].compare( "GoldBox" ) == 0 )
	{
		if( count < 3 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-5 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		DefaultColor color = COL_YELLOW;

		// with color information
		if( count > 3 )
		{
			if( !is_valid_number( parts[3] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[3].c_str() );
				return 0; // error
			}

			color = static_cast<DefaultColor>(string_to_int( parts[3] ));
		}

		cBonusBox *temp = new cBonusBox( (float)string_to_int( parts[1] ), (float)( game_res_h - string_to_int( parts[2] ) ) );
		temp->Set_Animation( "Default" );
		temp->Set_Bonus_Type( TYPE_GOLDPIECE );
		temp->Set_Goldcolor( color );
		pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );

		// with useable times information
		if( count > 4 )
		{
			if( !is_valid_number( parts[4] ) )
			{
				printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[4].c_str() );
				return 0; // error
			}

			temp->Set_Useable_Count( string_to_int( parts[4] ), 1 );
		}
	}
	/***************************************************************************/
	else if( parts[0].compare( "SpinBox" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		cSpinBox *temp = new cSpinBox( (float)string_to_int( parts[1] ), (float)( game_res_h - string_to_int( parts[2] ) ) );
		pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Cloud" ) == 0 )
	{
		if( count < 3 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-5 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		// convert to halfmassive
		cMovingSprite *temp = new cMovingSprite( pVideo->Get_Surface( "clouds/default_1/1_middle.png" ), (float)string_to_int( parts[1] ), (float)( game_res_h - string_to_int( parts[2] ) ) );
		temp->Set_Sprite_Type( TYPE_HALFMASSIVE );

		pSprite_Manager->Add( temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "EnemyStopper" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}
		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		cEnemyStopper *temp = new cEnemyStopper( (float)string_to_int( parts[1] ), (float)( game_res_h - string_to_int( parts[2] ) ) );
		pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
	}
	/***************************************************************************/
	else if( parts[0].compare( "movingplatform" ) == 0 )
	{
		if( count < 10 || count > 10 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 10 parameters" );
			return 0; // error
		}

		cMoving_Platform *temp = new cMoving_Platform( (float)string_to_int( parts[1] ), (float)(game_res_h - string_to_int( parts[2] )) );

		if( string_to_int( parts[3] ) == 1 )
		{
			temp->Set_Direction( DIR_DOWN );
		}
		else
		{
			temp->Set_Direction( DIR_RIGHT );
		}

		temp->Set_Max_Distance( string_to_int( parts[4] ) );
		temp->Set_Speed( string_to_float( parts[5] ) );
		temp->Set_Middle_Count( string_to_int( parts[6] ) );

		temp->Set_Image_Top_Left( pVideo->Get_Surface( parts[7] ) );
		temp->Set_Image_Top_Middle( pVideo->Get_Surface( parts[8] ) );
		temp->Set_Image_Top_Right( pVideo->Get_Surface( parts[9] ) );

		pSprite_Manager->Add( static_cast<cMovingSprite *>(temp) );
	}
	/***************************************************************************/
	else if( parts[0].compare( "camera_limits" ) == 0 )
	{
		if( count < 5 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 5 parameters" );
			return 0; // error
		}

		pLevel_Manager->camera->Set_Limits( GL_rect( (float)string_to_int( parts[1] ), (float)string_to_int( parts[2] ), (float)string_to_int( parts[3] ), (float)string_to_int( parts[4] ) ) );
	}
	/***************************************************************************/
	else
	{
		printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
		printf( "Unknown Command : %s\n", parts[0].c_str() );
		return 0; // error
	}

	// Successful
	return 1;
}

bool cLevel :: Read_Color_Data( string *parts, unsigned int count, unsigned int line, Color &color )
{
	if( count != 4 )
	{
		printf( "%s : line %d Error : \n", Get_Filename( data_file, 0, 0 ).c_str(), line );
		printf( "Error : %s %s\n", parts[0].c_str(), "needs 4 parameters" );
		return 0; // error
	}

	if( !is_valid_number( parts[1] ) )
	{
		printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
		printf( "%s is not a valid integer value\n", parts[1].c_str() );
		return 0; // error
	}
	if( !is_valid_number( parts[2] ) )
	{
		printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
		printf( "%s is not a valid integer value\n", parts[2].c_str() );
		return 0; // error
	}
	if( !is_valid_number( parts[3] ) )
	{
		printf( "%s : line %d Error : ", Get_Filename( data_file, 0, 0 ).c_str(), line );
		printf( "%s is not a valid integer value\n", parts[3].c_str() );
		return 0; // error
	}

	int background_red = string_to_int( parts[1] );
	int background_green = string_to_int( parts[2] );
	int background_blue = string_to_int( parts[3] );

	if( background_red < 0 || background_red > 255 )
	{
		printf( "Warning : Background color red is not between 0 - 255\n" );
		background_red = 0;
	}
	else if( background_green < 0 || background_green > 255 )
	{
		printf( "Warning : Background color green is not between 0 - 255\n" );
		background_green = 0;
	}
	else if( background_blue < 0 || background_blue > 255 )
	{
		printf( "Warning : Background color blue is not between 0 - 255\n" );
		background_blue = 0;
	}

	color.red = background_red;
	color.green = background_green;
	color.blue = background_blue;

	return 1;  // success
}

// XML element start
void cLevel :: elementStart( const CEGUI::String &element, const CEGUI::XMLAttributes &attributes )
{
	// Property/Item/Tag of an Element
    if( element == "Property" )
    {
		xml_attributes.add( attributes.getValueAsString( "name" ), attributes.getValueAsString( "value" ) );
    }
}

// XML element end
void cLevel :: elementEnd( const CEGUI::String &element )
{
	if( element != "Property" )
	{
		if( element == "information" )
		{
			engine_version = xml_attributes.getValueAsFloat( "engine_version" );
			last_saved = xml_attributes.getValueAsInteger( "save_time" );
		}
		else if( element == "settings" )
		{
			// Author
			author = xml_attributes.getValueAsString( "lvl_author" ).c_str();
			// Version
			version = xml_attributes.getValueAsString( "lvl_version" ).c_str();
			// Music
			Set_Musicfile( xml_attributes.getValueAsString( "lvl_music" ).c_str() );
			// Camera Limits
			pLevel_Manager->camera->Set_Limits( GL_rect( static_cast<float>(xml_attributes.getValueAsInteger( "cam_limit_x" )), static_cast<float>(xml_attributes.getValueAsInteger( "cam_limit_y" )), static_cast<float>(xml_attributes.getValueAsInteger( "cam_limit_w" )), static_cast<float>(xml_attributes.getValueAsInteger( "cam_limit_h" )) ) );
			// fixed camera horizontal velocity
			pLevel_Manager->camera->fixed_hor_vel = xml_attributes.getValueAsFloat( "cam_fixed_hor_vel" );
		}
		else if( element == "background" )
		{
			BackgroundType bg_type = static_cast<BackgroundType>(xml_attributes.getValueAsInteger( "type" ));

			// use gradient background
			if( bg_type == BG_GR_HOR || bg_type == BG_GR_VER )
			{
				background_manager->Get_Pointer(0)->Create_from_Stream( xml_attributes );
			}
			// default background
			else
			{
				background_manager->Add( new cBackground( xml_attributes ) );
			}
		}
		else if( element == "global_effect" )
		{
			pGlobal_effect->Create_from_Stream( xml_attributes );
		}
		else if( element == "player" )
		{
			pPlayer->Create_from_Stream( xml_attributes );
		}
		else
		{
			// get Level object
			cSprite *object = Get_Level_Object( element, xml_attributes, engine_version );
			
			// valid
			if( object )
			{
				pSprite_Manager->Add( object );
			}
			else if( element == "level" )
			{
				// ignore
			}
			else if( element.length() )
			{
				printf( "Warning : Level Unknown element : %s\n", element.c_str() );
			}
		}

		// clear
		xml_attributes = CEGUI::XMLAttributes();
	}
}

cSprite *Get_Level_Object( const CEGUI::String &xml_element, CEGUI::XMLAttributes &attributes, float engine_version /* = level_engine_version */ )
{
	// element could change
	CEGUI::String element = xml_element;

	if( element == "sprite" )
	{
		// if V.1.5 and lower : change pipe connection image paths
		if( engine_version < 2.8f )
		{
			Relocate_Image( attributes, "blocks/pipe/connection_left_down.png", "blocks/pipe/connection/plastic_1/orange/right_up.png" );
			Relocate_Image( attributes, "blocks/pipe/connection_left_up.png", "blocks/pipe/connection/plastic_1/orange/right_down.png" );
			Relocate_Image( attributes, "blocks/pipe/connection_right_down.png", "blocks/pipe/connection/plastic_1/orange/left_up.png" );
			Relocate_Image( attributes, "blocks/pipe/connection_right_up.png", "blocks/pipe/connection/plastic_1/orange/left_down.png" );
			Relocate_Image( attributes, "blocks/pipe/metal_connector.png", "blocks/pipe/connection/metal_1/grey/middle.png" );
		}

		// if V.1.4 and lower : change some image paths
		if( engine_version < 2.5f )
		{
			// change stone8 to metal stone 2 violet
			Relocate_Image( attributes, "game/box/stone8.png", "blocks/metal/stone_2_violet.png" );
			// move jungle_1 trees into a directory
			Relocate_Image( attributes, "ground/jungle_1/tree_type_1.png", "ground/jungle_1/tree/1.png" );
			Relocate_Image( attributes, "ground/jungle_1/tree_type_1_front.png", "ground/jungle_1/tree/1_front.png" );
			Relocate_Image( attributes, "ground/jungle_1/tree_type_2.png", "ground/jungle_1/tree/2.png" );
			// move yoshi_1 extra to jungle_2 hedge
			Relocate_Image( attributes, "ground/yoshi_1/extra_1_blue.png", "ground/jungle_2/hedge/1_blue.png" );
			Relocate_Image( attributes, "ground/yoshi_1/extra_1_green.png", "ground/jungle_2/hedge/1_green.png" );
			Relocate_Image( attributes, "ground/yoshi_1/extra_1_red.png", "ground/jungle_2/hedge/1_red.png" );
			Relocate_Image( attributes, "ground/yoshi_1/extra_1_yellow.png", "ground/jungle_2/hedge/1_yellow.png" );
			// move yoshi_1 rope to jungle_2
			Relocate_Image( attributes, "ground/yoshi_1/rope_1_leftright.png", "ground/jungle_2/rope_1_hor.png" );
		}

		cSprite *sprite = new cSprite( attributes );

		// if image not available display its filename
		if( !sprite->start_image )
		{
			string text = attributes.getValueAsString( "image" ).c_str();

			if( text.empty() )
			{
				text = "Invalid image here";
			}

			cGL_Surface *text_image = pFont->Render_Text( pFont->font_small, text );
			text_image->filename = text;
			// set text image
			sprite->Set_Image( text_image, 1, 1 );
			// display it as front passive
			sprite->Set_Sprite_Type( TYPE_FRONT_PASSIVE );
			// only display it in the editor
			sprite->Set_Visible( 0 );
		}

		// if V.1.2 and lower : change pipe position
		if( engine_version < 2.2f && sprite->image )
		{
			if( sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/up.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/ver.png" ) == 0 || 
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/down.png" ) == 0 || 
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/up.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/ver.png" ) == 0 || 
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/down.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/up.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/ver.png" ) == 0 || 
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/down.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/up.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/ver.png" ) == 0 || 
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/down.png" ) == 0 )
			{
				sprite->Move( -6, 0, 1 );
				sprite->startposx = sprite->posx;
			}
			else if( sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/right.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/hor.png" ) == 0 || 
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/green/left.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/right.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/hor.png" ) == 0 || 
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/blue/left.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/right.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/hor.png" ) == 0 || 
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/yellow/left.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/right.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/hor.png" ) == 0 || 
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "pipes/grey/left.png" ) == 0 )
			{
				sprite->Move( 0, -6, 1 );
				sprite->startposy = sprite->posy;
			}
		}
		// if V.1.2 and lower : change some hill positions
		if( engine_version < 2.3f && sprite->image )
		{
			if( sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "hills/green_1/head.png" ) == 0 ||
				sprite->image->filename.compare( DATA_DIR "/" GAME_PIXMAPS_DIR "/" "hills/light_blue_1/head.png" ) == 0 )
			{
				sprite->Move( 0, -6, 1 );
				sprite->startposy = sprite->posy;
			}
		}

		return sprite;
	}
	else if( element == "enemystopper" )
	{
		return new cEnemyStopper( attributes );
	}
	else if( element == "levelexit" )
	{
		return new cLevel_Exit( attributes );
	}
	else if( element == "level_entry" )
	{
		return new cLevel_Entry( attributes );
	}
	else if( element == "box" )
	{
		CEGUI::String str_type = attributes.getValueAsString( "type" );

		if( str_type == "bonus" )
		{
			return new cBonusBox( attributes );
		}
		// gold is somewhere pre 0.99.5
		else if( str_type == "gold" )
		{
			// update old values
			attributes.add( "type", "bonus" );
			attributes.add( "animation", "Default" );
			attributes.add( "item", int_to_string( TYPE_GOLDPIECE ) );
			// renamed color to gold_color
			if( attributes.exists( "color" ) )
			{
				attributes.add( "gold_color", attributes.getValue( "color" ) );
				attributes.remove( "color" );
			}

			return new cBonusBox( attributes );
		}
		else if( str_type == "spin" )
		{
			return new cSpinBox( attributes );
		}
		else if( str_type == "text" )
		{
			return new cText_Box( attributes );
		}
		// pre 0.99.4
		else if ( str_type == "empty" )
		{
			// update old values
			attributes.add( "type", "bonus" );
			attributes.add( "item", "0" );

		    return new cBonusBox( attributes );
		}
		// pre 0.99.4
		else if ( str_type == "invisible" )
		{
			// update old values
			attributes.add( "type", "bonus" );
			attributes.add( "item", "0" );
			attributes.add( "invisible", "1" );

		    return new cBonusBox( attributes );
		}
		else
		{
			printf( "Warning : Unknown Level Box type : %s\n", str_type.c_str() );
		}
	}
	// powerup is pre 0.99.5
	else if( element == "item" || element == "powerup" )
	{
		CEGUI::String str_type = attributes.getValueAsString( "type" );

		if( str_type == "goldpiece" )
		{
			return new cGoldpiece( attributes );
		}
		else if( str_type == "mushroom" )
		{
			return new cMushroom( attributes );
		}
		else if( str_type == "moon" )
		{
			return new cMoon( attributes );
		}
		else
		{
			printf( "Warning : Unknown Level Item type : %s\n", str_type.c_str() );
		}
	}
	else if( element == "moving_platform" )
	{
		return new cMoving_Platform( attributes );
	}
	// pre 1.5
	else if( element == "falling_platform" )
	{
		// no moving
		attributes.add( "speed", "0" );
		// renamed time_fall to touch_time and change to the new value
		if( attributes.exists( "time_fall" ) )
		{
			attributes.add( "touch_time", CEGUI::PropertyHelper::floatToString( attributes.getValueAsFloat( "time_fall" ) * speedfactor_fps ) );
			attributes.remove( "time_fall" );
		}
		else
		{
			attributes.add( "touch_time", "48" );
		}
		// enable falling
		attributes.add( "shake_time", "12" );

	    return new cMoving_Platform( attributes );
	}
	else if( element == "enemy" )
	{
		CEGUI::String str_type = attributes.getValueAsString( "type" );

		// if V.1.5 and lower
		if( engine_version < 2.6f )
		{
			// change gumba to furball
			if( str_type == "gumba" )
			{
				// change type
				str_type = "furball";
				attributes.add( "type", "furball" );
				// fix color : red was used in pre 1.0 but later was blue
				if( attributes.exists( "color" ) && attributes.getValueAsString( "color" ) == "red" )
				{
					attributes.add( "color", "blue" );
				}
			}
			// change rex to krush
			else if( str_type == "rex" )
			{
				// change type
				str_type = "krush";
				attributes.add( "type", "krush" );
			}
		}

		if( str_type == "eato" )
		{
			return new cEato( attributes );
		}
		else if( str_type == "furball" )
		{
			return new cFurball( attributes );
		}
		else if( str_type == "turtle" )
		{
			return new cTurtle( attributes );
		}
		else if( str_type == "turtleboss" )
		{
			// if V.1.5 and lower : max_downgrade_time changed to shell_time
			if( engine_version < 2.7f )
			{
				if( attributes.exists( "max_downgrade_time" ) )
				{
					attributes.add( "shell_time", attributes.getValueAsString( "max_downgrade_time" ) );
					attributes.remove( "max_downgrade_time" );
				}
			}

			return new cTurtleBoss( attributes );
		}
		else if( str_type == "jpiranha" )
		{
			return new cjPiranha( attributes );
		}
		else if( str_type == "thromp" )
		{
			cThromp *thromp = new cThromp( attributes );

			// if V.1.4 and lower : fix thromp distance was smaller
			if( engine_version < 2.5f )
			{
				thromp->Set_Max_Distance( thromp->max_distance + 36 );
			}

			return thromp;
		}
		else if( str_type == "rokko" )
		{
			return new cRokko( attributes );
		}
		else if( str_type == "krush" )
		{
			return new cKrush( attributes );
		}
		else if( str_type == "gee" )
		{
			return new cGee( attributes );
		}
		else if( str_type == "spika" )
		{
			return new cSpika( attributes );
		}
		else if( str_type == "static" )
		{
			return new cStaticEnemy( attributes );
		}
		else
		{
			printf( "Warning : Unknown Level Enemy type : %s\n", str_type.c_str() );
		}
	}
	else if( element == "sound" )
	{
		return new cRandom_Sound( attributes );
	}
	else if( element == "particle_emitter" )
	{
		cParticle_Emitter *particle_emitter = new cParticle_Emitter( attributes );
		// set to not spawned
		particle_emitter->spawned = 0;
		// init animation
		particle_emitter->Init_Anim();

		return particle_emitter;
	}

	return NULL;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel *pActive_Level = NULL;
