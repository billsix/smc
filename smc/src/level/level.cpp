/***************************************************************************
    level.cpp  -  class for handling level loading and many other things
                             -------------------
    copyright            : (C) 2003-2004 by Artur Hallmann
						   (C) 2003-2006 by FluXy
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

cBackground :: cBackground( void )
{
	type = BG_NONE;

	color_1 = (Uint8)0;
	color_2 = (Uint8)0;

	img_file_1.reserve( 120 );
	img_1 = NULL;

	posx = 0;
	posy = 0;

	speedx = 1;
	speedy = 1;
}

cBackground :: ~cBackground( void )
{
	//
}

void cBackground :: Set_type( BackgroundType ntype )
{
	type = ntype;
}

void cBackground :: Set_color_1( const Color &color )
{ 
	color_1 = color; 
}

void cBackground :: Set_color_2( const Color &color )
{
	color_2 = color; 
}

void cBackground :: Set_image( string nimg_file_1 )
{
	img_file_1 = nimg_file_1;

	if( img_file_1.length() > 3 )
	{
		if( img_file_1.find( PIXMAPS_DIR ) != string::npos )
		{
			img_file_1.erase( 0, strlen( PIXMAPS_DIR ) + 1 );
		}

		img_1 = GetImage( img_file_1 );
	}
}

void cBackground :: Set_image_scroll( double x /* = 1 */, double y /* = 1 */ )
{
	speedx = x;
	speedy = y;
}

void cBackground :: Draw( void )
{
	if( type == BG_GR_VER || type == BG_GR_HOR )  // Draw the gradient
	{
		DrawGradient();
	}
	else if( type == BG_IMG_LEFTRIGHT && img_1 ) 
	{
		if( pPreferences->Backgroundimages_disabled )
		{
			return;
		}

		double xpos = -( ( cameraposx / 5 ) * speedx ); // default speed

		while( xpos < -img_1->w )
		{
			xpos += img_1->w;
		}
		
		while( xpos < GAME_RES_W )
		{
			posx = xpos;
			posy = ( GAME_RES_H - img_1->h ) - ( ( cameraposy / 3 ) * speedy );

			img_1->Blit( posx, posy );

			xpos += img_1->w;
		}
	}
}

void cBackground :: DrawGradient( void )
{
	ResetScreen();

	if( color_1 == color_2 ) // no need to draw a gradient if both colors are the same
	{
		DrawRect( NULL, &color_1 );
	}
	else
	{
		/* Calculates the color for each line, based in the generic
		 *	equation for functions : y = mx + b 
		 */
		double w, h;
		Color color, color_start;
		double pos = 0;


		color_start = color_1;

		if( cameraposy < -1 )
		{
			double power = ( -cameraposy / 10000 );

			if( power > 1 )
			{
				power = 1;
			}

			color_start.red += (Uint8)( (double)( color_2.red - color_1.red ) * power );
			color_start.green += (Uint8)( (double)( color_2.green - color_1.green ) * power );
			color_start.blue += (Uint8)( (double)( color_2.blue - color_1.blue ) * power );
		}

		if( type == BG_GR_VER )
		{
			while( pos < GAME_RES_H )
			{
				posx = 0;
				posy = pos;
				w = GAME_RES_W;
				h = 2;

				color.red = (Uint8)( ( (float)( color_start.red - color_2.red ) / -GAME_RES_H ) * pos + color_start.red );
				color.green = (Uint8)( ( (float)( color_start.green - color_2.green ) / -GAME_RES_H ) * pos + color_start.green );
				color.blue = (Uint8)( ( (float)( color_start.blue - color_2.blue ) / -GAME_RES_H ) * pos + color_start.blue );
		
				DrawRect( posx, posy, w, h, &color );

				pos += h;
			}
		}
		else if( type == BG_GR_HOR )
		{
			while( pos < GAME_RES_W )
			{
				posx = pos;
				posy = 0;
				w = 4;
				h = GAME_RES_H;

				color.red = (Uint8)( ( (float)( color_start.red - color_2.red ) / -GAME_RES_W ) * pos + color_start.red );
				color.green = (Uint8)( ( (float)( color_start.green - color_2.green ) / -GAME_RES_W ) * pos + color_start.green );
				color.blue = (Uint8)( ( (float)( color_start.blue - color_2.blue ) / -GAME_RES_W ) * pos + color_start.blue );
		
				DrawRect( posx, posy, w, h, &color );

				pos += w;
			}
		}
	}
}

string cBackground :: Get_Typename( void )
{
	if( type == BG_NONE )
	{
		return "Disabled";
	}
	else if( type == BG_IMG_LEFTRIGHT )
	{
		return "Left and Right";
	}
	else
	{
		return "Unknown";
	}

	return "";
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel :: cLevel( void ) : cFile_parser()
{
	levelengine_version = -1; // no version

	musicfile = "";
	valid_music = 0;

	levelfile = "";

	musicfile.reserve( 150 );
	levelfile.reserve( 150 );
}

cLevel :: ~cLevel( void )
{
	Unload();
}

int cLevel :: New( string filename )
{
	Unload();
	
	if( filename.compare( "null" ) == 0 )
	{
		unsigned int i = 1;

		filename = pPreferences->level_dir;
		filename.insert( filename.length(), "/new_1.txt" );

		FILE *fp = fopen( filename.c_str(), "r" );
		
		// search for a not existing file
		while( fp )
		{
			i++;
			fclose( fp );

			// replace number
			filename.replace( filename.length() - 5, 1, int_to_string( i ) );
			// try to open the file
			fp = fopen( filename.c_str(), "r" );
			
			// stop on 99
			if( i > 99 ) 
			{
				break;
			}
		}
	}

	ifstream ifs;
	ifs.open( filename.c_str(), ios :: out );

	if( ifs )	// Level already exists
	{
		ifs.close();
		printf( "New Level Error : Level already exists ( %s )\n", filename.c_str() );
		return 0; 
	} 
	else // Level does not exists - Successful
	{
		printf( "Succesfully created a New Level ( %s )\n", filename.c_str() );
		Set_Levelfile( filename, 0 );
		Save();
		return 1; 
	}

	return 0;
}

bool cLevel :: Load( string filename )
{
	if( filename.find( ".txt" ) == string::npos ) 
	{
		filename.insert( filename.length(), ".txt" );
	}

	if( filename.find( pPreferences->level_dir ) == string::npos ) 
	{
		filename.insert( 0, "/" );
		filename.insert( 0, pPreferences->level_dir );
	}
	
	if( !valid_file( filename ) )
	{
		printf( "Couldn't load Level : %s\n", filename.c_str() );
		return 0;
	}
	
	Unload();
	
	levelfile = filename;
	musicfile = MUSIC_DIR "/" DEFAULT_MUSIC;
	valid_music = valid_file( musicfile );

	Parse( filename );
	
	if( levelengine_version < 0 ) // version entry not found
	{
		levelengine_version = 0;
	}

	// hud reset
	pHudManager->Load();

	pAudio->FadeOutMusic( 1000 );
	if( valid_music )
	{
		pAudio->PlayMusic( musicfile, -1, 0, 1000 );
	}
	
	Framerate.Reset();

	return 1;
}

void cLevel :: Save( void ) // V 2.5
{
	pAudio->PlaySound( SOUNDS_DIR "/level_saved.ogg" );

	ofstream ofs( levelfile.c_str(), ios::out | ios::trunc );

	if( !ofs )
	{
		debugdisplay->text = "Couldn't save level " + levelfile ;
		debugdisplay->counter = DESIRED_FPS * 2.5;
		return;
	}

	string tempname; // temporary image path
	tempname.reserve( 120 );

	unsigned int i;
	char row[300];

	// Game Version
	sprintf( row, "### Level Saved with %s V.%s ###\n", CAPTION, VERSION );
	ofs.write( row, strlen( row ) );

	// Get_current_time() has a \n in the string
	sprintf( row, "### Level Saved on the %s\n", Get_current_time().c_str() );
	ofs.write( row, strlen( row ) );

	 // current Levelengine version
	sprintf( row, "Levelengine_version %d\n\n", 1 );
	ofs.write( row, strlen( row ) );
	
	// Music
	tempname = musicfile;
	tempname.erase( 0, strlen( MUSIC_DIR ) + 1 );
	
	if( tempname.compare( DEFAULT_MUSIC ) != 0 ) // only save if it's not the default music
	{
		sprintf( row, "Music %s\n", tempname.c_str() );
		ofs.write( row, strlen( row ) );
	}

	if( backgrounds.size() )
	{
		// Background layer 1 color 1
		sprintf( row, "Background_color_1 %d %d %d\n", backgrounds[0]->color_1.red, backgrounds[0]->color_1.green, backgrounds[0]->color_1.blue );
		ofs.write( row, strlen( row ) );

		// Background layer 1 color 2
		sprintf( row, "Background_color_2 %d %d %d\n\n", backgrounds[0]->color_2.red, backgrounds[0]->color_2.green, backgrounds[0]->color_2.blue );
		ofs.write( row, strlen( row ) );

		// only save the Background layer image informations if needed
		if( backgrounds.size() > 1 && backgrounds[1]->type != BG_NONE && backgrounds[1]->img_file_1.length() > 3 )
		{
			// Background image layer 1
			sprintf( row, "Background_image_type_1 %d\n", (unsigned int)backgrounds[1]->type );
			ofs.write( row, strlen( row ) );

			// Background image layer 1 type
			sprintf( row, "Background_image_1 %s\n\n", backgrounds[1]->img_file_1.c_str() );
			ofs.write( row, strlen( row ) );
		}
	}

	// Maryo's start postition
	sprintf( row, "Player %d %d\n\n", (int)pPlayer->startposx, GAME_RES_H - (int)( pPlayer->startposy + pPlayer->col_rect.h ) );
	ofs.write( row, strlen( row ) );

	// Massive Objects
	sprintf( row, "### Massive Objects ###\n" );
	ofs.write( row, strlen( row ) );

	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( pObjManager->items[i]->array != ARRAY_MASSIVE )
		{
			continue;
		}

		tempname = pObjManager->items[i]->start_image->filename;

		if( tempname.find( PIXMAPS_DIR ) == 0 )
		{
			tempname.erase( 0, strlen( PIXMAPS_DIR ) + 1 );
		}

		sprintf( row, "Sprite %s %d %d MASSIVE\n", tempname.c_str(), (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
		ofs.write( row, strlen( row ) );
	}
	
	// Passive Objects
	sprintf( row, "\n### Passive Objects ###\n" );
	ofs.write( row, strlen( row ) );

	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( pObjManager->items[i]->array != ARRAY_PASSIVE )
		{
			continue;
		}

		tempname = pObjManager->items[i]->start_image->filename;

		if( tempname.find( PIXMAPS_DIR ) == 0 )
		{
			tempname.erase( 0, strlen( PIXMAPS_DIR ) + 1 );
		}

		sprintf( row, "Sprite %s %d %d PASSIVE\n", tempname.c_str(), (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );

		ofs.write( row, strlen( row ) );
	}

	// Enemy Objects
	sprintf( row, "\n### Enemy Objects ###\n" );
	ofs.write( row, strlen( row ) );


	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( pObjManager->items[i]->array != ARRAY_ENEMY )
		{
			continue;
		}
		
		switch( pObjManager->items[i]->type )
		{
		case TYPE_GOOMBA:
		{
			cGoomba *enemy = (cGoomba *)pObjManager->items[i];

			sprintf( row, "Enemy %s %d %d %d %d\n", "GOOMBA", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy, enemy->color, enemy->start_direction );
			break;
		}
		case TYPE_TURTLE:
		{
			sprintf( row, "Enemy %s %d %d\n", "TURTLE", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
			break;
		}
		case TYPE_JPIRANHA:
		{
			cjPiranha *enemy = (cjPiranha *)pObjManager->items[i];

			sprintf( row, "Enemy %s %d %d %d %d\n", "JPIRANHA", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy, (int)pObjManager->items[i]->direction, enemy->max_distance );
			break;
		}
		case TYPE_BANZAI_BILL:
		{
			sprintf( row, "Enemy %s %d %d %d\n", "BANZAI_BILL", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy, pObjManager->items[i]->direction );
			break;
		}
		case TYPE_REX:
		{
			sprintf( row, "Enemy %s %d %d\n", "REX", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
			break;
		}
		default:
		{
			continue;
			break;
		}
		}

		ofs.write( row, strlen( row ) );
	}
	
	// Active Objects
	sprintf( row, "\n### Active Objects ###\n" );
	ofs.write( row, strlen( row ) );

	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( pObjManager->items[i]->array != ARRAY_ACTIVE )
		{
			continue;
		}

		switch( pObjManager->items[i]->type )
		{
		case TYPE_GOLDPIECE:
		{
			if( pObjManager->items[i]->spawned )
			{
				continue;
			}

			cGoldPiece *active = (cGoldPiece*)pObjManager->items[i];

			sprintf( row, "Goldpiece %d %d %d\n", (int)active->startposx, GAME_RES_H - (int)active->startposy, active->color );
			break;
		}
		case TYPE_MOON:
		{
			if( pObjManager->items[i]->spawned )
			{
				continue;
			}

			sprintf( row, "Moon %d %d\n", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
			break;
		}
		case TYPE_HALFMASSIVE:
		{
			tempname = pObjManager->items[i]->start_image->filename;
			
			if( tempname.find( PIXMAPS_DIR ) == 0 )
			{
				tempname.erase( 0, strlen( PIXMAPS_DIR ) + 1 );
			}

			string type = "Sprite";

			if( pObjManager->items[i]->massivetype == MASS_CLIMBABLE )
			{
				type.insert( 0, "Climb" );
			}

			sprintf( row, "%s %s %d %d HALFMASSIVE\n", type.c_str(), tempname.c_str(), (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
			break;
		}
		case TYPE_CLOUD:
		{
			sprintf( row, "Cloud %d %d\n", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
			break;
		}
		case TYPE_LEVELEXIT:
		{
			cLevelExit *active = (cLevelExit*)pObjManager->items[i];

			sprintf( row, "Levelexit %d %d %d %s\n", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy, active->levelchange, active->levelname.c_str() );
			break;
		}
		case TYPE_ENEMYSTOPPER:
		{
			sprintf( row, "EnemyStopper %d %d\n", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
			break;
		}
		case TYPE_BONUSBOX_MUSHROOM_FIRE:
		{
			sprintf( row, "BonusBox %d %d AUTO\n", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
			break;
		}
		case TYPE_BONUSBOX_LIVE:
		{
			sprintf( row, "BonusBox %d %d LIFE\n", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
			break;
		}
		case TYPE_BONUSBOX_JSTAR:
		{
			sprintf( row, "BonusBox %d %d STAR\n", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
			break;
		}
		case TYPE_SPINBOX:
		{
			sprintf( row, "SpinBox %d %d\n", (int)pObjManager->items[i]->startposx, GAME_RES_H - (int)pObjManager->items[i]->startposy );
			break;
		}
		case TYPE_GOLDBOX:
		{
			cGoldBox *active = (cGoldBox*)pObjManager->items[i];

			sprintf( row, "GoldBox %d %d %d %d\n", (int)active->startposx, GAME_RES_H - (int)active->startposy, active->color, active->start_useable_count );
			break;
		}
		default:
		{
			continue;
			break;
		}
		}
		
		ofs.write( row, strlen( row ) );
	}

	ofs.close();

	debugdisplay->text = "Level " + Get_filename( 0, 0 ) + " saved";
	debugdisplay->counter = DESIRED_FPS * 2.5;
}

void cLevel :: Show_Error( string errortext )
{
	GL_Surface *image = RenderText( font_normal, errortext, black );
	cSprite* temp = new cSprite( image, GAME_RES_W / 2 - image->w / 2, GAME_RES_H / 2 - image->h / 2 );

	temp->Draw();
	SDL_GL_SwapBuffers();
	SDL_Delay( 4000 ); // todo with pollevent for ESC and a counter
	delete temp;
	delete image;

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

void cLevel :: Draw( LevelDrawType type /* = LVL_UPDATE */ )
{
	if( Leveleditor_Mode )
	{
		type = LVL_DRAW_EDITOR;
		pMouseCursor->Update_Position();
	}

	if( type != LVL_DRAW_NO_BG ) // with background
	{
		for( unsigned int i = 0; i < backgrounds.size(); i++ )
		{
			backgrounds[i]->Draw();
		}
	}

	if( type == LVL_DRAW_BG ) // only background
	{
		return;
	}

	unsigned int i;

	// Active Objects
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( pObjManager->items[i]->array != ARRAY_ACTIVE )
		{
			continue;
		}

		if( type == LVL_UPDATE )
		{
			if( pObjManager->items[i]->type != TYPE_MUSHROOM_DEFAULT && pObjManager->items[i]->type != TYPE_MUSHROOM_LIVE_1 && pObjManager->items[i]->type != TYPE_FIREPLANT && pObjManager->items[i]->type != TYPE_JSTAR &&
				pObjManager->items[i]->type != TYPE_HALFMASSIVE && pObjManager->items[i]->type != TYPE_GOLDBOX &&
				pObjManager->items[i]->type != TYPE_SPINBOX && pObjManager->items[i]->type != TYPE_BONUSBOX_LIVE && pObjManager->items[i]->type != TYPE_BONUSBOX_MUSHROOM_FIRE && pObjManager->items[i]->type != TYPE_BONUSBOX_JSTAR )
			{
				pObjManager->items[i]->Update();
			}
		}
		else // No Update
		{
			if( pObjManager->items[i]->visible || type == LVL_DRAW_EDITOR )
			{
				if( pObjManager->items[i]->type != TYPE_HALFMASSIVE && pObjManager->items[i]->type != TYPE_GOLDPIECE && pObjManager->items[i]->type != TYPE_MOON && 
					pObjManager->items[i]->type != TYPE_SPINBOX && pObjManager->items[i]->type != TYPE_GOLDBOX && 
					pObjManager->items[i]->type != TYPE_BONUSBOX_MUSHROOM_FIRE && pObjManager->items[i]->type != TYPE_BONUSBOX_LIVE && pObjManager->items[i]->type != TYPE_BONUSBOX_JSTAR &&
					pObjManager->items[i]->type != TYPE_MUSHROOM_DEFAULT && pObjManager->items[i]->type != TYPE_MUSHROOM_LIVE_1 && pObjManager->items[i]->type != TYPE_FIREPLANT && pObjManager->items[i]->type != TYPE_JSTAR && 
					pObjManager->items[i]->type != TYPE_ENEMYSTOPPER && pObjManager->items[i]->type != TYPE_LEVELEXIT )
				{
					pObjManager->items[i]->Draw();
				}
			}
		}
	}

	// Passive Objects 
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( pObjManager->items[i]->array != ARRAY_PASSIVE )
		{
			continue;
		}

		pObjManager->items[i]->Draw();
	}
	
	// Enemy Objects
	// only Banzai Bill
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
        if( pObjManager->items[i]->array != ARRAY_ENEMY )
		{
			continue;
		}

		if( type == LVL_UPDATE )
		{
			if( pObjManager->items[i]->type == TYPE_BANZAI_BILL )
			{
				pObjManager->items[i]->Update();
			}
		}
		else
		{
			if( type != LVL_DRAW_EDITOR && pObjManager->items[i]->type == TYPE_BANZAI_BILL )
			{
				pObjManager->items[i]->Draw();
			}
		}
	}
	
	// Active Objects
	// Only Halfmassive
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( pObjManager->items[i]->array != ARRAY_ACTIVE )
		{
			continue;
		}
		
		if( type == LVL_UPDATE )
		{
			if( pObjManager->items[i]->type == TYPE_HALFMASSIVE )
			{
				pObjManager->items[i]->Update();
			}
		}
		else // No Update
		{
			if( pObjManager->items[i]->visible || type == LVL_DRAW_EDITOR )
			{
				if( pObjManager->items[i]->type == TYPE_HALFMASSIVE )
				{
					pObjManager->items[i]->Draw();
				}
			}
		}
	}

	// Active Objects
	// All the other Objects
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( pObjManager->items[i]->array != ARRAY_ACTIVE )
		{
			continue;
		}
		
		if( type == LVL_UPDATE )
		{
			if( pObjManager->items[i]->type == TYPE_MUSHROOM_DEFAULT || pObjManager->items[i]->type == TYPE_MUSHROOM_LIVE_1 || pObjManager->items[i]->type == TYPE_JSTAR || pObjManager->items[i]->type == TYPE_FIREPLANT ||
				pObjManager->items[i]->type == TYPE_GOLDBOX || pObjManager->items[i]->type == TYPE_SPINBOX || 
				pObjManager->items[i]->type == TYPE_BONUSBOX_LIVE || pObjManager->items[i]->type == TYPE_BONUSBOX_MUSHROOM_FIRE || pObjManager->items[i]->type == TYPE_BONUSBOX_JSTAR )
			{
				pObjManager->items[i]->Update();
			}
		}
		else // No Update
		{
			if( pObjManager->items[i]->visible || type == LVL_DRAW_EDITOR )
			{
				if( pObjManager->items[i]->type == TYPE_GOLDPIECE || pObjManager->items[i]->type == TYPE_MOON ||
					pObjManager->items[i]->type == TYPE_SPINBOX || pObjManager->items[i]->type == TYPE_GOLDBOX ||
					pObjManager->items[i]->type == TYPE_BONUSBOX_MUSHROOM_FIRE || pObjManager->items[i]->type == TYPE_BONUSBOX_LIVE || pObjManager->items[i]->type == TYPE_BONUSBOX_JSTAR || 
					( type != LVL_DRAW_EDITOR && ( pObjManager->items[i]->type == TYPE_MUSHROOM_DEFAULT || pObjManager->items[i]->type == TYPE_BONUSBOX_MUSHROOM_FIRE || pObjManager->items[i]->type == TYPE_MUSHROOM_LIVE_1 || pObjManager->items[i]->type == TYPE_JSTAR ) ) )
				{
					pObjManager->items[i]->Draw();
				}
			}
		}
	}

	// Enemy Objects
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
        if( pObjManager->items[i]->array != ARRAY_ENEMY )
		{
			continue;
		}

		if( type == LVL_UPDATE )
		{
			if( pObjManager->items[i]->type == TYPE_JPIRANHA )
			{
				pObjManager->items[i]->Update();
			}
		}
		else
		{
			if( ( pObjManager->items[i]->type != TYPE_JPIRANHA && pObjManager->items[i]->type != TYPE_BANZAI_BILL ) || 
				( type != LVL_DRAW_EDITOR && pObjManager->items[i]->type == TYPE_JPIRANHA ) )
			{
				pObjManager->items[i]->Draw();
			}
		}
	}

	// Animations
	if( type == LVL_UPDATE )
	{
		pAnimationManager->Update();
	}
	pAnimationManager->Draw();

	// Massive Objects
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( pObjManager->items[i]->array != ARRAY_MASSIVE )
		{
			continue;
		}

		pObjManager->items[i]->Draw();
	}

	// Enemy Objects
	/* default : enemies without jpiranha and banzai bill
	 * leveleditor : only jpiranha and banzai bill
	*/
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
        if( pObjManager->items[i]->array != ARRAY_ENEMY )
		{
			continue;
		}

		if( type == LVL_UPDATE )
		{
			if( pObjManager->items[i]->type != TYPE_JPIRANHA && pObjManager->items[i]->type != TYPE_BANZAI_BILL )
			{
				pObjManager->items[i]->Update();
			}
		}
		else // No Update
		{
			if( type == LVL_DRAW_EDITOR && ( pObjManager->items[i]->type == TYPE_JPIRANHA || pObjManager->items[i]->type == TYPE_BANZAI_BILL ) )
			{
				pObjManager->items[i]->Draw();
			}
		}
	}

	// Active Objects
	// leveleditor : only enemystopper and levelexit
	for( i = 0; i < pObjManager->Get_Size(); i++ )
	{
		if( pObjManager->items[i]->array != ARRAY_ACTIVE )
		{
			continue;
		}

		if( type == LVL_UPDATE )
		{ 
			// Currently nothing
			/*if(  )
			{
				pObjManager->items[i]->Update();
			}*/
		}
		else // No Update
		{
			if( pObjManager->items[i]->visible || type == LVL_DRAW_EDITOR )
			{
				if( pObjManager->items[i]->type == TYPE_ENEMYSTOPPER || pObjManager->items[i]->type == TYPE_LEVELEXIT )
				{
					pObjManager->items[i]->Draw();
				}
			}
		}

	}

	pHudManager->Update();

	// Leveleditor mouse update
	if( Leveleditor_Mode )
	{
		pMouseCursor->Editor_Update();
	}
}

void cLevel :: Unload( void )
{
	pObjManager->Delete_All( 1 );
	pDialogManager->DeleteAll();
	pAnimationManager->DeleteAll();

	pHudManager->Update();
	pJoystick->Reset_keys();

	for( unsigned int i = 0; i < backgrounds.size(); i++ )
	{
		if( backgrounds[i] )
		{
			delete backgrounds[i];
		}
	}

	backgrounds.clear();

	// default gradient layer
	backgrounds.push_back( new cBackground() );
	backgrounds[0]->Set_type( BG_GR_VER );

	if( timedisplay )
	{
		timedisplay->counter = -1;
	}

	if( pMouseCursor )
	{
		pMouseCursor->Reset();
	}

	levelengine_version = -1; // no version

	musicfile.clear();
	levelfile.clear();
}

string cLevel :: Get_Musicfile( bool with_dir /* = 1 */, bool with_end /* = 1 */ )
{
	string name = musicfile;

	if( !with_dir && name.find( "/" ) != string::npos ) 
	{
		name.erase( 0, name.rfind( "/" ) + 1 );
	}

	if( !with_end && name.rfind( "." ) != string::npos ) 
	{
		name.erase( name.rfind( "." ), name.rfind( "." ) - name.length() );
	}

	return name;
}

void cLevel :: Set_Musicfile( string filename )
{
	if( filename.length() > 4 ) 
	{
		if( filename.find( MUSIC_DIR ) == string::npos ) 
		{
			filename.insert( 0, "/" );
			filename.insert( 0, MUSIC_DIR );
		}

		musicfile = filename;
	}
}

void cLevel :: Set_Levelfile( string filename, bool delete_old /* = 1 */ )
{
	if( filename.length() < 2 )
	{
		return;
	}

	if( delete_old )
	{
		Delete_file( levelfile );
	}

	if( filename.find( ".txt" ) == string::npos ) 
	{
		filename.insert( filename.length(), ".txt" );
	}

	levelfile = filename;
	data_file = filename;

	if( levelfile.find( pPreferences->level_dir ) == string::npos ) 
	{
		levelfile.insert( 0, "/" );
		levelfile.insert( 0, pPreferences->level_dir );
	}

	Save();
}

bool cLevel :: is_Loaded( void )
{
	if( pObjManager->Get_Size() > 0 || levelfile.length() > 2 )
	{
		return 1;
	}

	return 0;
}

int cLevel :: Load_Savegame( unsigned int save_slot )
{
	cSavegame_data savegame = pSavegame->Load( save_slot );
	string level_name = pPreferences->level_dir + "/" + savegame.levelname;

	if( savegame.lives < 0 )
	{
		printf( "Error loading Savegame %d\n", save_slot );
		return 0;
	}
	
	Unload();
	pObjManager->Update(); // clear deleted objects

	if( !pOverWorld->Load( "default" ) )
	{
		printf( "Error loading Savegame %d : Couldn't load Overworld\n", save_slot );
		return 0;
	}

	if( !savegame.Overworld_save )
	{
		Load( level_name );
	}
	
	if( savegame.version >= 4 )
	{
		pOverWorld->Set_Progress( savegame.Overworld_nlevel, savegame.Overworld_slevel );
		pOverWorld->Maryo->Set_Waypoint( savegame.Overworld_curr_waypoint );
	}
	else // savegames below version 4 are unsupported
	{
		printf( "Error loading Savegame %d : versions below 4 are unsupported\n", save_slot );
		return 0;
	}
	
	pPlayer->Reset();
	pPlayer->maryo_type = (Maryo_type)savegame.state;
	pPlayer->LoadImages();
	pPlayer->SetPos( (double)savegame.posx, (double)( savegame.posy - GAME_RES_H ) );
	pointsdisplay->SetPoints( savegame.points );
	golddisplay->SetGold( savegame.goldpieces );
	livedisplay->SetLives( savegame.lives );

	if( savegame.version >= 5 ) // version 5 and higher with Itembox support
	{
		Itembox->Set_Item( savegame.itembox_item );	
	}

	cameraposx = 0;
	cameraposy = 0;

	if( !savegame.Overworld_save )
	{
		pPlayer->invincible = DESIRED_FPS;
	}

	pPlayer->Fireball_clear();

	debugdisplay->Update ();
	pointsdisplay->Update();
	pPlayer->Update();
	golddisplay->Update();

	debugdisplay->text = "Savegame " + int_to_string( save_slot ) + " loaded";
	debugdisplay->counter = DESIRED_FPS * 1.5;

	if( savegame.Overworld_save ) // Overworld Savegame
	{
		return 2;
	}
	
	return 1; // Level Savegame
}

bool cLevel :: Save_Savegame( unsigned int save_slot, string description )
{
	if( pPlayer->maryo_type == MARYO_DEAD || pPlayer->lives < 0 )
	{
		printf( "Couldn't save Savegame %s\n", description.c_str() );
		return 0;
	}

	cSavegame_data savegame;

	// Description
	savegame.description = description;
	// Goldpieces
	savegame.goldpieces = pPlayer->goldpieces;
	// Level

	if( is_Loaded() && pLevel->levelfile.length() > 3 ) // Player is in a Level
	{
		string Level_name;
		
		Level_name = pLevel->levelfile;
		Level_name.erase( 0, pPreferences->level_dir.length() + 1 );
		
		savegame.levelname = Level_name;
		savegame.Overworld_save = 0;
	}
	else if( pOverWorld->Maryo->current_waypoint >= 0 ) // valid waypoint
	{
		savegame.levelname = pOverWorld->Waypoints[pOverWorld->Maryo->current_waypoint]->levelname;
		savegame.Overworld_save = 1;
	}
	
	// Lifes
	savegame.lives = pPlayer->lives;
	// Points
	savegame.points = pPlayer->points;
	// Position
	savegame.posx = (int)pPlayer->posx;
	savegame.posy = (int)pPlayer->posy + GAME_RES_H - 5;
	// State
	savegame.state = pPlayer->maryo_type;
	// Itembox Item
	savegame.itembox_item = Itembox->item_id;

	// OverWorld normal Levels accessed
	savegame.Overworld_nlevel = pOverWorld->Get_LastValidWaypoint();
	// OverWorld secret Levels accessed
	savegame.Overworld_slevel = 0;
	// which OverWorld
	savegame.Overworld_world = pOverWorld->current_world;
	// Overworld current Waypoint
	savegame.Overworld_curr_waypoint = pOverWorld->Maryo->current_waypoint;
	// Time
	savegame.timestamp = Get_current_time();
	// Version
	savegame.version = 5;

	// Save it
	pSavegame->Save( save_slot, savegame );

	// Print
	if( debugdisplay )
	{
		debugdisplay->text = "Saved to Slot " + int_to_string( save_slot );
		debugdisplay->counter = DESIRED_FPS * 1.5;
	}

	return 1;
}

bool cLevel :: HandleMessage( string *parts, unsigned int count, unsigned int line )
{
	if( parts[0].compare( "Sprite" ) == 0 || parts[0].compare( "ClimbSprite" ) == 0 )
	{
		if( count != 5 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
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
			filename.insert( 0, "/" );
			filename.insert( 0, PIXMAPS_DIR );	
		}

		string settings_file = filename;
		settings_file.erase( settings_file.rfind( "." ) + 1, 3 );
		settings_file.insert( settings_file.rfind( "." ) + 1, "settings" );

		if( !valid_file( filename ) && !valid_file( settings_file ) )
		{
			// try it with the old_pixmaps
			filename.insert( strlen( PIXMAPS_DIR ), "/old_pixmaps" );
			
			if( !valid_file( filename ) ) // nowhere found = error
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "file not found : %s/%s\n", PIXMAPS_DIR, parts[1].c_str() );
				return 0; // error
			}
		}
		
		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[3] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[3].c_str() );
			return 0; // error
		}
		if( !parts[4].compare( "MASSIVE" ) == 0 && !parts[4].compare( "PASSIVE" ) == 0 && !parts[4].compare( "HALFMASSIVE" ) == 0 )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "parameter five has to be MASSIVE, PASSIVE, or HALFMASSIVE\n" );
			return 0; // error
		}

		string getimage_path = filename;
		getimage_path.erase( 0, strlen( PIXMAPS_DIR ) + 1 );

		if( parts[4].compare( "MASSIVE" ) == 0 )
		{
			GL_Surface *sTemp = GetImage( getimage_path );
			cSprite *temp = new cSprite( sTemp, string_to_int( parts[2] ), GAME_RES_H - string_to_int( parts[3] ) );
			temp->massivetype = MASS_MASSIVE;
			temp->array = ARRAY_MASSIVE;
			pObjManager->Add( temp );
		}
		else if( parts[4].compare( "PASSIVE" ) == 0 )
		{
			GL_Surface *sTemp = GetImage( getimage_path );
			cSprite *temp = new cSprite( sTemp, string_to_int( parts[2] ), GAME_RES_H - string_to_int( parts[3] ) );
			temp->massivetype = MASS_PASSIVE;
			temp->array = ARRAY_PASSIVE;
			pObjManager->Add( temp );
		}
		else if( parts[4].compare( "HALFMASSIVE" ) == 0 )
		{
			GL_Surface *sTemp = GetImage( getimage_path );
			cSprite *temp = new cSprite( sTemp, string_to_int( parts[2] ), GAME_RES_H - string_to_int( parts[3] ) );
			temp->array = ARRAY_ACTIVE;
			temp->type = TYPE_HALFMASSIVE;
			
			if( parts[0].compare( "ClimbSprite" ) == 0 ) // Climbable
			{
				temp->massivetype = MASS_CLIMBABLE;
			}
			else
			{
				temp->massivetype = MASS_HALFMASSIVE;
			}

			pObjManager->Add( temp );
		}
	} 
	/***************************************************************************/
	else if( parts[0].compare( "Enemy" ) == 0 )
	{
		if( count < 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs more than 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[3] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[3].c_str() );
			return 0; // error
		}
		
		if( parts[1].compare( "GOOMBA" ) == 0 ) // Goomba
		{
			/* 3 and 4 is a very old description
			 * 5 is lower than V.0.98
			*/
			if( !( count == 3 || count == 4 || count == 5 || count == 6 ) )
			{
				printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 5 parameters" );
				return 0; // error
			}

			if( count >= 5 ) // color support
			{
				cGoomba *temp = NULL;
				unsigned int color = string_to_int( parts[4] );

				// correct old color types
				if( count == 5 )
				{
					if( color == 1 ) // Red
					{
						color = COL_RED;
					}
					else if( color == 0 ) // Brown
					{
						color = COL_BROWN;
					}
				}
				
				temp = new cGoomba( string_to_int( parts[2] ), GAME_RES_H - string_to_int( parts[3] ), color );

				if( count >= 6 ) // direction support
				{
					if( !is_valid_number( parts[5] ) )
					{
						printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
						printf( "%s is not a valid integer value\n", parts[5].c_str() );
						return 0; // error
					}

					ObjectDirection direction = DIR_RIGHT;
					direction = (ObjectDirection)string_to_int( parts[5] );

					if( direction == DIR_LEFT || direction == DIR_RIGHT )
					{
						temp->Set_Direction( direction, 1 );
					}
				}

				if( temp )
				{
					pObjManager->Add( (cSprite *)temp );
				}
			}
			else // support for old levels ( lower than V.0.8 )
			{
				cGoomba *temp = new cGoomba( string_to_int( parts[2] ), GAME_RES_H - string_to_int( parts[3] ) );
				pObjManager->Add( (cSprite *)temp );
			}

		}
		else if( parts[1].compare( "RTURTLE" ) == 0 || parts[1].compare( "TURTLE" ) == 0 ) // "RTURTLE" for older levels
		{
			if( count != 4 )
			{
				printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 4 parameters" );
				return 0; // error
			}

			// 1 for red
			cTurtle *temp = new cTurtle( string_to_int( parts[2] ), GAME_RES_H - string_to_int( parts[3] ), 1 );
			pObjManager->Add( (cSprite *)temp );
		}
		else if( parts[1].compare( "JPIRANHA" ) == 0 )
		{
			if( count != 6 && count != 4 )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "Error : %s %s\n", parts[1].c_str(), "needs 6 parameters" );
				return 0; // error
			}

			ObjectDirection direction = DIR_UP;
			unsigned int max_height = 200;

			if( count == 6 )
			{
				if( !is_valid_number( parts[4] ) )
				{
					printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
					printf( "%s is not a valid integer value\n", parts[4].c_str() );
					return 0; // error
				}
				if( !is_valid_number( parts[5] ) )
				{
					printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
					printf( "%s is not a valid integer value\n", parts[5].c_str() );
					return 0; // error
				}

				direction = (ObjectDirection)string_to_int( parts[4] );
				max_height = string_to_int( parts[5] );
			}

			cjPiranha *temp = new cjPiranha( string_to_int( parts[2] ), GAME_RES_H - string_to_int( parts[3] ), direction, max_height );
			pObjManager->Add( (cSprite *)temp );
		}
		else if( parts[1].compare( "BANZAI_BILL" ) == 0 )
		{
			if( count != 5 )
			{
				printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
				printf( "Line %d, error : %s %s\n", line, parts[1].c_str(), "needs 5 parameters" );
				return 0; // error
			}

			if( string_to_int( parts[4] ) != 1 && string_to_int( parts[4] ) != 0 )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "Enemy BANZAI_BILL parameter 4 is not 1 or 0\n" );
				return 0; // error
			}

			cbanzai_bill *temp = new cbanzai_bill( string_to_int( parts[2] ), GAME_RES_H - string_to_int( parts[3] ), (ObjectDirection)string_to_int( parts[4] ) );
			pObjManager->Add( (cSprite*)temp );
		}
		else if( parts[1].compare( "REX" ) == 0 )
		{
			if( count != 4 )
			{
				printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
				printf( "Line %d, error : %s %s\n", line, parts[1].c_str(), "needs 4 parameters" );
				return 0; // error
			}

			cRex *temp = new cRex( string_to_int( parts[2] ), GAME_RES_H - string_to_int( parts[3] ) );
			pObjManager->Add( (cSprite*)temp );
		}
		else
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Line %d, error : %s %s\n", line, parts[1].c_str(), "Unknown Enemy type" );
			return 0; // error
		}
	}
	/***************************************************************************/
	else if( parts[0].compare( "Goldpiece" ) == 0 )
	{
		if( count < 3 || count > 4 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-4 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		
		DefaultColor color = COL_YELLOW;

		if( count > 3 ) // with color information
		{
			if( !is_valid_number( parts[3] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[3].c_str() );
				return 0; // error
			}

			color = (DefaultColor)string_to_int( parts[3] );
		}

		cGoldPiece *temp = new cGoldPiece( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ), color );
		pObjManager->Add( (cSprite*)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Moon" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		
		cMoon *temp = new cMoon( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ) );
		pObjManager->Add( (cSprite*)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Levelexit" ) == 0 )
	{
		if( count < 3 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-5 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}

		cLevelExit *temp = NULL;

		if( count == 5 || count == 4 ) // new
		{
			if( !is_valid_number( parts[3] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[2].c_str() );
				return 0; // error
			}
			
			string level;

			if( count == 5 ) // with a given level
			{
				level = parts[4];

				if( level.find( pPreferences->level_dir ) == string::npos ) 
				{
					level.insert( 0, "/" );
					level.insert( 0, pPreferences->level_dir );
				}

				if( !valid_file( level ) ) // if not found
				{
					printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
					printf( "Sublevel not found : %s\n", level.c_str() );
					level.clear();
				}
			}

			temp = new cLevelExit( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ), (Levelchange_type)string_to_int( parts[3] ), level );
		}
		else // older level support
		{
			// add a door sprite
			cSprite *door_sprite = new cSprite( GetImage( "game/level/door_yellow_1.png" ), string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ) );
			door_sprite->massivetype = MASS_PASSIVE;
			door_sprite->array = ARRAY_PASSIVE;
			door_sprite->type = TYPE_SPRITE;
			pObjManager->Add( (cSprite *)door_sprite );
			// add the levelexit
			temp = new cLevelExit( string_to_int( parts[1] ) + 13, GAME_RES_H - string_to_int( parts[2] ) + 60 );
		}

		pObjManager->Add( (cSprite*)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "BonusBox" ) == 0 )
	{
		if( !( count == 3 || count == 4 ) )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-4 parameters" );
			return 0; // error
		}

		if( count == 3 ) // old Level Support
		{
			if( !is_valid_number( parts[1] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[1].c_str() );
				return 0; // error
			}
			if( !is_valid_number( parts[2] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[2].c_str() );
				return 0; // error
			}
			
			cBonusBox *temp = new cBonusBox( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ) );
			pObjManager->Add( (cSprite *)temp );
		}
		else if( count == 4 )
		{
			if( !is_valid_number( parts[1] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[1].c_str() );
				return 0; // error
			}
			if ( !is_valid_number( parts[2] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[2].c_str() );
				return 0; // error
			}

			if( parts[3].compare( "AUTO" ) == 0 ) // Auto Mushroom or Fireplant
			{
				cBonusBox *temp = new cBonusBox( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ),TYPE_BONUSBOX_MUSHROOM_FIRE);
				pObjManager->Add( (cSprite *)temp );
			}
			else if( parts[3].compare( "LIFE" ) == 0 ) // Life Mushroom
			{
				cBonusBox *temp = new cBonusBox( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ),TYPE_BONUSBOX_LIVE );
				pObjManager->Add( (cSprite *)temp );
			}
			else if( parts[3].compare( "STAR" ) == 0 ) // Jumping Star
			{
				cBonusBox *temp = new cBonusBox( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ),TYPE_BONUSBOX_JSTAR );
				pObjManager->Add( (cSprite *)temp );
			}
			else // unknown Bonus item
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "Bonusbox type Error %s\n", parts[3].c_str() );
				return 0; // error
			}
		}
		else
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "Bonusbox count Error %d\n", count );
			return 0; // error
		}
	}
	/***************************************************************************/
	else if( parts[0].compare( "GoldBox" ) == 0 )
	{
		if( count < 3 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3-5 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		
		DefaultColor color = COL_YELLOW;
		int usable = 1;

		if( count > 3 ) // with color information
		{
			if( !is_valid_number( parts[3] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[3].c_str() );
				return 0; // error
			}

			color = (DefaultColor)string_to_int( parts[3] );
		}

		cGoldBox *temp = new cGoldBox( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ), color );

		if( count > 4 ) // with useable times information
		{
			if( !is_valid_number( parts[4] ) )
			{
				printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
				printf( "%s is not a valid integer value\n", parts[4].c_str() );
				return 0; // error
			}

			temp->Set_UseableCount( string_to_int( parts[4] ), 1 );
		}

		
		pObjManager->Add( (cSprite *)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "SpinBox" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		
		cSpinBox *temp = new cSpinBox( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ) );
		pObjManager->Add( (cSprite *)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Cloud" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		
		cCloud *temp = new cCloud( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ) );
		pObjManager->Add( (cSprite*)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Player" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}
		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		
		pPlayer->SetPos( (double)string_to_int( parts[1] ), (double)GAME_RES_H - (double)( string_to_int( parts[2] ) + pPlayer->image->h ), 1 );
	}
	/***************************************************************************/
	else if( parts[0].compare( "EnemyStopper" ) == 0 )
	{
		if( count != 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 3 parameters" );
			return 0; // error
		}
		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}
		if( !is_valid_number( parts[2] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[2].c_str() );
			return 0; // error
		}
		
		cEnemyStopper *temp = new cEnemyStopper( string_to_int( parts[1] ), GAME_RES_H - string_to_int( parts[2] ) );
		pObjManager->Add( (cSprite*)temp );
	}
	/***************************************************************************/
	else if( parts[0].compare( "Music" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( parts[1].length() < 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "No valid Music file\n" );
			return 0;  // error
		}
		
		string filename;

		filename.reserve( parts[1].length() + 50 );
		filename = parts[1];

		if( filename.find( MUSIC_DIR ) == string::npos )
		{
			filename.insert( 0, "/" );
			filename.insert( 0, MUSIC_DIR );
		}

		Set_Musicfile( filename );
		valid_music = valid_file( filename );

		if( !valid_music )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Music file not found : %s\n", filename.c_str() );

			return 0; // error
		}
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
		if( count < 2 || count > 5 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s %s\n", parts[0].c_str(), "needs 2-5 parameters" );
			return 0; // error
		}

		if( parts[1].length() < 3 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Not a valid %s file\n", parts[0].c_str() );
			return 0;  // error
		}
		
		string filename;

		filename.reserve( parts[1].length() + 50 );
		filename = parts[1];

		if( filename.find( PIXMAPS_DIR ) == string::npos )
		{
			filename.insert( 0, "/" );
			filename.insert( 0, PIXMAPS_DIR );
		}

		if( !valid_file( filename ) )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s file not found : %s\n", parts[0].c_str(), filename.c_str() );

			return 0; // error
		}

		cBackground *background = new cBackground();
		background->Set_image( parts[1] );
	
		if( count > 2 ) // type is given
		{
			unsigned int bg_type = string_to_int( parts[2] );

			if( bg_type < 0 || bg_type > 200 )
			{
				printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
				printf( "Not a valid Backgroundimage type ( %d )\n", bg_type );
				return 0;  // error
			}
			
			background->Set_type( (BackgroundType)bg_type );
		}
		else // default type
		{
			background->Set_type( BG_IMG_LEFTRIGHT );
		}

		if( count > 3 ) // x speed is given
		{
			double speedx = string_to_double( parts[3] );
			double speedy = 1;

			if( count > 4 ) // y speed is given
			{
				speedy = string_to_double( parts[3] );
			}

			background->Set_image_scroll( speedx, speedy );
		}

		backgrounds.push_back( background );
	}
	/***************************************************************************/
	else if( parts[0].find( "Background_image_type" ) == 0 )
	{
		// ignore
	}
	/***************************************************************************/
	else if( parts[0].compare( "Levelengine_version" ) == 0 )
	{
		if( count != 2 )
		{
			printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
			printf( "Error : %s %s\n", parts[0].c_str(), "needs 2 parameters" );
			return 0; // error
		}

		if( !is_valid_number( parts[1] ) )
		{
			printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
			printf( "%s is not a valid integer value\n", parts[1].c_str() );
			return 0; // error
		}

		levelengine_version = string_to_int( parts[1] );
	}
	else
	{
		printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
		printf( "Unknown Command : %s\n", parts[0].c_str() );
		return 0; // error
	}

	return 1; // Succesful
}

bool cLevel :: read_color_info( string *parts, unsigned int count, unsigned int line, Color &color )
{
	if( count != 4 )
	{
		printf( "%s : line %d Error : \n", Get_filename( 0, 0 ).c_str(), line );
		printf( "Error : %s %s\n", parts[0].c_str(), "needs 4 parameters" );
		return 0; // error
	}

	if( !is_valid_number( parts[1] ) )
	{
		printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
		printf( "%s is not a valid integer value\n", parts[1].c_str() );
		return 0; // error
	}
	if( !is_valid_number( parts[2] ) )
	{
		printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
		printf( "%s is not a valid integer value\n", parts[2].c_str() );
		return 0; // error
	}
	if( !is_valid_number( parts[3] ) )
	{
		printf( "%s : line %d Error : ", Get_filename( 0, 0 ).c_str(), line );
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
