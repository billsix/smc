/***************************************************************************
     levelexit.cpp  -  class for a little point to enter another level or exit/finish the current
                             -------------------
    copyright            :	(C) 2003 - 2006 by FluXy
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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLevelExit :: cLevelExit( double x, double y, Levelchange_type ntype /* = LVLCHANGE_EXIT */, string nlevelname /* = "" */ ) : cActiveSprite( x, y )
{
	type = TYPE_LEVELEXIT;
	massivetype = MASS_PASSIVE;

	rect.w = 20;
	rect.h = 20;
	col_rect.w = 20;
	col_rect.h = 20;
	start_rect.w = 20;
	start_rect.h = 20;

	levelchange = ntype;
	Set_Level( nlevelname );

	col_change = lila;
	col_change.alpha = 128;
	col_exit = red;
	col_exit.alpha = 128;
}

cLevelExit :: ~cLevelExit( void )
{

}

void cLevelExit :: Update( void )
{
	if( !isVisible_onScreen() )
	{
		return;
	}

	if( ( levelchange == LVLCHANGE_BEAM && ( keys[pPreferences->Key_up] || pJoystick->Button( pPreferences->Joy_jump ) ) && Col_Box( &pPlayer->col_rect, &col_rect ) && pPlayer->groundObject ) ||
		( levelchange == LVLCHANGE_WARP && ( keys[pPreferences->Key_down] || pJoystick->down ) && Col_Box( &pPlayer->col_rect, &col_rect ) ) )
	{
		pAudio->FadeOutMusic( 500 );
		
		if( levelchange == LVLCHANGE_WARP )
		{
			pAudio->PlaySound( SOUNDS_DIR "/pipe.ogg" );

			if( pPlayer->ducked )
			{
				pPlayer->Stop_ducking();
			}

			for( double i = 0; i < ( DESIRED_FPS * 0.9 ); i += Framerate.speedfactor )
			{
				pPlayer->Move( 0, 2.7, 0, 1, 0 );
				pLevel->Draw( LVL_DRAW_BG );
				pPlayer->Draw_Player( pPlayer->direction );
				pLevel->Draw( LVL_DRAW_NO_BG );

				SDL_GL_SwapBuffers();
				Framerate.Update();
			}
		}

		if( levelname.length() )
		{
			pPlayer->GotoSubLevel( levelname );
		}
		else
		{
			pAudio->PlaySound( MUSIC_DIR "/game/courseclear.ogg" );
			pPlayer->GotoNextLevel();
		}
	}
	else
	{
		Draw();
	}
}

void cLevelExit :: Draw( void )
{
	if( !Leveleditor_Mode )
	{
		rect.x = startposx;
		rect.y = startposy;
		col_rect.x = startposx + col_pos.x;
		col_rect.y = startposy + col_pos.y;
		start_rect.x = startposx + col_pos.x;
		start_rect.y = startposy + col_pos.y;

		return;
	}
	
	rect.x = startposx;
	rect.y = startposy;
	col_rect.x = startposx + col_pos.x;
	col_rect.y = startposy + col_pos.y;
	start_rect.x = startposx + col_pos.x;
	start_rect.y = startposy + col_pos.y;

	if( !isVisible_onScreen() ) 
	{
		return;
	}

	if( levelname.empty() ) // red for levelexit
	{
		DrawRect( col_rect.x - cameraposx, col_rect.y - cameraposy, col_rect.w, col_rect.h, &col_exit ); 
	}
	else // lila for levelchange
	{
		DrawRect( col_rect.x - cameraposx, col_rect.y - cameraposy, col_rect.w, col_rect.h, &col_change ); 
	}
}

void cLevelExit :: Set_Level( string filename )
{
	if( filename.empty() )
	{
		levelname.clear();
		return;
	}

	levelname = filename;

	if( levelname.find( ".txt" ) == string::npos ) 
	{
		levelname.insert( levelname.length(), ".txt" );
	}

	if( levelname.find( pPreferences->level_dir ) != string::npos ) 
	{
		levelname.erase( 0, pPreferences->level_dir.length() + 1 );
	}
}

string cLevelExit :: Get_Level( bool with_dir /* = 1 */, bool with_end /* = 1 */ )
{
	string name = levelname;

	if( with_dir && name.find( pPreferences->level_dir ) == string::npos ) 
	{
		name.insert( 0, "/" );
		name.insert( 0, pPreferences->level_dir );
	}

	if( with_end && name.find( ".txt" ) == string::npos ) 
	{
		name.insert( name.length(), ".txt" );
	}
	else if( !with_end && name.find( ".txt" ) != string::npos ) 
	{
		name.erase( name.find( ".txt" ), 4 );
	}

	return name;
}

void cLevelExit :: Editor_Activate( void )
{
	pDialogManager->Add( 0, 0, "destination_level", Get_Level( 0, 0 ), TYPE_DIALOG_ALL, 100, 20 );

	// set position
	Editor_pos_update();
}

void cLevelExit :: Editor_Deactivate( void )
{
	// remove
	pDialogManager->Delete( "destination_level" );
}

void cLevelExit :: Editor_pos_update( void )
{
	cDialog *dialog = pDialogManager->Get( "destination_level" );

	if( dialog )
	{
		dialog->SetPos( posx + col_rect.w + 5, posy + 5 );
	}
}

void cLevelExit :: Editor_val_update( void )
{
	cDialog *dialog = pDialogManager->Get( "destination_level" );

	if( dialog )
	{
		// update
		if( dialog->gotChanged() )
		{
			Set_Level( dialog->Get_Value_string() );
		}
	}
}
