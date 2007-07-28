/***************************************************************************
     levelexit.cpp  -  area to exit the current level
                             -------------------
    copyright            :	(C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../objects/levelexit.h"
#include "../player/player.h"
#include "../core/game_core.h"
#include "../user/preferences.h"
#include "../core/camera.h"
#include "../audio/audio.h"
#include "../core/framerate.h"
#include "../core/main.h"
#include "../video/gl_surface.h"
#include "../video/font.h"
#include "../video/renderer.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel_Exit :: cLevel_Exit( float x, float y )
: cImageObjectSprite( x, y )
{
	Init();
}

cLevel_Exit :: cLevel_Exit( XMLAttributes &attributes )
: cImageObjectSprite()
{
	Init();
	Create_from_Stream( attributes );
}

cLevel_Exit :: ~cLevel_Exit( void )
{
	if( editor_entry_name )
	{
		delete editor_entry_name;
		editor_entry_name = NULL;
	}
}

void cLevel_Exit :: Init( void )
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

    exit_type = LEVELEXIT_BEAM;

	Set_Direction( DIR_DOWN );

	editor_color = red;
	editor_color.alpha = 128;

	editor_entry_name = NULL;
}

cLevel_Exit *cLevel_Exit :: Copy( void )
{
	cLevel_Exit *level_exit = new cLevel_Exit( startposx, startposy );
	level_exit->Set_Type( exit_type );
	level_exit->Set_Direction( start_direction );
	level_exit->Set_Level( dest_level );

	return level_exit;
}

void cLevel_Exit :: Create_from_Stream( XMLAttributes &attributes )
{
	// position
	Set_Pos( (float)attributes.getValueAsInteger( "posx" ), (float)attributes.getValueAsInteger( "posy" ), 1 );
	// type
	Set_Type( (Level_Exit_type)attributes.getValueAsInteger( "type", exit_type ) );
	// destination level
	Set_Level( attributes.getValueAsString( "level_name" ).c_str() );
	// destination entry
	Set_Entry( attributes.getValueAsString( "entry" ).c_str() );
	// direction
	if( exit_type == LEVELEXIT_WARP )
	{
		Set_Direction( Get_Direction_id( attributes.getValueAsString( "direction", Get_Direction_name( start_direction ) ).c_str() ) );
	}
}

void cLevel_Exit :: Save_to_Stream( ofstream &file )
{
	// begin levelexit
	file << "\t<levelexit>" << std::endl;

	// position
	file << "\t\t<Property name=\"posx\" value=\"" << (int)startposx << "\" />" << std::endl;
	file << "\t\t<Property name=\"posy\" value=\"" << (int)startposy << "\" />" << std::endl;
	// type
	file << "\t\t<Property name=\"type\" value=\"" << exit_type << "\" />" << std::endl;

	// destination level name
	string str_level = Get_Level( 0, 0 );
	if( !str_level.empty() )
	{
		file << "\t\t<Property name=\"level_name\" value=\"" << string_to_xml_string( str_level ) << "\" />" << std::endl;
	}

	// destination entry name
	if( !dest_entry.empty() )
	{
		file << "\t\t<Property name=\"entry\" value=\"" << string_to_xml_string( dest_entry ) << "\" />" << std::endl;
	}

	if( exit_type == LEVELEXIT_WARP )
	{
		// direction
		file << "\t\t<Property name=\"direction\" value=\"" << Get_Direction_name( start_direction ) << "\" />" << std::endl;
	}

	// end levelexit
	file << "\t</levelexit>" << std::endl;
}

void cLevel_Exit :: Set_Direction( ObjectDirection dir )
{
	// already set
	if( direction == dir )
	{
		return;
	}

	cImageObjectSprite::Set_Direction( dir, 1 );

	Create_Name();
}

void cLevel_Exit :: Create_Name( void )
{
    name = "Level";

    if( exit_type == LEVELEXIT_BEAM )
    {
        name += " Beam";
    }
    else if( exit_type == LEVELEXIT_WARP )
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

void cLevel_Exit :: Draw( cSurfaceRequest *request /* = NULL */ )
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

	// draw color rect
	pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, editor_posz, &editor_color );

	// draw destination entry name
	if( editor_entry_name )
	{
		// create request
		cSurfaceRequest *surface_request = new cSurfaceRequest();
		// blit
		editor_entry_name->Blit( col_rect.x + col_rect.w + 5 - pCamera->x, col_rect.y - pCamera->y, editor_posz, surface_request );
		surface_request->shadow_pos = 2;
		surface_request->shadow_color = lightgreyalpha64;
		// add request
		pRenderer->Add( surface_request );
	}
}

void cLevel_Exit :: Activate( void )
{
	pAudio->FadeOutMusic( 500 );

	// warp player out
	if( exit_type == LEVELEXIT_WARP )
	{
		pAudio->PlaySound( "enter_pipe.ogg" );

		pPlayer->Set_MovingState( STA_FALL );
		pPlayer->Set_Image( pPlayer->Get_Image() + pPlayer->direction );
		pPlayer->Stop_ducking();
		pPlayer->Reset_onGround();

		// set position and image
		if( direction == DIR_UP || direction == DIR_DOWN )
		{
			pPlayer->Set_PosX( posx + ( col_rect.w * 0.5f ) - ( pPlayer->col_rect.w * 0.5f ) );
		}
		else if( direction == DIR_LEFT || direction == DIR_RIGHT )
		{
			pPlayer->Set_PosY( posy + ( col_rect.h * 0.5f ) - ( pPlayer->col_rect.h * 0.5f ) );

			// set rotation
			if( direction == DIR_RIGHT )
			{
				pPlayer->Set_RotationZ( 90 );
			}
			else if( direction == DIR_LEFT )
			{
				pPlayer->Set_RotationZ( 270 );
			}
		}

		float player_posz = pPlayer->posz;
		pPlayer->posz = 0.0799f; // just before massive

		for( float i = 0; i < ( DESIRED_FPS * 0.9f ); i += pFramerate->speedfactor )
		{
			if( direction == DIR_DOWN )
			{
				pPlayer->Move( 0, 2.7f );
			}
			else if( direction == DIR_UP )
			{
				pPlayer->Move( 0, -2.7f );
			}
			else if( direction == DIR_RIGHT )
			{
				pPlayer->Move( 2.7f, 0 );
			}
			else if( direction == DIR_LEFT )
			{
				pPlayer->Move( -2.7f, 0 );
			}

			// draw
			Draw_Game();

			pVideo->Render();
			pFramerate->Update();
		}

		// set position z back
		pPlayer->posz = player_posz;
		// set invisible
		pPlayer->Set_Visible( 0 );

		if( direction == DIR_RIGHT || direction == DIR_LEFT )
		{
			pPlayer->Set_RotationZ( 0 );
		}
	}

	pPlayer->Collisions_Clear();

	if( dest_level.length() )
	{
		pPlayer->Goto_SubLevel( dest_level, dest_entry );
	}
	else
	{
		pAudio->PlayMusic( "game/courseclear.ogg" );
		pPlayer->Goto_next_Level();
	}
}

void cLevel_Exit :: Set_Type( Level_Exit_type ltype )
{
	exit_type = ltype;

	Create_Name();
}

void cLevel_Exit :: Set_Level( string filename )
{
	if( filename.empty() )
	{
		dest_level.clear();
		// red for no destination level
		editor_color = red;
		editor_color.alpha = 128;
		return;
	}

	// lila for set destination level
	editor_color = lila;
	editor_color.alpha = 128;

	// erase file type if set
	if( filename.rfind( ".txt" ) != string::npos || filename.rfind( ".smclvl" ) != string::npos )
	{
		filename.erase( filename.rfind( "." ) );
	}

	if( filename.find( pPreferences->level_dir ) != string::npos )
	{
		filename.erase( 0, pPreferences->level_dir.length() + 1 );
	}

	dest_level = filename;
}

void cLevel_Exit :: Set_Entry( string entry_name )
{
	if( editor_entry_name )
	{
		delete editor_entry_name;
		editor_entry_name = NULL;
	}

	// Set new name
	dest_entry = entry_name;

	// if empty don't create editor image
	if( dest_entry.empty() )
	{
		return;
	}

	editor_entry_name = pFont->RenderText( pFont->font_small, dest_entry, white );
}

string cLevel_Exit :: Get_Level( bool with_dir /* = 1 */, bool with_end /* = 1 */ )
{
	string name = dest_level;

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

bool cLevel_Exit :: is_Draw_valid( void )
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

void cLevel_Exit :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// warp
	if( exit_type == LEVELEXIT_WARP )
	{
		// direction
		Combobox *combobox = static_cast<Combobox *>(wmgr.createWindow( "TaharezLook/Combobox", "level_exit_direction" ));
		combobox->getEditbox()->setTooltipText( "Direction" );
		Editor_Add( combobox, 100, 105 );

		combobox->addItem( new ListboxTextItem( "up" ) );
		combobox->addItem( new ListboxTextItem( "down" ) );
		combobox->addItem( new ListboxTextItem( "right" ) );
		combobox->addItem( new ListboxTextItem( "left" ) );
		combobox->setText( Get_Direction_name( start_direction ) );

		combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cLevel_Exit::Editor_Direction_Select, this ) );
	}

	// destination level
	Editbox *editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "level_exit_destination_level" ));
	editbox->setTooltipText( "Destination Level" );
	Editor_Add( editbox, 150 );

	editbox->setText( Get_Level( 0, 0 ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Exit::Editor_Destination_Level_Key, this ) );

	// destination entry
	editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "level_exit_destination_entry" ));
	editbox->setTooltipText( "Destination Entry" );
	Editor_Add( editbox, 150 );

	editbox->setText( dest_entry.c_str() );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Exit::Editor_Destination_Entry_Key, this ) );

	// set position
	Editor_pos_update();
}

bool cLevel_Exit :: Editor_Direction_Select( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	ListboxItem *item = static_cast<Combobox *>( windowEventArgs.window )->getSelectedItem();

	Set_Direction( Get_Direction_id( item->getText().c_str() ) );

	return 1;
}

bool cLevel_Exit :: Editor_Destination_Level_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Level( str_text );

	return 1;
}

bool cLevel_Exit :: Editor_Destination_Entry_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	Set_Entry( str_text );

	return 1;
}
