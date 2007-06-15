/***************************************************************************
     levelexit.cpp  -  little point to enter exit the current level
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../objects/levelexit.h"
#include "../player/player.h"
#include "../core/game_core.h"
#include "../user/preferences.h"
#include "../input/joystick.h"
#include "../core/camera.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../level/level.h"
#include "../gui/hud.h"
#include "../input/keyboard.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLevelExit :: cLevelExit( float x, float y )
: cImageObjectSprite( x, y )
{
	Init();
}

cLevelExit :: cLevelExit( XMLAttributes &attributes )
: cImageObjectSprite()
{
	Init();
	Create_from_Stream( attributes );
}

cLevelExit :: ~cLevelExit( void )
{

}

void cLevelExit :: Init( void )
{
	sprite_array = ARRAY_ACTIVE;
	type = TYPE_LEVELEXIT;
	massivetype = MASS_PASSIVE;
	editor_posz = 0.111f;
	player_range = 1000;

	rect.w = 10;
	rect.h = 20;
	col_rect.w = rect.w;
	col_rect.h = rect.h;
	start_rect.w = rect.w;
	start_rect.h = rect.h;

    levelchange_type = LEVELEXIT_BEAM;

	Set_Direction( DIR_DOWN );

	col_change = lila;
	col_change.alpha = 128;
	col_exit = red;
	col_exit.alpha = 128;
}

cLevelExit *cLevelExit :: Copy( void )
{
	cLevelExit *levelexit = new cLevelExit( startposx, startposy );
	levelexit->Set_Type( levelchange_type );
	levelexit->Set_Direction( start_direction );
	levelexit->Set_Level( levelname );

	return levelexit;
}

void cLevelExit :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// type
	Set_Type( (Level_Exit_type)attributes.getValueAsInteger( "type", levelchange_type ) );
	// destination level
	Set_Level( attributes.getValueAsString( "level_name" ).c_str() );
	// direction
	if( levelchange_type == LEVELEXIT_WARP )
	{
		Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ) );
	}
}

void cLevelExit :: Save_to_Stream( ofstream &file )
{
	// begin levelexit
	file << "\t<levelexit>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// type
	file << "\t\t<Property name=\"type\" value=\"" << levelchange_type << "\" />" << std::endl;

	// destination level name
	string str_level = Get_Level( 0, 0 );
	if( !str_level.empty() )
	{
		file << "\t\t<Property name=\"level_name\" value=\"" << str_level << "\" />" << std::endl;
	}

	if( levelchange_type == LEVELEXIT_WARP )
	{
		// direction
		file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;
	}

	// end levelexit
	file << "\t</levelexit>" << std::endl;
}

void cLevelExit :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( direction == dir )
	{
		return;
	}

	cImageObjectSprite::Set_Direction( dir, 1 );

	Create_Name();
}

void cLevelExit :: Create_Name( void )
{
    name = "Level";

    if( levelchange_type == LEVELEXIT_BEAM )
    {
        name += " Beam";
    }
    else if( levelchange_type == LEVELEXIT_WARP )
    {
        name += " Warp";

        if( direction == DIR_UP )
        {
            name += " U";
        }
        else if( direction == DIR_LEFT )
        {
            name += " L";
        }
        else if( direction == DIR_DOWN )
        {
            name += " D";
        }
        else if( direction == DIR_RIGHT )
        {
            name += " R";
        }
    }
}

void cLevelExit :: Update( void )
{
	if( !is_Visible_onScreen() )
	{
		return;
	}

	// colliding with player
	if( Col_Box( &pPlayer->col_rect, &col_rect ) )
	{
		if( levelchange_type == LEVELEXIT_BEAM )
		{
			if( pPlayer->ground_object && ( pKeyboard->keys[pPreferences->key_up] || pJoystick->Button( pPreferences->joy_jump ) ) )
			{
				Activate();
			}
		}
		else if( levelchange_type == LEVELEXIT_WARP )
		{
			if( direction == DIR_DOWN )
			{
				if( ( pKeyboard->keys[pPreferences->key_down] || pJoystick->down ) )
				{
					Activate();
				}
			}
			else if( direction == DIR_UP )
			{
				if( pPlayer->vely <= 0 && ( pKeyboard->keys[pPreferences->key_up] || pJoystick->up ) )
				{
					Activate();
				}
			}
			else if( direction == DIR_RIGHT )
			{
				if( pPlayer->velx <= 0 && ( pKeyboard->keys[pPreferences->key_right] || pJoystick->right ) )
				{
					Activate();
				}
			}
			else if( direction == DIR_LEFT )
			{
				if( pPlayer->velx >= 0 && ( pKeyboard->keys[pPreferences->key_left] || pJoystick->left ) )
				{
					Activate();
				}
			}
		}
	}
}

void cLevelExit :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !valid_draw )
	{
		return;
	}

	// editor enabled
	if( !editor_level_enabled )
	{
		return;
	}

	// red for exit
	if( levelname.empty() )
	{
		pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, editor_posz, &col_exit );
	}
	// lila for change
	else
	{
		pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, editor_posz, &col_change );
	}
}

void cLevelExit :: Activate( void )
{
	pAudio->FadeOutMusic( 500 );

	// warp player out
	if( levelchange_type == LEVELEXIT_WARP )
	{
		pAudio->PlaySound( "enter_pipe.ogg" );

		if( pPlayer->ducked )
		{
			pPlayer->Stop_ducking();
		}

		float player_posz = pPlayer->posz;
		pPlayer->posz = 0.0799f; // just before massive

		for( float i = 0; i < ( DESIRED_FPS * 0.9 ); i += pFramerate->speedfactor )
		{
			float move_x = 0;
			float move_y = 0;

			if( direction == DIR_DOWN )
			{
				move_y = 2.7f;
			}
			else if( direction == DIR_UP )
			{
				move_y = -2.7f;
			}
			else if( direction == DIR_RIGHT )
			{
				move_x = 2.7f;
			}
			else if( direction == DIR_LEFT )
			{
				move_x = -2.7f;
			}

			pPlayer->Move( move_x, move_y );
			pLevel->Draw_Layer1( LVL_DRAW_BG );
			pPlayer->Set_Image( pPlayer->direction );
			pPlayer->Draw_Player();
			pLevel->Draw_Layer1( LVL_DRAW_NO_BG );
			pLevel->Draw_Layer2( LVL_DRAW_NO_BG );
			pHudManager->Draw();

			pVideo->Render();
			pFramerate->Update();
		}

		pPlayer->posz = player_posz;
	}

	if( levelname.length() )
	{
		pPlayer->Goto_SubLevel( levelname );
	}
	else
	{
		pAudio->PlayMusic( "game/courseclear.ogg" );
		pPlayer->Goto_next_Level();
	}
}

void cLevelExit :: Set_Type( Level_Exit_type ltype )
{
	levelchange_type = ltype;

	Create_Name();
}

void cLevelExit :: Set_Level( string filename )
{
	if( filename.empty() )
	{
		levelname.clear();
		return;
	}

	// erase file type if set
	if( filename.rfind( ".txt" ) != string::npos || filename.rfind( ".smclvl" ) != string::npos )
	{
		filename.erase( filename.rfind( "." ) );
	}

	if( filename.find( pPreferences->level_dir ) != string::npos )
	{
		filename.erase( 0, pPreferences->level_dir.length() + 1 );
	}

	levelname = filename;
}

string cLevelExit :: Get_Level( bool with_dir /* = 1 */, bool with_end /* = 1 */ )
{
	string name = levelname;

	// add directory
	if( with_dir && name.find( pPreferences->level_dir ) == string::npos )
	{
		name.insert( 0, pPreferences->level_dir + "/" );
	}

	// add file type
	if( with_end && name.find( ".smclvl" ) == string::npos )
	{
		name.insert( name.length(), ".smclvl" );
	}
	else if( !with_end )
	{
		// erase file type if set
		if( name.rfind( ".txt" ) != string::npos || name.rfind( ".smclvl" ) != string::npos )
		{
			name.erase( name.rfind( "." ) );
		}
	}

	return name;
}

bool cLevelExit :: is_Draw_valid( void )
{
	// if editor not enabled
	if( !editor_enabled )
	{
		// not visible
		if( !visible || !image )
		{
			return 0;
		}
	}

	// not visible on the screen
	if( !is_Visible_onScreen() )
	{
		return 0;
	}

	return 1;
}

void cLevelExit :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// warp
	if( levelchange_type == LEVELEXIT_WARP )
	{
		// direction
		Combobox *combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "levelexit_direction" ));
		combobox->getEditbox()->setTooltipText( "Direction" );
		Editor_Add( combobox, 100, 105 );

		combobox->addItem( new ListboxTextItem( "up" ) );
		combobox->addItem( new ListboxTextItem( "down" ) );
		combobox->addItem( new ListboxTextItem( "right" ) );
		combobox->addItem( new ListboxTextItem( "left" ) );
		combobox->setText( Get_Direction_name( start_direction ) );

		combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cLevelExit::Editor_Direction_Select, this ) );
	}

	// destination level
	Editbox *editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "levelexit_destination_level" ));
	editbox->setTooltipText( "Destination Level" );
	Editor_Add( editbox, 150 );

	editbox->setText( Get_Level( 0, 0 ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevelExit::Editor_Destination_Level_Key, this ) );

	// set position
	Editor_pos_update();
}

bool cLevelExit :: Editor_Direction_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_id( item->getText().c_str() ) );

	return 1;
}

bool cLevelExit :: Editor_Destination_Level_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Level( str_text );

	return 1;
}
