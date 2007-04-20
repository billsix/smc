/***************************************************************************
    level.cpp  -  class for handling level loading and many other things
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

#include "../level/level.h"
#include "../level/level_editor.h"
#include "../core/game_core.h"
#include "../core/main.h"
#include "../core/camera.h"
#include "../gui/hud.h"
#include "../gui/menu.h"
#include "../user/preferences.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../player/player.h"
#include "../core/obj_manager.h"
#include "../objects/goldpiece.h"
#include "../objects/levelexit.h"
#include "../video/font.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../input/joystick.h"
#include "../user/savegame.h"
#include "../overworld/worlds.h"
#include "../overworld/overworld.h"
#include "../enemies/turtle.h"
#include "../enemies/bosses/turtle_boss.h"
#include "../enemies/rokko.h"
#include "../enemies/rex.h"
#include "../enemies/gumba.h"
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
#include "../objects/moving_platform.h"
#include "../objects/falling_platform.h"
#include "../video/renderer.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** cLevel *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel :: cLevel( void )
: cFile_parser()
{
	engine_version = -1; // no version

	musicfile.clear();
	valid_music = 0;

	levelfile.clear();
	next_levelfile.clear();

	author.clear();
	version.clear();
	delayed_unload = 0;

	pGlobal_effect = new cGlobal_effect();
}

cLevel :: ~cLevel( void )
{
	Unload();

	if( pGlobal_effect )
	{
		delete pGlobal_effect;
	}
}

int cLevel :: New( string filename )
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
			filename = pPreferences->level_dir + "/new_" + int_to_string( i ) + ".smclvl";
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
		if( filename.find( ".smclvl" ) == string::npos )
		{
			filename.insert( filename.length(), ".smclvl" );
		}

		if( filename.find( pPreferences->level_dir ) == string::npos )
		{
			filename.insert( 0, pPreferences->level_dir + "/" );
		}
	}

	// open file
	ifs.open( filename.c_str(), ios::in );

	// Level does not exists
	if( !ifs )
	{
		Set_Levelfile( filename, 0 );
		engine_version = 2;

		// set default music
		musicfile = MUSIC_DIR "/" LEVEL_DEFAULT_MUSIC;
		valid_music = valid_file( musicfile );

		// todo : remove save ?
		Save();
		return 1;
	}

	// Level already exists
	ifs.close();
	return 0;
}

bool cLevel :: Load( string filename, bool delayed /* = 0 */ )
{
	// erase old file type if set
	if( filename.rfind( ".txt" ) != string::npos )
	{
		filename.erase( filename.rfind( "." ) );
	}

	// use new file type as default
	if( filename.find( ".smclvl" ) == string::npos )
	{
		filename.insert( filename.length(), ".smclvl" );
	}

	// add level directory
	if( filename.find( pPreferences->level_dir ) == string::npos )
	{
		filename.insert( 0, pPreferences->level_dir + "/" );
	}

	next_levelfile.clear();

	if( !valid_file( filename ) )
	{
		// erase file type and use the old
		filename.erase( filename.rfind( "." ) );
		filename.insert( filename.length(), ".txt" );

		if( !valid_file( filename ) )
		{
			// show error without file type
			filename.erase( filename.rfind( "." ) );

			printf( "Couldn't load Level : %s\n", filename.c_str() );
			return 0;
		}
	}

	if( delayed )
	{
		next_levelfile = filename;
		return 1;
	}

	Unload();

	levelfile = filename;

	// set default music
	musicfile = MUSIC_DIR "/" LEVEL_DEFAULT_MUSIC;
	valid_music = valid_file( musicfile );

	// new level format
	if( filename.rfind( ".smclvl" ) != string::npos )
	{
		try
		{
			System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), SCHEMA_DIR "/Level.xsd", "" );
		}
		// catch CEGUI Exceptions
		catch( Exception &ex )
		{
			printf( "Level Loading CEGUI Exception %s\n", ex.getMessage().c_str() );
			debugdisplay->Set_Text( "Level Loading failed : " + (string)ex.getMessage().c_str() );
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

	// init global effect
	pGlobal_effect->Init();

	// engine version entry not set
	if( engine_version < 0 )
	{
		engine_version = 0;
	}

	// hud reset
	pHudManager->Load();

	pAudio->FadeOutMusic( 1000 );
	if( valid_music )
	{
		pAudio->PlayMusic( musicfile, -1, 0, 1000 );
	}
	else if( pAudio->music_enabled )
	{
		printf( "Warning : Music file not found %s\n", musicfile.c_str() );
	}

	pFramerate->Reset();

	// center camera position
	pCamera->Center();

	return 1;
}

void cLevel :: Save( void )
{
	pAudio->PlaySound( "editor/save.ogg" );

	// check if old filename ending
	if( levelfile.rfind( ".txt" ) != string::npos )
	{
		levelfile.erase( levelfile.rfind( ".txt" ) );
		levelfile.insert( levelfile.length(), ".smclvl" );
	}

	ofstream file( levelfile.c_str(), ios::out | ios::trunc );

	if( !file )
	{
		debugdisplay->Set_Text( "Couldn't save level " + levelfile );
		return;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin level
	file << "<level>" << std::endl;

	// begin info
	file << "\t<information>" << std::endl;
		// game version
		file << "\t\t<Property name=\"game_version\" value=\"" << VERSION << "\" />" << std::endl;
		// engine version
		file << "\t\t<Property name=\"engine_version\" value=\"" << 2.1 << "\" />" << std::endl;
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
		file << "\t\t<Property name=\"cam_limit_x\" value=\"" << (int)pCamera->limit_rect.x << "\" />" << std::endl;
		file << "\t\t<Property name=\"cam_limit_y\" value=\"" << (int)pCamera->limit_rect.y << "\" />" << std::endl;
		file << "\t\t<Property name=\"cam_limit_w\" value=\"" << (int)pCamera->limit_rect.w << "\" />" << std::endl;
		file << "\t\t<Property name=\"cam_limit_h\" value=\"" << (int)pCamera->limit_rect.h << "\" />" << std::endl;
	// end settings
	file << "\t</settings>" << std::endl;

	// backgrounds
	for( BackgroundList::iterator itr = backgrounds.begin(), itr_end = backgrounds.end(); itr != itr_end; ++itr )
	{
		(*itr)->Save_to_Stream( file );
	}
	// global effect
	pGlobal_effect->Save_to_Stream( file );
	// player
	pPlayer->Save_to_Stream( file );
	// objects
	for( ObjectList::iterator itr = pObjManager->items.begin(), itr_end = pObjManager->items.end(); itr != itr_end; ++itr )
	{
		cSprite *obj = (*itr);

		// skip spawned objects
		if( obj->spawned )
		{
			continue;
		}

		// save to file stream
		obj->Save_to_Stream( file );
	}

	// end level
	file << "</level>" << std::endl;
	file.close();

	debugdisplay->Set_Text( "Level " + Get_filename( data_file, 0, 0 ) + " saved" );
}

void cLevel :: Process_Input( void )
{
	// only non editor
	if( !editor_level_enabled )
	{
		// todo : move to player class
		// player direction
		if( pKeyboard->keys[pPreferences->key_left] || pJoystick->left )
		{
			if( pPlayer->state != STA_FLY )
			{
				if( pPlayer->direction != DIR_LEFT )
				{
					// play stop sound if already running
					if( pPlayer->velx > 12 && pPlayer->ground_object )
					{
						pAudio->PlaySound( "player/maryo_stop.ogg", RID_MARYO_STOP );
					}

					pPlayer->direction = DIR_LEFT;
				}
			}
		}
		else if( pKeyboard->keys[pPreferences->key_right] || pJoystick->right )
		{
			if( pPlayer->state != STA_FLY )
			{
				if( pPlayer->direction != DIR_RIGHT )
				{
					// play stop sound if already running
					if( pPlayer->velx < -12 && pPlayer->ground_object )
					{
						pAudio->PlaySound( "player/maryo_stop.ogg", RID_MARYO_STOP );
					}

					pPlayer->direction = DIR_RIGHT;
				}
			}
		}

		// todo : move to player class
		// player ducking / falling
		if( pKeyboard->keys[pPreferences->key_down] || pJoystick->down )
		{
			if( pPlayer->state != STA_FLY )
			{
				if( pPlayer->ground_object )
				{
					if( pPlayer->ground_object->massivetype == MASS_MASSIVE )
					{
						pPlayer->Start_ducking();
					}
					else if( pPlayer->ground_object->massivetype == MASS_HALFMASSIVE )
					{
						pPlayer->Start_falling();
					}
				}
			}
		}
	}
}

bool cLevel :: Key_Down( SDLKey key )
{
	// debug key F2
	if( key == SDLK_F2 && Game_debug && !editor_level_enabled )
	{
		pPlayer->Set_Type( MARYO_CAPE, 0 );
	}
	// debug key F3
	else if( key == SDLK_F3 && !editor_level_enabled )
	{
		//pPlayer->GotoNextLevel();
		//DrawEffect( HORIZONTAL_VERTICAL_FADE );
		//pPlayer->Draw_Animation( MARYO_FIRE );

		cParticleAnimation *anim = new cParticleAnimation( pPlayer->posx + (float)( pPlayer->col_rect.w / 2 ), pPlayer->posy - 100 );
		anim->Set_Emitter_Time_to_Live( 2 );
		anim->Set_Emitter_Iteration_Interval( 0.1f );
		anim->Set_Quota( 2 );
		anim->Set_Image( pVideo->Get_Surface( "animation/particles/star.png" ) );
		anim->Set_Time_to_Live( 3, 3 );
		anim->Set_Fading_Alpha( 1 );
		anim->Set_Speed( 2, 8 );
		anim->Set_Scale( 0.5f );
		anim->Set_ConstRotationZ( 0.5f, 15 );
		pAnimationManager->Add( anim );
	}
	// debug key F4
	else if( key == SDLK_F4 && !editor_level_enabled )
	{
		/*for( unsigned int i = 0; i < pObjManager->Get_Size(); i++ )
		{
			if( pObjManager->Get_Pointer( i )->type == TYPE_TURTLE_BOSS )
			{
				cTurtleBoss *boss = (cTurtleBoss *)pObjManager->Get_Pointer( i );

				boss->Throw_Fireballs();
			}
		}*/

		/*if( pGlobal_effect->type != GL_EFF_FALLING )
		{
			pGlobal_effect->Set_image( "animation/particles/rain.png" );
			pGlobal_effect->Set_type( GL_EFF_FALLING );
			pGlobal_effect->Set_Lifetime_mod( 4 );
			pGlobal_effect->Set_Scale( 0.5f, 0.5f );
			pGlobal_effect->Set_Creation_Speed( 2 );
			pGlobal_effect->Set_Speed( 13, 5 );
			pGlobal_effect->Set_Direction( 90, 0 );
			pGlobal_effect->Set_ConstRotationZ( 0, 0 );
			pGlobal_effect->Init();
		}*/
	}
	// toggle leveleditor
	else if( key == SDLK_F8 )
	{
		pLevel_Editor->Toggle();
	}
	// ## only game
	else if( key == pPreferences->key_shoot && !editor_enabled )
	{
		pPlayer->Action_Shoot();
	}
	else if( key == pPreferences->key_jump && !editor_enabled )
	{
		pPlayer->Action_Jump();
	}
	else if( key == pPreferences->key_action && !editor_enabled )
	{
		pPlayer->Action_Interact();
	}
	else if( key == SDLK_RETURN && !editor_enabled )
	{
		Itembox->Request_Item();
	}
	else if( pKeyboard->keys[SDLK_g] && pKeyboard->keys[SDLK_o] && pKeyboard->keys[SDLK_d] && !editor_enabled )
	{
		if( pPlayer->godmode )
		{
			debugdisplay->Set_Text( "Funky Godmode disabled" );
		}
		else
		{
			debugdisplay->Set_Text( "Funky Godmode enabled" );
		}

		pPlayer->godmode = !pPlayer->godmode;
	}
	else if( pKeyboard->keys[SDLK_k] && pKeyboard->keys[SDLK_i] && pKeyboard->keys[SDLK_d] && !editor_enabled )
	{
		pPlayer->Set_Type( MARYO_SMALL, 0 );
	}
	// enter menu
	else if( key == SDLK_ESCAPE )
	{
		pMenuCore->Enter();
	}
	// ## editor
	else if( pLevel_Editor->Key_Down( key )  )
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
	// only non Leveleditor
	if( !editor_level_enabled )
	{
		if( key == pPreferences->key_right || key == pPreferences->key_left )
		{
			pPlayer->Hold();
		}
		else if( key == pPreferences->key_down )
		{
			pPlayer->Stop_ducking();
		}
		else if( key == pPreferences->key_jump )
		{
			pPlayer->Stop_flying();
			pPlayer->UpKeytime = 0;
		}
		else if( key == pPreferences->key_action )
		{
			pPlayer->Release_item();
		}
		else
		{
			// not processed
			return 0;
		}
	}

	// key got processed
	return 1;
}

void cLevel :: Show_Error( string errortext )
{
	GL_Surface *image = pFont->RenderText( pFont->font_normal, errortext, black );
	cHudSprite *temp = new cHudSprite( image, GAME_RES_W / 2 - image->w / 2, GAME_RES_H / 2 - image->h / 2, 1 );

	temp->Draw();
	pVideo->Render();
	SDL_Delay( 4000 ); // todo with pollevent for ESC and a counter
	delete temp;

	exit( 1 );
}

unsigned int cLevel :: Get_Backgroundlayers( void )
{
	return backgrounds.size();
}

cBackground *cLevel :: Get_background( unsigned int num )
{
	if( num >= backgrounds.size() )
	{
		return NULL;
	}

	return backgrounds[num];
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
		// Objects
		pObjManager->Update_items();

		// Animations
		pAnimationManager->Update();

		// global effect
		pGlobal_effect->Update();
		pGlobal_effect->Update_anim();
	}
}

void cLevel :: Draw_Layer1( LevelDrawType type /* = LVL_DRAW */ )
{
	// with background
	if( type != LVL_DRAW_NO_BG )
	{
		for( BackgroundList::iterator itr = backgrounds.begin(), itr_end = backgrounds.end(); itr != itr_end; ++itr )
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
	pObjManager->Draw_items();
	// Animations
	pAnimationManager->Draw();
}

void cLevel :: Draw_Layer2( LevelDrawType type /* = LVL_DRAW */ )
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
		pVideo->Draw_Rect( 0, 0, GAME_RES_W, GAME_RES_H, 0.12f, &color, request );

		request->combine_type = GL_MODULATE;

		request->combine_col[0] = 0.9f;
		request->combine_col[1] = 0.6f;
		request->combine_col[2] = 0.8f;


		// add request
		pRenderer->Add( request );
	}
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

	pObjManager->Delete_All();
	pAnimationManager->Delete_All();

	pHudManager->Update();
	pJoystick->Reset_keys();

	// backgrounds
	for( BackgroundList::iterator itr = backgrounds.begin(), itr_end = backgrounds.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}
	backgrounds.clear();

	// default gradient layer
	backgrounds.push_back( new cBackground() );
	backgrounds[0]->Set_type( BG_GR_VER );
	backgrounds[0]->posz = 0.0001f;

	// reset music
	musicfile.clear();
	valid_music = 0;

	author.clear();
	version.clear();

	// reset camera limits
	pCamera->Reset_Limits();

	// clear global effect
	pGlobal_effect->Clear();

	if( timedisplay )
	{
		timedisplay->counter = -1;
	}

	if( pMouseCursor )
	{
		pMouseCursor->Reset( 0 );
	}

	engine_version = -1; // no version

	levelfile.clear();
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
	else if( with_dir == 1 && filename.find( MUSIC_DIR ) != string::npos )
	{
		filename.erase( 0, strlen( MUSIC_DIR ) + 1 );
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

	Convert_path_separators( filename );

	// add music dir
	if( filename.find( MUSIC_DIR ) == string::npos )
	{
		filename.insert( 0, MUSIC_DIR "/" );
	}

	musicfile = filename;
	// check if music is available
	valid_music = valid_file( filename );
}

void cLevel :: Set_Levelfile( string filename, bool delete_old /* = 1 */ )
{
	// if invalid
	if( filename.length() < 2 )
	{
		return;
	}

	// delete file with the old name
	if( delete_old )
	{
		Delete_file( levelfile );
	}

	// add level file type
	if( filename.find( ".smclvl" ) == string::npos )
	{
		filename.insert( filename.length(), ".smclvl" );
	}

	Convert_path_separators( filename );

	levelfile = filename;
	data_file = filename;

	// add level dir
	if( levelfile.find( pPreferences->level_dir ) == string::npos )
	{
		levelfile.insert( 0, pPreferences->level_dir + "/" );
	}

	// save with new filename
	Save();
}

void cLevel :: Set_Author( string name )
{
	author = name;
}

void cLevel :: Set_Version( string level_version )
{
	version = level_version;
}

bool cLevel :: is_Loaded( void )
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
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		pPlayer->Set_Pos( (float)string_to_int( parts[1] ), (float)GAME_RES_H - (float)( string_to_int( parts[2] ) + pPlayer->image->h ), 1 );
	}
	else if( parts[0].compare( "author" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		author = parts[1];
	}
	else if( parts[0].compare( "version" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
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
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( parts[1].length() < 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
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

		if( read_color_info( parts, count, line, color ) == 0 )
		{
			return 0;
		}

		backgrounds[0]->Set_color_1( color );
		backgrounds[0]->Set_color_2( color );
	}
	else if( parts[0].compare( "Background_color_1" ) == 0 ) // top background color
	{
		Color color;

		if( !read_color_info( parts, count, line, color ) )
		{
			return 0;
		}

		backgrounds[0]->Set_color_1( color );
	}
	else if( parts[0].compare( "Background_color_2" ) == 0 ) // bottom background color
	{
		Color color;

		if( !read_color_info( parts, count, line, color ) )
		{
			return 0;
		}

		backgrounds[0]->Set_color_2( color );
	}
	/***************************************************************************/
	else if( parts[0].find( "Background_image" ) == 0 && parts[0].length() <= 18 )
	{
		if( count < 2 || count > 6 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s %s\n", parts[0].c_str(), "needs 2-6 parameters" );
			return 0; // error
		}

		if( parts[1].length() < 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Not a valid %s file\n", parts[0].c_str() );
			return 0;  // error
		}

		string filename;

		filename.reserve( parts[1].length() + 50 );
		filename = parts[1];

		if( filename.find( PIXMAPS_DIR ) == string::npos )
		{
			filename.insert( 0, PIXMAPS_DIR "/" );
		}

		if( !valid_file( filename ) )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s file not found : %s\n", parts[0].c_str(), filename.c_str() );

			return 0; // error
		}

		cBackground *background = new cBackground();
		background->Set_image( parts[1] );

		// type is given
		if( count > 2 )
		{
			unsigned int bg_type = string_to_int( parts[2] );

			if( bg_type < 0 || bg_type > 200 )
			{
				printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "Not a valid Background image type ( %d )\n", bg_type );
				return 0;  // error
			}

			background->Set_type( (BackgroundType)bg_type );


		}
		// default type
		else
		{
			background->Set_type( BG_IMG_LEFTRIGHT );
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
			background->Set_scroll_speed( speedx, speedy );
		}

		// z position is given
		if( count > 5 )
		{
			background->Set_PosZ( string_to_float( parts[4] ) );
		}

		backgrounds.push_back( background );
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
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( parts[2].length() < 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Not a valid %s file\n", parts[0].c_str() );
			return 0;  // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		string filename;

		filename.reserve( parts[2].length() + 50 );
		filename = parts[2];

		if( filename.find( PIXMAPS_DIR ) == string::npos )
		{
			filename.insert( 0, PIXMAPS_DIR "/" );
		}

		if( !valid_file( filename ) )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s file not found : %s\n", parts[0].c_str(), filename.c_str() );

			return 0; // error
		}

		// remove pixmaps dir
		if( filename.find( PIXMAPS_DIR ) == 0 )
		{
			filename.erase( 0, strlen( PIXMAPS_DIR ) + 1 );
		}

		int effect_type = string_to_int( parts[1] );

		pGlobal_effect->Set_type( (GlobalEffectType)effect_type );
		pGlobal_effect->Set_image( filename );
	}
	/***************************************************************************/
	else if( parts[0].find( "ge_rect" ) == 0 )
	{
		pGlobal_effect->Set_rect( GL_rect( string_to_float( parts[1] ), string_to_float( parts[2] ), string_to_float( parts[3] ), string_to_float( parts[4] ) ) );
	}
	else if( parts[0].find( "ge_lifetime_mod" ) == 0 )
	{
		pGlobal_effect->Set_Time_to_Live( string_to_float( parts[1] ) * 0.3f );
	}
	else if( parts[0].find( "ge_creation_speed" ) == 0 )
	{
		pGlobal_effect->Set_Creation_Speed( string_to_float( parts[1] ) );
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
		pGlobal_effect->Set_ConstRotationZ( string_to_float( parts[1] ), string_to_float( parts[2] ) );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Levelengine_version" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		engine_version = (float)string_to_int( parts[1] );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Sprite" ) == 0 || parts[0].compare( "ClimbSprite" ) == 0 )
	{
		if( count != 5 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 5 parameters" );
			return 0; // error
		}

		// check if the image file exists
		string filename = parts[1];

		while( filename.find( "/" ) == 0 )
		{
			filename.erase( 0, 1 );
		}

		if( filename.find( PIXMAPS_DIR ) == string::npos )
		{
			filename.insert( 0, PIXMAPS_DIR "/" );
		}

		string settings_file = filename;
		settings_file.erase( settings_file.rfind( "." ) + 1 );
		settings_file.insert( settings_file.rfind( "." ) + 1, "settings" );

		if( !valid_file( filename ) && !valid_file( settings_file ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "file not found : %s\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[3] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[3].c_str() );
			return 0; // error
		}
		if( !parts[4].compare( "MASSIVE" ) == 0 && !parts[4].compare( "PASSIVE" ) == 0 && !parts[4].compare( "FRONT_PASSIVE" ) == 0 && !parts[4].compare( "HALFMASSIVE" ) == 0 )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "parameter five has to be MASSIVE, PASSIVE, or HALFMASSIVE\n" );
			return 0; // error
		}

		string getimage_path = filename;
		getimage_path.erase( 0, strlen( PIXMAPS_DIR ) + 1 );

		GL_Surface *surface = pVideo->Get_Surface( getimage_path );
		int posx = string_to_int( parts[2] );
		int posy = GAME_RES_H - string_to_int( parts[3] );

		// if image is obsolete show a warning
		if( surface->obsolete )
		{
			printf( "%s : line %d Warning :\n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "using obsolete image : %s\nAt position x : %d, y : %d\n", getimage_path.c_str(), posx, posy );
		}

		cSprite *temp = new cSprite( surface, (float)posx, (float)posy );

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

		pObjManager->Add( temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Enemy" ) == 0 )
	{
		if( count < 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs more than 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[3] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[3].c_str() );
			return 0; // error
		}

		// Gumba
		if( parts[1].compare( "GOOMBA" ) == 0 )
		{
			/* 3 and 4 is a very old description
			 * 5 is lower than V.0.98
			*/
			if( !( count == 3 || count == 4 || count == 5 || count == 6 ) )
			{
				printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 5 parameters" );
				return 0; // error
			}

			if( count >= 5 ) // color support
			{
				cGumba *temp = NULL;
				DefaultColor color = (DefaultColor)string_to_int( parts[4] );

				// correct old color types
				if( count == 5 )
				{
					// Red
					if( color == 1 )
					{
						color = COL_RED;
					}
					// Brown
					else if( color == 0 )
					{
						color = COL_BROWN;
					}
				}

				temp = new cGumba( (float)string_to_int( parts[2] ), (float)( GAME_RES_H - string_to_int( parts[3] ) ) );
				temp->Set_Color( color );

				if( count >= 6 ) // direction support
				{
					if( !is_valid_number( parts[5] ) )
					{
						printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
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
					pObjManager->Add( (cMovingSprite *)temp );
				}
			}
			else // support for old levels ( lower than V.0.8 )
			{
				cGumba *temp = new cGumba( (float)string_to_int( parts[2] ), (float)( GAME_RES_H - string_to_int( parts[3] ) ) );
				pObjManager->Add( (cMovingSprite *)temp );
			}

		}
		// Turtle
		// RTURTLE for old level support
		else if( parts[1].compare( "RTURTLE" ) == 0 || parts[1].compare( "TURTLE" ) == 0 )
		{
			if( count < 4 || count > 5 )
			{
				printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 4-5 parameters" );
				return 0; // error
			}

			ObjectDirection direction = DIR_RIGHT;

			// with direction
			if( count > 4 )
			{
				direction = (ObjectDirection)string_to_int( parts[4] );
			}

			cTurtle *temp = new cTurtle( (float)string_to_int( parts[2] ), (float)( GAME_RES_H - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			pObjManager->Add( (cMovingSprite *)temp );
		}
		// Jumping Piranha
		else if( parts[1].compare( "JPIRANHA" ) == 0 )
		{
			if( count < 4 || count > 7 )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
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
					printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
					printf( "%s is not a valid integer value\n", parts[4].c_str() );
					return 0; // error
				}

				if( !is_valid_number( parts[5] ) )
				{
					printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
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

			cjPiranha *temp = new cjPiranha( (float)string_to_int( parts[2] ), (float)( GAME_RES_H - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			temp->Set_Max_Distance( max_distance );
			temp->Set_Speed( speed );
			pObjManager->Add( (cMovingSprite *)temp );
		}
		// Rokko
		// BANZAI_BILL was the old name
		else if( parts[1].compare( "ROKKO" ) == 0 || parts[1].compare( "BANZAI_BILL" ) == 0 )
		{
			if( count < 5 || count > 6 )
			{
				printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "Line %d, error : %s %s\n", line, parts[1].c_str(), "needs 5-6 parameters" );
				return 0; // error
			}

			float speed = 8.5f;

			// with speed
			if( count > 5 )
			{
				speed = string_to_float( parts[5] );
			}

			cRokko *temp = new cRokko( (float)string_to_int( parts[2] ), (float)( GAME_RES_H - string_to_int( parts[3] ) ) );
			temp->Set_Direction( (ObjectDirection)string_to_int( parts[4] ) );
			temp->Set_Speed( speed );
			pObjManager->Add( (cMovingSprite *)temp );
		}
		// Rex
		else if( parts[1].compare( "REX" ) == 0 )
		{
			if( count < 4 || count > 5 )
			{
				printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "Line %d, error : %s %s\n", line, parts[1].c_str(), "needs 4-5 parameters" );
				return 0; // error
			}

			ObjectDirection direction = DIR_RIGHT;

			// with direction
			if( count > 4 )
			{
				direction = (ObjectDirection)string_to_int( parts[4] );
			}

			cRex *temp = new cRex( (float)string_to_int( parts[2] ), (float)( GAME_RES_H - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			pObjManager->Add( (cMovingSprite *)temp );
		}
		// Thromp
		// Thwomp was the old name
		else if( parts[1].compare( "THROMP" ) == 0 || parts[1].compare( "THWOMP" ) == 0 )
		{
			if( count < 6 || count > 7 )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 6-7 parameters" );
				return 0; // error
			}

			if( !is_valid_number( parts[4] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[4].c_str() );
				return 0; // error
			}

			if( !is_valid_number( parts[5] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
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

			cThromp *temp = new cThromp( (float)string_to_int( parts[2] ), (float)( GAME_RES_H - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			temp->Set_Max_Distance( max_distance );
			temp->Set_Speed( speed );

			pObjManager->Add( (cMovingSprite *)temp );
		}
		// Eato
		else if( parts[1].compare( "EATO" ) == 0 )
		{
			if( count != 5 )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 5 parameters" );
				return 0; // error
			}

			if( !is_valid_number( parts[4] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[4].c_str() );
				return 0; // error
			}

			ObjectDirection direction = (ObjectDirection)string_to_int( parts[4] );

			cEato *temp = new cEato( (float)string_to_int( parts[2] ), (float)( GAME_RES_H - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			pObjManager->Add( (cMovingSprite *)temp );
		}
		// Gee
		else if( parts[1].compare( "GEE" ) == 0 )
		{
			if( count < 6 || count > 7 )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 6-7 parameters" );
				return 0; // error
			}

			if( !is_valid_number( parts[4] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[4].c_str() );
				return 0; // error
			}

			if( !is_valid_number( parts[5] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
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

			cGee *temp = new cGee( (float)string_to_int( parts[2] ), (float)( GAME_RES_H - string_to_int( parts[3] ) ) );
			temp->Set_Direction( direction );
			temp->Set_Max_Distance( max_distance );
			temp->Set_Color( color );
			pObjManager->Add( (cMovingSprite *)temp );
		}
		else
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Line %d, error : %s %s\n", line, parts[1].c_str(), "Unknown Enemy type" );
			return 0; // error
		}
	}
	/***************************************************************************/
	else if( parts[0].compare( "Goldpiece" ) == 0 )
	{
		if( count < 3 || count > 4 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-4 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		DefaultColor color = COL_YELLOW;

		// with color information
		if( count > 3 )
		{
			if( !is_valid_number( parts[3] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[3].c_str() );
				return 0; // error
			}

			color = (DefaultColor)string_to_int( parts[3] );
		}

		cGoldpiece *temp = new cGoldpiece( (float)string_to_int( parts[1] ), (float)( GAME_RES_H - string_to_int( parts[2] ) ) );
		temp->Set_Goldcolor( color );
		pObjManager->Add( (cMovingSprite *)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Moon" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		cMoon *temp = new cMoon( (float)string_to_int( parts[1] ), (float)( GAME_RES_H - string_to_int( parts[2] ) ) );
		pObjManager->Add( (cMovingSprite*)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Levelexit" ) == 0 )
	{
		if( count < 3 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-5 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		cLevelExit *temp = NULL;

		// with type and level
		if( count == 4 || count == 5 )
		{
			if( !is_valid_number( parts[3] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[2].c_str() );
				return 0; // error
			}

			temp = new cLevelExit( (float)string_to_int( parts[1] ), (float)( GAME_RES_H - string_to_int( parts[2] ) ) );

			// type
			temp->Set_Type( (Levelchange_type)string_to_int( parts[3] ) );

			// with a given level
			if( count == 5 )
			{
				string level = parts[4];

				if( level.find( pPreferences->level_dir ) == string::npos )
				{
					level.insert( 0, pPreferences->level_dir + "/" );
				}

				if( level.find( ".txt" ) == string::npos )
				{
					level.insert( level.length(), ".txt" );
				}

				// if not found
				if( !valid_file( level ) )
				{
					printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
					printf( "Sublevel not found : %s\n", level.c_str() );
				}
				else
				{
					temp->Set_Level( level );
				}
			}
		}
		else // older level support
		{
			// add a door sprite
			cMovingSprite *door_sprite = new cMovingSprite( pVideo->Get_Surface( "game/level/door_yellow_1.png" ), (float)string_to_int( parts[1] ), (float)( GAME_RES_H - string_to_int( parts[2] ) ) );
			door_sprite->Set_Sprite_Type( TYPE_PASSIVE );
			pObjManager->Add( door_sprite );
			// add the levelexit
			temp = new cLevelExit( (float)( string_to_int( parts[1] ) + 13 ), (float)( GAME_RES_H - string_to_int( parts[2] ) + 60 ) );
		}

		pObjManager->Add( (cMovingSprite *)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "BonusBox" ) == 0 )
	{
		if( !( count == 3 || count == 4 ) )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-4 parameters" );
			return 0; // error
		}

		if( count == 3 ) // old Level Support
		{
			if( !is_valid_number( parts[1] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[1].c_str() );
				return 0; // error
			}
			if( !is_valid_number( parts[2] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[2].c_str() );
				return 0; // error
			}

			cBonusBox *temp = new cBonusBox( (float)string_to_int( parts[1] ), (float)( GAME_RES_H - string_to_int( parts[2] ) ) );
			pObjManager->Add( (cMovingSprite *)temp );
		}
		else if( count == 4 )
		{
			if( !is_valid_number( parts[1] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[1].c_str() );
				return 0; // error
			}
			if ( !is_valid_number( parts[2] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[2].c_str() );
				return 0; // error
			}

			cBonusBox *temp = new cBonusBox( (float)string_to_int( parts[1] ), (float)( GAME_RES_H - string_to_int( parts[2] ) ) );

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
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "Bonusbox type Error %s\n", parts[3].c_str() );
				return 0; // error
			}

			pObjManager->Add( (cMovingSprite *)temp );
		}
		else
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Bonusbox count Error %d\n", count );
			return 0; // error
		}
	}
	/***************************************************************************/
	else if( parts[0].compare( "GoldBox" ) == 0 )
	{
		if( count < 3 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-5 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		DefaultColor color = COL_YELLOW;

		if( count > 3 ) // with color information
		{
			if( !is_valid_number( parts[3] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[3].c_str() );
				return 0; // error
			}

			color = (DefaultColor)string_to_int( parts[3] );
		}

		cBonusBox *temp = new cBonusBox( (float)string_to_int( parts[1] ), (float)( GAME_RES_H - string_to_int( parts[2] ) ) );
		temp->Set_Animation( "Default" );
		temp->Set_Bonus_Type( TYPE_GOLDPIECE );
		temp->Set_Goldcolor( color );
		pObjManager->Add( (cMovingSprite *)temp );

		// with useable times information
		if( count > 4 )
		{
			if( !is_valid_number( parts[4] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[4].c_str() );
				return 0; // error
			}

			temp->Set_UseableCount( string_to_int( parts[4] ), 1 );
		}
	}
	/***************************************************************************/
	else if( parts[0].compare( "SpinBox" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		cSpinBox *temp = new cSpinBox( (float)string_to_int( parts[1] ), (float)( GAME_RES_H - string_to_int( parts[2] ) ) );
		pObjManager->Add( (cMovingSprite *)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Cloud" ) == 0 )
	{
		if( count < 3 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-5 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		// convert to halfmassive
		cMovingSprite *temp = new cMovingSprite( pVideo->Get_Surface( "clouds/default_1/1_middle.png" ), (float)string_to_int( parts[1] ), (float)( GAME_RES_H - string_to_int( parts[2] ) ) );
		temp->Set_Sprite_Type( TYPE_HALFMASSIVE );

		pObjManager->Add( temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "EnemyStopper" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}
		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		cEnemyStopper *temp = new cEnemyStopper( (float)string_to_int( parts[1] ), (float)( GAME_RES_H - string_to_int( parts[2] ) ) );
		pObjManager->Add( (cMovingSprite *)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "movingplatform" ) == 0 )
	{
		if( count < 10 || count > 10 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 10 parameters" );
			return 0; // error
		}

		cMoving_Platform *temp = new cMoving_Platform( (float)string_to_int( parts[1] ), (float)(GAME_RES_H - string_to_int( parts[2] )) );

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

		temp->Set_image_top_left( pVideo->Get_Surface( parts[7] ) );
		temp->Set_image_top_middle( pVideo->Get_Surface( parts[8] ) );
		temp->Set_image_top_right( pVideo->Get_Surface( parts[9] ) );

		pObjManager->Add( (cMovingSprite *)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "camera_limits" ) == 0 )
	{
		if( count < 5 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 5 parameters" );
			return 0; // error
		}

		pCamera->Set_Limits( GL_rect( (float)string_to_int( parts[1] ), (float)string_to_int( parts[2] ), (float)string_to_int( parts[3] ), (float)string_to_int( parts[4] ) ) );
	}
	/***************************************************************************/
	else
	{
		printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
		printf( "Unknown Command : %s\n", parts[0].c_str() );
		return 0; // error
	}

	return 1; // Succesful
}

bool cLevel :: read_color_info( string *parts, unsigned int count, unsigned int line, Color &color )
{
	if( count != 4 )
	{
		printf( "%s : line %d Error : \n", Get_filename( data_file, 0, 0 ).c_str(), line );
		printf( "Error : %s %s\n", parts[0].c_str(), "needs 4 parameters" );
		return 0; // error
	}

	if( !is_valid_number( parts[1] ) )
	{
		printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
		printf( "%s is not a valid integer value\n", parts[1].c_str() );
		return 0; // error
	}
	if( !is_valid_number( parts[2] ) )
	{
		printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
		printf( "%s is not a valid integer value\n", parts[2].c_str() );
		return 0; // error
	}
	if( !is_valid_number( parts[3] ) )
	{
		printf( "%s : line %d Error : ", Get_filename( data_file, 0, 0 ).c_str(), line );
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
void cLevel :: elementStart( const String &element, const XMLAttributes &attributes )
{
	// Property/Item/Tag of an Element
    if( element == "Property" )
    {
		xml_attributes.add( attributes.getValueAsString( "name" ), attributes.getValueAsString( "value" ) );
    }
}

// XML element end
void cLevel :: elementEnd( const String &element )
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
			pCamera->Set_Limits( GL_rect( (float)xml_attributes.getValueAsInteger( "cam_limit_x" ), (float)(float)xml_attributes.getValueAsInteger( "cam_limit_y" ), (float)(float)xml_attributes.getValueAsInteger( "cam_limit_w" ), (float)(float)xml_attributes.getValueAsInteger( "cam_limit_h" ) ) );
		}
		else if( element == "background" )
		{
			BackgroundType bg_type = (BackgroundType)xml_attributes.getValueAsInteger( "type" );

			// use gradient background
			if( bg_type == BG_GR_HOR || bg_type == BG_GR_VER )
			{
				backgrounds[0]->Set_type( bg_type );
				backgrounds[0]->Set_color_1( Color( (Uint8)xml_attributes.getValueAsInteger( "bg_color_1_red" ), xml_attributes.getValueAsInteger( "bg_color_1_green" ), xml_attributes.getValueAsInteger( "bg_color_1_blue" ) ) );
				backgrounds[0]->Set_color_2( Color( (Uint8)xml_attributes.getValueAsInteger( "bg_color_2_red" ), xml_attributes.getValueAsInteger( "bg_color_2_green" ), xml_attributes.getValueAsInteger( "bg_color_2_blue" ) ) );
			}
			// default background
			else
			{
				backgrounds.push_back( new cBackground( xml_attributes ) );
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
			cSprite *object = Get_Level_Object( element, xml_attributes );
			
			// valid
			if( object )
			{
				pObjManager->Add( object );
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
		xml_attributes = XMLAttributes();
	}
}

cSprite *Get_Level_Object( const String &element, XMLAttributes &attributes )
{
	if( element == "sprite" )
	{
		return new cSprite( attributes );
	}
	else if( element == "enemystopper" )
	{
		return new cEnemyStopper( attributes );
	}
	else if( element == "levelexit" )
	{
		return new cLevelExit( attributes );
	}
	else if( element == "box" )
	{
		String str_type = attributes.getValueAsString( "type" );

		if( str_type == "bonus" )
		{
			return new cBonusBox( attributes );
		}
		else if( str_type == "gold" )
		{
			// update old values
			attributes.add( "type", "bonus" );
			attributes.add( "animation", "Default" );
			attributes.add( "item", int_to_string( TYPE_GOLDPIECE ) );
			attributes.add( "gold_color", attributes.getValue( "color" ) );

			return new cBonusBox( attributes );
		}
		else if( str_type == "spin" )
		{
			return new cSpinBox( attributes );
		}
		else if ( str_type == "empty" )
		{
			// update old values
			attributes.add( "type", "bonus" );
			attributes.add( "item", "0" );

		    return new cBonusBox( attributes );
		}
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
		String str_type = attributes.getValueAsString( "type" );

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
	else if( element == "falling_platform" )
	{
	    return new cFalling_Platform( attributes );
	}
	else if( element == "enemy" )
	{
		String str_type = attributes.getValueAsString( "type" );

		if( str_type == "eato" )
		{
			return new cEato( attributes );
		}
		else if( str_type == "gumba" )
		{
			return new cGumba( attributes );
		}
		else if( str_type == "turtle" )
		{
			return new cTurtle( attributes );
		}
		else if( str_type == "turtleboss" )
		{
			return new cTurtleBoss( attributes );
		}
		else if( str_type == "jpiranha" )
		{
			return new cjPiranha( attributes );
		}
		else if( str_type == "thromp" )
		{
			return new cThromp( attributes );
		}
		else if( str_type == "rokko" )
		{
			return new cRokko( attributes );
		}
		else if( str_type == "rex" )
		{
			return new cRex( attributes );
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

	return NULL;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel *pLevel = NULL;
