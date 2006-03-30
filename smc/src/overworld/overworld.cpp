/***************************************************************************
    overworld.cpp  -  class for the Overworld
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
#include "../main.h"

/* *** *** *** *** *** *** *** *** Overworld handler *** *** *** *** *** *** *** *** *** */

cOverworld_world :: cOverworld_world( void )
{
	name = "Not Loaded";
	dir = "default";
}

cOverworld_world :: ~cOverworld_world( void )
{
	//
}

string cOverworld_world :: Get_WorldFile( void )
{
	string filename = "/world.xml";
	filename.insert( 0, dir );
	filename.insert( 0, "data/world/" );

	return filename;
}

cOverworld_worlds :: cOverworld_worlds( void )
{
	world_filename.insert( 0, "/worlds.xml" );
	world_filename.insert( 0, OVERWORLD_DIR );

	Load();
}

cOverworld_worlds :: ~cOverworld_worlds( void )
{
	Unload();
}

void cOverworld_worlds :: Load( void )
{
	if( !valid_file( world_filename ) )
	{
		printf( "Error : Couldn't open Overworlds description file : %s\n", world_filename.c_str() );
		return;
	}

	System::getSingleton().getXMLParser()->parseXMLFile( *this, world_filename, "data/schemes/WorldsScheme.xsd", "" );
}

void cOverworld_worlds :: Unload( void )
{
	for( unsigned int i = 0; i < worlds.size(); i++ )
	{
		if( worlds[i] )
		{
			delete worlds[i];
		}
	}
	
	worlds.clear();
}

cOverworld_world *cOverworld_worlds :: Get( string name )
{
	for( unsigned int i = 0; i < worlds.size(); i++ )
	{
		if( worlds[i]->name.compare( name ) == 0 )
		{
			return worlds[i];
		}
	}

	return NULL;
}

// XML element start
void cOverworld_worlds :: elementStart( const String &element, const XMLAttributes &attributes )
{
    if( element == "World" )
    {
        handle_world( attributes );
    }
}

// XML element end
void cOverworld_worlds :: elementEnd( const String &element )
{
	
}

void cOverworld_worlds :: handle_world( const XMLAttributes &attributes )
{
	string name = attributes.getValueAsString( "Name" ).c_str();
	string dir = attributes.getValueAsString( "Path" ).c_str();

	cOverworld_world *item = new cOverworld_world();

	item->name = name;
	item->dir = dir;

	worlds.push_back( item );
}

/* *** *** *** *** *** *** *** *** Overworld *** *** *** *** *** *** *** *** *** */


cOverWorld :: cOverWorld( void )
{
	current_world = 0;

	Map = 0;
	Layer = NULL;

	Maryo = new cWorld_Maryo();
	world_handler = new cOverworld_worlds();
	hud_level = new cSprite( NULL, 350, 0, 0, &black );
	hud_level->no_camera = 1;

	// debug images
	debugimage = NULL;
	debug_curr_waypoint = NULL;
	debug_curr_line = NULL;
	debug_nlevel = NULL;
	
	entered = 0;

	debugmode = 0;

	cameramode = 0;
	editormode = 0;
}

cOverWorld :: ~cOverWorld( void )
{
	Unload();

	if( debugimage )
	{
		delete debugimage;
	}

	if( debug_curr_waypoint )
	{
		delete debug_curr_waypoint;
	}

	if( debug_curr_line )
	{
		delete debug_curr_line;
	}

	if( debug_nlevel )
	{
		delete debug_nlevel;
	}

	delete Maryo;
	delete world_handler;
	delete hud_level;
}

void cOverWorld :: Enter( void )
{
	if( done )
	{
		return;
	}

	entered = 0;
	Game_Mode = MODE_OVERWORLD;
	Leveleditor_Mode = 0;

	Maryo->Load_Images();

	pAudio->FadeOutMusic( 1000 );
	pAudio->PlayMusic( MUSIC_DIR "/overworld/land_1.ogg", -1, 0, 3000 );

	if( !MapObjects.size() && !Layer ) // if not loaded
	{
		Load( "default" ); // load the default world
	}
	else // update the hud
	{
		pHudManager->Load();
	}

	cameraposx = 0;
	cameraposy = 0;

	while( !entered )
	{
		while( SDL_PollEvent( &event ) )
		{
			HandleInput();

			switch( event.type )
			{
				case SDL_QUIT:
				{
					entered = 1;
					break;
				}
				default:
				{
					break;
				}
			}
		}

		// ## Input Keys
		keys = SDL_GetKeyState( NULL );

		// ## Update
		Update();

		if( entered )
		{
			continue;
		}

		Draw();

		// ## Frametime
		Framerate.Update();
	}

	if( entered == 2 )
	{
		pMenu->Enter_Menu();
	}

	Framerate.Reset();
	Game_Mode = MODE_LEVEL;
	pHudManager->UpdateText();

	ClearEvents();
}

void cOverWorld :: KeyDown_OW( SDLKey key )
{
	if( key == SDLK_LEFT && !cameramode )
	{
		Maryo->Start_Walk( DIR_LEFT );
	}
	else if( key == SDLK_RIGHT && !cameramode )
	{
		Maryo->Start_Walk( DIR_RIGHT );
	}
	else if( key == SDLK_UP && !cameramode )
	{
		Maryo->Start_Walk( DIR_UP );
	}
	else if( key == SDLK_DOWN && !cameramode )
	{
		Maryo->Start_Walk( DIR_DOWN );
	}
	else if( key == SDLK_c )
	{
		cameramode = !cameramode;
	}
	else if( key == SDLK_F8 )
	{
		editormode = !editormode;
		cameramode = editormode;
		
		if( editormode )
		{
			debugdisplay->text = "Editor enabled";
		}
		else
		{
			debugdisplay->text = "Editor disabled";
		}

		debugdisplay->counter = DESIRED_FPS * 2;
		pAudio->PlaySound( SOUNDS_DIR "/editor.ogg" );
	}
	else if( key == SDLK_d && ( event.key.keysym.mod & KMOD_LCTRL ) )
	{
		debugmode = !debugmode;
	}
	else if( key == SDLK_l && debugmode )
	{
		Layer->draw = !Layer->draw;
	}
	else if( key == SDLK_ESCAPE || event.key.keysym.sym == SDLK_BACKSPACE )
	{
		entered = 2;
	}
	else if( key == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER )
	{
		if( Maryo->current_waypoint >= 0 && Maryo->direction == -1 )
		{
			pLevel->Load( Waypoints[Maryo->current_waypoint]->Get_Levelfile() );
			
			if( pLevel->is_Loaded() ) // if it's a valid level
			{
				entered = 1;
				DrawEffect( FIXED_COLORBOX_FADEOUT ); // Cool Pixelation
			}
			else // if it's not a valid level just walk to the next
			{
				printf( "Not a valid Overworld level %s\n", Waypoints[Maryo->current_waypoint]->Get_Levelfile().c_str() );
				GotoNextLevel();
			}
		}

		ClearEvents();
	}
}

bool cOverWorld :: Load( string name )
{
	Unload();

	string world_filename = world_handler->Get( name )->Get_WorldFile();

	if( !valid_file( world_filename ) )
	{
		printf( "Couldn't find Overworld file : %s from %s\n", world_filename.c_str(), name.c_str() );
		return 0;
	}

	System::getSingleton().getXMLParser()->parseXMLFile( *this, world_filename, "data/schemes/WorldScheme.xsd", "" );

	Maryo->Load_Images();
	pHudManager->Load();

	return 1;
}

// XML element start
void cOverWorld :: elementStart( const String &element, const XMLAttributes &attributes )
{
    if( element == "Sprite" )
    {
        handle_sprite( attributes );
    }
    else if( element == "Waypoint" )
    {
        handle_waypoint( attributes );
    }
    else if( element == "Layer" )
    {
        handle_layer( attributes );
    }
    else if( element == "Player" )
    {
        handle_player( attributes );
    }
}

// XML element end
void cOverWorld :: elementEnd( const String &element )
{
	
}

void cOverWorld :: Unload( void )
{
	for( unsigned int i = 0; i < Waypoints.size(); i++ )
	{
		delete Waypoints[i];
	}

	Waypoints.clear();

	for( unsigned int i = 0; i < MapObjects.size(); i++ )
	{
		if( MapObjects[i] )
		{
			delete MapObjects[i];
		}
	}

	MapObjects.clear();

	if( Layer )
	{
		delete Layer;
		Layer = NULL;
	}

	current_world = 0;
	Maryo->current_waypoint = -2;

	Map = 0;
}

void cOverWorld :: Set_Progress( unsigned int normal_level, unsigned int secret_level )
{
	for( unsigned int i = 0; i < Waypoints.size(); i++ )
	{
		if( normal_level >= i && Waypoints[i]->type == WAYPOINT_NORMAL )
		{
			Waypoints[i]->access = 1;
		}
		else if( secret_level >= i && Waypoints[i]->type == WAYPOINT_SECRET )
		{
			Waypoints[i]->access = 1;
		}
		else
		{
			Waypoints[i]->access = 0;
		}
	}
}

cWaypoint *cOverWorld :: Get_Waypoint( unsigned int num )
{
	if( num >= Waypoints.size() )
	{
		return NULL;
	}

	return Waypoints[num];
}

int cOverWorld :: Get_Waypoint_Collision( GL_rect *rect_2 )
{
	for( unsigned int i = 0; i < Waypoints.size(); i++ )
	{
		if( Col_Box( rect_2, &Waypoints[i]->rect ) )
		{
			return i;
		}
	}

	return -1;
}

int cOverWorld :: Get_LastValidWaypoint( void )
{
	for( unsigned int i = (unsigned int)Waypoints.size() - 1; i > 0; i-- )
	{
		if( Waypoints[i]->access )
		{
			return i;
		}
	}

	return -1;
}

bool cOverWorld :: GotoNextLevel( void )
{
	if( !Waypoints.size() )
	{
		return 0;
	}

	// Checks if Waypoint exist and if its not the first
	if( Maryo->current_waypoint < (int)Waypoints.size() && Waypoints[Maryo->current_waypoint]->direction_forward != DIR_NOTHING )
	{
		cWaypoint *next_waypoint = Get_Waypoint( Maryo->current_waypoint + 1 );

		// if no next waypoint available return
		if( !next_waypoint )
		{
			return 0;
		}

		if( !next_waypoint->access )
		{
			next_waypoint->access = 1;
		}
		
		Maryo->Start_Walk( Waypoints[Maryo->current_waypoint]->direction_forward );	
		return 1;
	}

	return  0;
}

void cOverWorld :: Reset_Waypoints( void )
{
	for( unsigned int i = 0; i < pOverWorld->Waypoints.size(); i++ )
	{
		pOverWorld->Waypoints[i]->access = pOverWorld->Waypoints[i]->access_default;
	}
}

void cOverWorld :: Load_Map( void )
{
	// todo
}

void cOverWorld :: handle_sprite( const XMLAttributes &attributes )
{
	string filename = attributes.getValueAsString( "Filename" ).c_str();
	int posx = attributes.getValueAsInteger( "PosX" );
	int posy = attributes.getValueAsInteger( "PosY" );

	string final_filename = filename;
	final_filename.insert( 0, "/world/overworlds/" );
	final_filename.insert( 0, PIXMAPS_DIR );

	if( !valid_file( final_filename ) )
	{
		printf( "%s : ", world_handler->worlds[current_world]->dir.c_str() );
		printf( "Line %d, error : %s %s\n", 0, final_filename.c_str(), "file not found" );
		return;
	}

	cSprite *temp = new cSprite( GetImage( final_filename ), posx, GAME_RES_H - posy );
	AddMapObject( temp );
}

void cOverWorld :: handle_waypoint( const XMLAttributes &attributes )
{
	int posx = attributes.getValueAsInteger( "PosX" );
	int posy = attributes.getValueAsInteger( "PosY" );
	int type = attributes.getValueAsInteger( "Type" );
	string levelname = attributes.getValueAsString( "LevelName" ).c_str();
	string dir_back = attributes.getValueAsString( "Dir_Back" ).c_str();
	string dir_for = attributes.getValueAsString( "Dir_For" ).c_str();
	int access = attributes.getValueAsInteger( "Access" );

	if( dir_back.compare( "FIRST" ) != 0 && dir_back.compare( "LAST" ) != 0 && dir_back.compare( "LEFT" ) != 0 && 
		dir_back.compare( "RIGHT" ) != 0 && dir_back.compare( "UP" ) != 0 && dir_back.compare( "DOWN" ) != 0 )
	{
		printf( "%s : ", world_handler->worlds[current_world]->dir.c_str() );
		printf( "Line %d, error : %s %s\n", 0, dir_back.c_str(), "is not a valid direction value" );
		return;
	}

	if( dir_for.compare( "FIRST" ) != 0 && dir_for.compare( "LAST" ) != 0 &&  dir_for.compare( "LEFT" ) != 0 && 
		dir_for.compare( "RIGHT" ) != 0 && dir_for.compare( "UP" ) != 0 && dir_for.compare( "DOWN" ) != 0 )
	{
		printf( "%s : ", world_handler->worlds[current_world]->dir.c_str() );
		printf( "Line %d, error : %s %s\n", 0, dir_for.c_str(), "is not a valid direction value" );
		return;
	}

	cWaypoint *waypoint = new cWaypoint();

	waypoint->rect.x = posx;
	waypoint->rect.y = GAME_RES_H - posy;
	waypoint->type = (Waypoint_type)type;

	string filename = levelname;
	filename.insert( filename.length(), ".txt" );
	// Todo : check if level exists
	waypoint->levelname = filename;

	if( dir_back.compare( "FIRST" ) == 0 )
	{
		waypoint->direction_back = DIR_FIRST;
	}
	else if( dir_back.compare( "LAST" ) == 0 )
	{
		waypoint->direction_back = DIR_LAST;
	}
	else if( dir_back.compare( "LEFT" ) == 0 )
	{
		waypoint->direction_back = DIR_LEFT;
	}
	else if( dir_back.compare( "RIGHT" ) == 0 )
	{
		waypoint->direction_back = DIR_RIGHT; 
	}
	else if( dir_back.compare( "UP" ) == 0 )
	{
		waypoint->direction_back = DIR_UP;
	}
	else if( dir_back.compare( "DOWN" ) == 0 )
	{
		waypoint->direction_back = DIR_DOWN;
	}
	else
	{
		printf( "%s : ", world_handler->worlds[current_world]->dir.c_str() );
		printf( "Line %d, error : %s %s\n", 0, dir_back.c_str(), "Waypoint backward direction unknown" );
	}

	if( dir_for.compare( "FIRST" ) == 0 )
	{
		waypoint->direction_forward = DIR_FIRST;
	}
	else if( dir_for.compare( "LAST" ) == 0 )
	{
		waypoint->direction_forward = DIR_LAST;
	}
	else if( dir_for.compare( "LEFT" ) == 0 )
	{
		waypoint->direction_forward = DIR_LEFT;
	}
	else if( dir_for.compare( "RIGHT" ) == 0 )
	{
		waypoint->direction_forward = DIR_RIGHT; 
	}
	else if( dir_for.compare( "UP" ) == 0 )
	{
		waypoint->direction_forward = DIR_UP;
	}
	else if( dir_for.compare( "DOWN" ) == 0 )
	{
		waypoint->direction_forward = DIR_DOWN;
	}
	else
	{
		printf( "%s : ", world_handler->worlds[current_world]->dir.c_str() );
		printf( "Line %d, error : %s %s\n", 0, dir_for.c_str(), "Waypoint forward direction unknown" );
	}

	if( access == 1 || Waypoints.size() == 0 )
	{
		waypoint->access = 1;
		waypoint->access_default = 1;
	}

	Waypoints.push_back( waypoint );
}

void cOverWorld :: handle_layer( const XMLAttributes &attributes )
{
	string filename = attributes.getValueAsString( "Filename" ).c_str();
	int posx = attributes.getValueAsInteger( "PosX" );
	int posy = attributes.getValueAsInteger( "PosY" );

	string final_filename = "data/world/" + world_handler->worlds[current_world]->dir + "/" + filename;

	if( !valid_file( final_filename ) )
	{
		printf( "%s : ", world_handler->worlds[current_world]->dir.c_str() );
		printf( "Line %d, error : %s %s\n", 0, final_filename.c_str(), "file not found" );
		return;
	}

	if( Layer )
	{
		printf( "%s : ", world_handler->worlds[current_world]->dir.c_str() );
		printf( "Line %d, error : %s %s\n", 0, final_filename.c_str(), "Layer already loaded" );
		return;
	}
	else
	{
		Layer = new cLayer();
		Layer->Load( final_filename );
	}
}

void cOverWorld :: handle_player( const XMLAttributes &attributes )
{
	unsigned int waypoint = attributes.getValueAsInteger( "Waypoint" );
	int state = attributes.getValueAsInteger( "State" );

	if( waypoint > Waypoints.size() )
	{
		printf( "%s : ", world_handler->worlds[current_world]->dir.c_str() );
		printf( "Line %d, error : %d %s\n", 0, waypoint, "is bigger as the available Waypoints" );
		return;
	}
	
	if( state > 5 )
	{
		printf( "%s : ", world_handler->worlds[current_world]->dir.c_str() );
		printf( "Line %d, error : %d %s\n", 0, state, "is not a valid state value" );
		return;
	}

	if( Maryo->current_waypoint < 0 )
	{
		Maryo->Set_Waypoint( waypoint );
	}

	Maryo->maryo_state = (Maryo_type)state;
}

void cOverWorld :: AddMapObject( cSprite *object )
{
	if( !object )
	{
		return;
	}

	MapObjects.push_back( object );
}

void cOverWorld :: Draw( void )
{
	ResetScreen();
	// TODO : defined Background color
	DrawRect( NULL, &darkblue );

	Draw_MapObjects();
	Draw_Waypoints();
	Maryo->Draw();
	Draw_HUD();
	Draw_Debug();
	Layer->Draw();

	if( editormode )
	{
		pMouseCursor->Update();
	}

	SDL_GL_SwapBuffers();
}

void cOverWorld :: Draw_MapObjects( void )
{
	for( unsigned int i = 0; i < MapObjects.size(); i++ )
	{
		if( !MapObjects[i] )
		{
			continue;
		}

		MapObjects[i]->Draw();
	}
}

void cOverWorld :: Draw_Waypoints( void )
{
	for( unsigned int i = 0; i < Waypoints.size(); i++ )
	{
		Waypoints[i]->Draw();
	}
}

void cOverWorld :: Draw_HUD( void )
{
	// Background
	Color color = Get_Color( 230, 170, 0 );
	DrawRect( 0, 0, GAME_RES_W, 30, &color );
	// Line
	color = Get_Color( 200, 150, 0 );
	DrawRect( 0, 30, GAME_RES_W, 5, &color );
	
	hud_level->Draw();
	pHudManager->Update();
}

void cOverWorld :: Draw_Debug( void )
{
	if( !debugmode )
	{
		return;
	}

	double x, y;

	// Debug Mode Text
	x = 15;
	y = GAME_RES_H / 1.06;

	if( debugimage )
	{
		debugimage->Blit( x, y, &black );
	}
	else
	{
		debugimage = RenderText( font_normal, "Debug Mode", white );
	}
	x += debugimage->w + 40;

	// The Current Waypoint Text
	char dtext[100];
	sprintf( dtext, "Curr Waypoint : %d", Maryo->current_waypoint );

	if( debug_curr_waypoint )
	{
		delete debug_curr_waypoint;
	}
	debug_curr_waypoint = RenderText( font_normal, dtext, green );

	debug_curr_waypoint->Blit( x, y, &black );
	x += debug_curr_waypoint->w + 40;

	// The Current Line Text
	sprintf( dtext, "Curr Line : %d", Maryo->current_line );

	if( debug_curr_line )
	{
		delete debug_curr_line;
	}
	debug_curr_line = RenderText( font_normal, dtext, green );

	debug_curr_line->Blit( x, y, &black );
	x += debug_curr_line->w + 40;

	// The Lines Text
	sprintf( dtext, "Lines : %d", Layer->lines.size() );

	if( debug_nlevel )
	{
		delete debug_nlevel;
	}
	debug_nlevel = RenderText( font_normal, dtext, blue );
	
	debug_nlevel->Blit( x, y, &black );
}

void cOverWorld :: Update( void )
{
	pAudio->Update();
	pMouseCursor->Update_Position();

	Update_Camera();
	Maryo->Update();
}

void cOverWorld :: Update_Camera( void )
{
	if( cameramode )
	{
		if( keys[pPreferences->Key_right] || ( pJoystick->right && pPreferences->Joy_enabled ) )
		{
			cameraposx += 10 * Framerate.speedfactor;
		}
		else if( keys[pPreferences->Key_left] || ( pJoystick->left && pPreferences->Joy_enabled ) )
		{
			cameraposx -= 10 * Framerate.speedfactor;
		}
		if( keys[pPreferences->Key_up] || ( ( pJoystick->up || pJoystick->Button( pPreferences->Joy_jump ) ) && pPreferences->Joy_enabled ) )
		{
			cameraposy -= 10 * Framerate.speedfactor;
		}
		else if( keys[pPreferences->Key_down] || ( pJoystick->down && pPreferences->Joy_enabled ) )
		{
			cameraposy += 10 * Framerate.speedfactor;
		}
	}
	else
	{
		if( Maryo->posx - cameraposx - 1> (GAME_RES_W / 4 * 3) ) // Right
		{
			cameraposx = Maryo->posx - ( GAME_RES_W / 4 * 3 );
		}
		else if( cameraposx && Maryo->posx - cameraposx + 1 < (GAME_RES_W / 8 * 2) ) // Left
		{
			cameraposx = Maryo->posx - ( GAME_RES_W / 8 * 2 );
		}
		
		if( Maryo->posy - cameraposy + 1 < ( GAME_RES_H / 6 ) ) // Up
		{
			cameraposy = Maryo->posy - ( GAME_RES_H / 6 );
		}
		else if( cameraposy && Maryo->posy - cameraposy - 1 > ( GAME_RES_H / 6 * 2.9 ) ) // Down
		{
			cameraposy = Maryo->posy - ( GAME_RES_H / 6 * 2.9 );
		}
	}
}