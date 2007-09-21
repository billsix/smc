/***************************************************************************
			level_settings.cpp  - class for the level editor settings
                             -------------------
    copyright            : (C) 2006 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/globals.h"
#include "../core/main.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../level/level_settings.h"
#include "../input/mouse.h"
#include "../level/level.h"
#include "../video/font.h"
#include "../video/video.h"
#include "../video/renderer.h"
#include "../video/gl_surface.h"

/* *** *** *** *** *** cLevel_Settings *** *** *** *** *** *** *** *** *** *** *** *** */

cLevel_Settings :: cLevel_Settings( void )
{
	// Main
	background_preview = new cHudSprite( NULL, GAME_RES_W * 0.73f, GAME_RES_H * 0.33f );
	background_preview->Set_Shadow( lightgrey, 2 );
	// Global Effects
	global_effect_preview = new cHudSprite( NULL, GAME_RES_W * 0.75f, GAME_RES_H * 0.15f );
	global_effect_preview->Set_Shadow( lightgrey, 2 );
	
	tabcontrol = NULL;

	Init();

	active = 0;
}

cLevel_Settings :: ~cLevel_Settings( void )
{
	delete background_preview;
	delete global_effect_preview;

	pGuiSystem->getGUISheet()->removeChildWindow( guiwindow );
	WindowManager::getSingleton().destroyWindow( guiwindow );
}

void cLevel_Settings :: Init( void )
{
	// ## Gui
	guiwindow = WindowManager::getSingleton().loadWindowLayout( "level_settings.layout" );
	pGuiSystem->getGUISheet()->addChildWindow( guiwindow );
	
	// Tab Control
	tabcontrol = static_cast<TabControl *>(WindowManager::getSingleton().getWindow( "tabcontrol_main" ));
	// tab main
	Window *tabwindow = WindowManager::getSingleton().loadWindowLayout( "level_settings_tab_main.layout" );
	tabcontrol->addTab( tabwindow );
	// Tab background
	tabwindow = WindowManager::getSingleton().loadWindowLayout( "level_settings_tab_background.layout" );
	tabcontrol->addTab( tabwindow );
	// tab global effects
	tabwindow = WindowManager::getSingleton().loadWindowLayout( "level_settings_tab_global_effect.layout" );
	tabcontrol->addTab( tabwindow );

	// Main
	// level filename
	Editbox *editbox_level_filename = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_level_filename" ));
	editbox_level_filename->setText( Get_filename( pLevel->data_file, 0, 0 ).c_str() );
	// music filename
	Editbox *editbox_music_filename = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_music_filename" ));
	editbox_music_filename->setText( pLevel->Get_Musicfile( 1 ).c_str() );
	// author
	Editbox *editbox_author = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_author" ));
	editbox_author->setText( pLevel->author.c_str() );
	// version
	Editbox *editbox_version = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_version" ));
	editbox_version->setText( pLevel->version.c_str() );
	// camera limits
	Spinner *spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_camera_limit_w" ));
	spinner->setCurrentValue( pCamera->limit_rect.w );
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_camera_limit_h" ));
	spinner->setCurrentValue( pCamera->limit_rect.h );
	// last Save time
	Editbox *editbox_save_time = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_save_time" ));
	editbox_save_time->setText( Time_to_String( pLevel->last_saved, "%Y-%m-%d  %H:%M:%S" ) );


	// add background image button
	PushButton *button_add_background_image = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "button_add_background_image" ));
	button_add_background_image->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cLevel_Settings::Add_Background_image, this ) );
	// delete background image button
	PushButton *button_delete_background_image = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "button_delete_background_image" ));
	button_delete_background_image->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cLevel_Settings::Delete_Background_image, this ) );
	// save and exit button
	PushButton *button_save = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "button_save" ));
	button_save->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cLevel_Settings::Button_Save, this ) );

	// Background
	// listbox
	Listbox *listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_backgrounds" ));
	listbox->subscribeEvent( Listbox::EventSelectionChanged, Event::Subscriber( &cLevel_Settings::Set_Background_image, this ) );
	// type
	Combobox *combobox = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_bg_image_type" ));
	ListboxTextItem *item = new ListboxTextItem( "Disabled", 1 );
	combobox->addItem( item );
	item = new ListboxTextItem( "Left and Right", 2 );
	combobox->addItem( item );
	combobox->subscribeEvent( Combobox::EventListSelectionAccepted, Event::Subscriber( &cLevel_Settings::Update_BG_image, this ) );
	// filename
	Editbox *editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_image_name" ));
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_image, this ) );
	// speed
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_x" ));
	spinner->subscribeEvent( Spinner::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_image, this ) );
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_y" ));
	spinner->subscribeEvent( Spinner::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_image, this ) );
	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_image_posz" ));
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_image, this ) );
	// Gradient colors
	bg_color_1 = Color( pLevel->backgrounds[0]->color_1.red, pLevel->backgrounds[0]->color_1.green, pLevel->backgrounds[0]->color_1.blue, 255 );
	bg_color_2 = Color( pLevel->backgrounds[0]->color_2.red, pLevel->backgrounds[0]->color_2.green, pLevel->backgrounds[0]->color_2.blue, 255 );

	// color 1
	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_color_1_red" ));
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_1.red ).c_str() );
	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_color_1_green" ));
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_1.green ).c_str() );
	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_color_1_blue" ));
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_1.blue ).c_str() );
	// color 2
	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_color_2_red" ));
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_2.red ).c_str() );
	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_color_2_green" ));
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_2.green ).c_str() );
	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_color_2_blue" ));
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_2.blue ).c_str() );

	Update_BG_Colors( EventArgs() );

	// global effect
	// filename
	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_global_effect_file" ));
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_global_effect_image, this ) );
	// type
	combobox = static_cast<Combobox *>(WindowManager::getSingleton().getWindow( "combo_global_effect_type" ));
	item = new ListboxTextItem( "Disabled", 1 );
	combobox->addItem( item );
	item = new ListboxTextItem( "Falling", 2 );
	combobox->addItem( item );
	//item = new ListboxTextItem( "Flying", 3 );
	//combobox->addItem( item );
	// Z Position
	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_global_effect_pos_z" ));
	editbox->setText( float_to_string( pLevel->pGlobal_effect->posz ) );
	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_global_effect_pos_z_rand" ));
	editbox->setText( float_to_string( pLevel->pGlobal_effect->posz_rand ) );
	// creation rect
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_x" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->startposx );
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_y" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->startposy );
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_w" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->rect.w );
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_h" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->rect.h );
	// time to live
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_time_to_live" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->time_to_live );
	// scale
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_scale" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->size_scale );
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_scale_rand" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->size_scale_rand );
	// emitter iteration interval
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_emitter_iteration_interval" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->emitter_iteration_interval );
	// velocity
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_vel" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->vel );
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_vel_rand" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->vel_rand );
	// angle
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_angle_start" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->angle_start );
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_angle_range" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->angle_range );
	// constant rotation z
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_const_rot_z" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->const_rotz );
	spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_const_rot_z_rand" ));
	spinner->setCurrentValue( pLevel->pGlobal_effect->const_rotz_rand );

	Clear_Layer_field();

	Load_BG_imageList();
	Load_global_effect();

	Update_global_effect_image( EventArgs() );
}

void cLevel_Settings :: Enter( void )
{
	Change_Game_Mode( MODE_LEVEL_SETTINGS );

	active = 1;

	while( active )
	{
		// input
		while( SDL_PollEvent( &input_event ) )
		{
			// was handled
			if( Handle_Input_Global( &input_event ) )
			{
				continue;
			}

			switch( input_event.type )
			{
				case SDL_QUIT:
				{
					done = 1;
					active = 0;
					break;
				}
				case SDL_KEYDOWN:
				{
					if( input_event.key.keysym.sym == SDLK_ESCAPE )
					{
						active = 0;
					}
					break;
				}
				default:
				{
					break;
				}
			}
		}

		// Clear Screen
		pVideo->Clear_Screen();
		// Mouse Position update
		pMouseCursor->Update_Position();

		// background Tab
		if( tabcontrol->getSelectedTabIndex() == 1 )
		{
			// create request
			cSurfaceRequest *request = new cSurfaceRequest();
			// draw background image preview
			background_preview->Draw( request );
			// add request
			pRenderer_GUI->Add( request );
		}
		// Global Effect Tab
		else if( tabcontrol->getSelectedTabIndex() == 2 )
		{
			// create request
			cSurfaceRequest *request = new cSurfaceRequest();
			// draw global effect preview
			global_effect_preview->Draw( request );
			// add request
			pRenderer_GUI->Add( request );
		}

		// draw
		pVideo->Render();

		// gui
		Gui_handle_Time();
	}

	// ### Save Changes

	// # Main
	// filename
	if( Get_filename( pLevel->data_file, 0, 0 ).compare( WindowManager::getSingleton().getWindow( "editbox_level_filename" )->getText().c_str() ) != 0 )
	{
		pLevel->Set_Levelfile( WindowManager::getSingleton().getWindow( "editbox_level_filename" )->getText().c_str() );
		// show no level saved info text
		debugdisplay->Set_Text( "" );
	}
	// musicfile
	pLevel->Set_Musicfile( WindowManager::getSingleton().getWindow( "editbox_music_filename" )->getText().c_str() );
	// Author
	pLevel->Set_Author( WindowManager::getSingleton().getWindow( "editbox_author" )->getText().c_str() );
	// Version
	pLevel->Set_Version( WindowManager::getSingleton().getWindow( "editbox_version" )->getText().c_str() );
	// Camera Limits
	pCamera->Set_Limit_W( (static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_camera_limit_w" )))->getCurrentValue() );
	pCamera->Set_Limit_H( (static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_camera_limit_h" )))->getCurrentValue() );

	// # background
	// Gradient
	pLevel->backgrounds[0]->Set_color_1( bg_color_1 );
	pLevel->backgrounds[0]->Set_color_2( bg_color_2 );

	// # Global Effect
	pLevel->pGlobal_effect->image_filename = WindowManager::getSingleton().getWindow( "editbox_global_effect_file" )->getText().c_str();
	pLevel->pGlobal_effect->Set_ZPos( string_to_float( ( static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_global_effect_pos_z" )))->getText().c_str() ), string_to_float( ( static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_global_effect_pos_z_rand" )))->getText().c_str() ) );
	pLevel->pGlobal_effect->Set_type( WindowManager::getSingleton().getWindow( "combo_global_effect_type" )->getText().c_str() );
	pLevel->pGlobal_effect->Init();
	// creation rect
	Spinner *spinner_rect_x = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_x" ));
	Spinner *spinner_rect_y = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_y" ));
	Spinner *spinner_rect_w = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_w" ));
	Spinner *spinner_rect_h = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_rect_h" ));
	pLevel->pGlobal_effect->Set_Emitter_Rect( spinner_rect_x->getCurrentValue(), spinner_rect_y->getCurrentValue(), spinner_rect_w->getCurrentValue(), spinner_rect_h->getCurrentValue() );
	// lifetime
	Spinner *spinner_time_to_live = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_time_to_live" ));
	pLevel->pGlobal_effect->Set_Time_to_Live( spinner_time_to_live->getCurrentValue() );
	// scale
	Spinner *spinner_scale = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_scale" ));
	Spinner *spinner_scale_rand = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_scale_rand" ));
	pLevel->pGlobal_effect->Set_Scale( spinner_scale->getCurrentValue(), spinner_scale_rand->getCurrentValue() );
	// Emitter Iteration Interval
	Spinner *spinner_emitter_iteration_interval = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_emitter_iteration_interval" ));
	pLevel->pGlobal_effect->Set_Emitter_Iteration_Interval( spinner_emitter_iteration_interval->getCurrentValue() );
	// velocity
	Spinner *spinner_vel = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_vel" ));
	Spinner *spinner_vel_rand = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_vel_rand" ));
	pLevel->pGlobal_effect->Set_Speed( spinner_vel->getCurrentValue(), spinner_vel_rand->getCurrentValue() );
	// direction
	Spinner *spinner_angle_start = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_angle_start" ));
	Spinner *spinner_angle_range = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_angle_range" ));
	pLevel->pGlobal_effect->Set_DirectionRange( spinner_angle_start->getCurrentValue(), spinner_angle_range->getCurrentValue() );
	// constant rotation z
	Spinner *spinner_const_rot_z = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_const_rot_z" ));
	Spinner *spinner_const_rot_z_rand = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_global_effect_const_rot_z_rand" ));
	pLevel->pGlobal_effect->Set_ConstRotationZ( spinner_const_rot_z->getCurrentValue(), spinner_const_rot_z_rand->getCurrentValue() );

	// back to level mode
	Change_Game_Mode( MODE_LEVEL );
	pMouseCursor->Reset( 0 );
}

bool cLevel_Settings :: Add_Background_image( const EventArgs &event )
{
	cBackground *background = new cBackground();
	background->Set_type( BG_IMG_LEFTRIGHT );
	background->Set_image( LEVEL_DEFAULT_BACKGROUND );

	pLevel->backgrounds.push_back( background );

	Load_BG_imageList();

	return 1;
}

bool cLevel_Settings :: Delete_Background_image( const EventArgs &event )
{
	Listbox *listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_backgrounds" ));
	ListboxItem *item = listbox->getFirstSelectedItem();

	if( !item )
	{
		return 1;
	}

	int bg_num = string_to_int( item->getText().c_str() );

	delete pLevel->backgrounds[bg_num];
	pLevel->backgrounds.erase( pLevel->backgrounds.begin() + bg_num );

	// uddate list
	Load_BG_imageList();
	// clear bg image gui
	Update_BG_image( EventArgs() );

	return 1;
}

bool cLevel_Settings :: Set_Background_image( const EventArgs &event )
{
	Listbox *listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_backgrounds" ));
	ListboxItem *item = listbox->getFirstSelectedItem();

	if( item ) // selected
	{
		cBackground *background = pLevel->Get_background( string_to_int( item->getText().c_str() ) );
		string background_filename = background->img_file_1;

		// type
		Editbox *editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "combo_bg_image_type" ));
		editbox->setText( background->Get_Typename().c_str() );
		// filename
		editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_image_name" ));
		editbox->setText( background_filename.c_str() );
		// speed
		Spinner *spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_x" ));
		spinner->setCurrentValue( background->speedx );
		spinner = static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_y" ));
		spinner->setCurrentValue( background->speedy );
		// posz
		editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_image_posz" ));
		editbox->setText( float_to_string( background->posz ).c_str() );

		// set image preview
		background_filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
		Set_Background_image_Preview( background_filename );
	}
	else // deselected
	{
		Clear_Layer_field();
	}

	return 1;
}

bool cLevel_Settings :: Button_Save( const EventArgs &event )
{
	active = 0;
	return 1;
}

bool cLevel_Settings :: Update_BG_Colors( const EventArgs &event )
{
	bg_color_1.red = string_to_int( WindowManager::getSingleton().getWindow( "editbox_bg_color_1_red" )->getText().c_str() );
	bg_color_1.green = string_to_int( WindowManager::getSingleton().getWindow( "editbox_bg_color_1_green" )->getText().c_str() );
	bg_color_1.blue = string_to_int( WindowManager::getSingleton().getWindow( "editbox_bg_color_1_blue" )->getText().c_str() );
	bg_color_2.red = string_to_int( WindowManager::getSingleton().getWindow( "editbox_bg_color_2_red" )->getText().c_str() );
	bg_color_2.green = string_to_int( WindowManager::getSingleton().getWindow( "editbox_bg_color_2_green" )->getText().c_str() );
	bg_color_2.blue = string_to_int( WindowManager::getSingleton().getWindow( "editbox_bg_color_2_blue" )->getText().c_str() );


	// color 1
	Window *statictext = WindowManager::getSingleton().getWindow( "text_bg_color_1_red" );
	statictext->setProperty( "TextColours", PropertyHelper::colourToString( colour( 1, 1 - ( (float)bg_color_1.red / 255 ), 1 - ( (float)bg_color_1.red / 255 ), 1 ) ) );
	statictext = WindowManager::getSingleton().getWindow( "text_bg_color_1_green" );
	statictext->setProperty( "TextColours", PropertyHelper::colourToString( colour( 1 - ( (float)bg_color_1.green / 255 ), 1, 1 - ( (float)bg_color_1.green / 255 ), 1 ) ) );
	statictext = WindowManager::getSingleton().getWindow( "text_bg_color_1_blue" );
	statictext->setProperty( "TextColours", PropertyHelper::colourToString( colour( 1 - ( (float)bg_color_1.blue / 255 ), 1 - ( (float)bg_color_1.blue / 255 ), 1, 1 ) ) );
	// color 2
	statictext = WindowManager::getSingleton().getWindow( "text_bg_color_2_red" );
	statictext->setProperty( "TextColours", PropertyHelper::colourToString( colour( 1, 1 - ( (float)bg_color_2.red / 255 ), 1 - ( (float)bg_color_2.red / 255 ), 1 ) ) );
	statictext = WindowManager::getSingleton().getWindow( "text_bg_color_2_green" );
	statictext->setProperty( "TextColours", PropertyHelper::colourToString( colour( 1 - ( (float)bg_color_2.green / 255 ), 1, 1 - ( (float)bg_color_2.green / 255 ), 1 ) ) );
	statictext = WindowManager::getSingleton().getWindow( "text_bg_color_2_blue" );
	statictext->setProperty( "TextColours", PropertyHelper::colourToString( colour( 1 - ( (float)bg_color_2.blue / 255 ), 1 - ( (float)bg_color_2.blue / 255 ), 1, 1 ) ) );

	statictext = WindowManager::getSingleton().getWindow( "rect_color_1" );
	statictext->setProperty( "TextColours", PropertyHelper::colourToString( bg_color_1.Get_cegui_colour() ) );
	statictext = WindowManager::getSingleton().getWindow( "rect_color_2" );
	statictext->setProperty( "TextColours", PropertyHelper::colourToString( bg_color_2.Get_cegui_colour() ) );

	return 1;
}

void cLevel_Settings :: Load_BG_imageList( void )
{
	Listbox *listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_backgrounds" ));
	listbox->resetList();

	for( unsigned int i = 0; i < pLevel->Get_Backgroundlayers() - 1; i++ )
	{
		ListboxTextItem *item = new ListboxTextItem( int_to_string( i + 1 ).c_str() );
		listbox->addItem( static_cast<ListboxItem *>(item) );
	}

	PushButton *button_add = static_cast<PushButton *>(WindowManager::getSingleton().getWindow( "button_add_background_image" ));

	if( pLevel->Get_Backgroundlayers() >= 4 ) // 3 layers + default background maximum
	{
		button_add->disable();
	}
	else
	{
		button_add->enable();
	}
}

void cLevel_Settings :: Set_Background_image_Preview( string filename )
{
	Convert_path_separators( filename );

	if( !valid_file( filename ) )
	{
		background_preview->Set_Image( NULL );
		return;
	}

	GL_Surface *temp = pVideo->Load_Surface( filename );
	
	if( !temp )
	{
		printf( "Warning : Couldn't open Background preview : %s\n", filename.c_str() );
	}
	else
	{
		float zoom = pVideo->Get_scale( temp, 100, 100 );
		temp->w *= zoom;
		temp->h *= zoom;
		background_preview->Set_Image( temp, 0, 1 );
	}
}

bool cLevel_Settings :: Update_BG_image( const EventArgs &event )
{
	string bg_type = WindowManager::getSingleton().getWindow( "combo_bg_image_type" )->getText().c_str();
	string bg_filename = WindowManager::getSingleton().getWindow( "editbox_bg_image_name" )->getText().c_str();
	float speed_x = (static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_x" )))->getCurrentValue();
	float speed_y = (static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_y" )))->getCurrentValue();
	float posz = string_to_float( ( static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "editbox_bg_image_posz" )))->getText().c_str() );

	Listbox *listbox = static_cast<Listbox *>(WindowManager::getSingleton().getWindow( "listbox_backgrounds" ));
	ListboxItem *item = listbox->getFirstSelectedItem();

	if( !item ) // clear
	{
		Clear_Layer_field();
		return 1;
	}

	// full filename for validation
	bg_filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );

	cBackground *background = pLevel->Get_background( string_to_int( item->getText().c_str() ) );
	background->Set_type( bg_type );

	// valid
	if( valid_file( bg_filename ) )
	{
		background->Set_image( bg_filename );
	}
	// invalid
	else
	{
		// empty
		background->Set_image( "" );
	}

	background->Set_scroll_speed( speed_x, speed_y );
	background->Set_PosZ( posz );

	// update image preview
	Set_Background_image_Preview( bg_filename );

	return 1;
}

void cLevel_Settings :: Clear_Layer_field( void )
{
	(static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "combo_bg_image_type" )))->setText( "Disabled" );
	(static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_image_name" )))->setText( "" );
	(static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_x" )))->setText( "" );
	(static_cast<Spinner *>(WindowManager::getSingleton().getWindow( "spinner_bg_image_speed_y" )))->setText( "" );
	(static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_bg_image_posz" )))->setText( "" );
	Set_Background_image_Preview( "" );
}

void cLevel_Settings :: Load_global_effect( void )
{
	Editbox *editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "editbox_global_effect_file" ));
	editbox->setText( pLevel->pGlobal_effect->image_filename.c_str() );

	editbox = static_cast<Editbox *>(WindowManager::getSingleton().getWindow( "combo_global_effect_type" ));
	editbox->setText( pLevel->pGlobal_effect->Get_Typename().c_str() );
}

void cLevel_Settings :: Set_global_effect_image_Preview( string filename )
{
	Convert_path_separators( filename );

	if( !valid_file( filename ) )
	{
		global_effect_preview->Set_Image( NULL );
		return;
	}

	GL_Surface *temp = pVideo->Load_Surface( filename );
	
	if( !temp )
	{
		printf( "Warning : Couldn't open global effect preview : %s\n", filename.c_str() );
	}
	else
	{
		float zoom = pVideo->Get_scale( temp, 100, 100 );
		temp->w *= zoom;
		temp->h *= zoom;
		global_effect_preview->Set_Image( temp, 0, 1 );
	}
}

bool cLevel_Settings :: Update_global_effect_image( const EventArgs &event )
{
	string ge_filename = WindowManager::getSingleton().getWindow( "editbox_global_effect_file" )->getText().c_str();

	// image preview
	ge_filename.insert( 0, DATA_DIR "/" GAME_PIXMAPS_DIR "/" );
	Set_global_effect_image_Preview( ge_filename );

	return 1;
}
