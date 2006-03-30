/***************************************************************************
     level_settings.cpp  - settings class for the Leveleditor
                             -------------------
    copyright            : (C) 2006 by FluXy
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

cLevel_Settings :: cLevel_Settings( void )
{
	background_preview = new cSprite( NULL, GAME_RES_W * 0.5, GAME_RES_H * 0.56, 0, &lightgreyalpha, 1 );
	background_type = new cSprite( NULL, GAME_RES_W * 0.26, GAME_RES_H * 0.58, 0, NULL, 1 );

	pDialogManager->DeleteAll();
	Init();

	active = 0;
}

cLevel_Settings :: ~cLevel_Settings( void )
{
	delete text_caption;

	delete background_preview;
	delete background_type;

	WindowManager::getSingleton().destroyWindow( guiwindow );
	pDialogManager->DeleteAll();
}

void cLevel_Settings :: Init( void )
{
	// level settings caption
	text_caption = new cSprite( GetImage( "game/leveleditor/settings.png" ), GAME_RES_W * 0.35, 5, 0, NULL, 1 );

	// ## Gui
	guiwindow = WindowManager::getSingleton().loadWindowLayout( "data/gui/layout/level_settings.layout" );
	pGuiSystem->setGUISheet( guiwindow );
	
	// level filename
	Editbox *editbox_level_filename = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_level_filename" );
	editbox_level_filename->setText( pLevel->Get_filename( 0, 0 ) );
	// music filename
	Editbox *editbox_music_filename = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_music_filename" );
	editbox_music_filename->setText( pLevel->Get_Musicfile( 0 ) );
	// add background image button
	PushButton *button_add_background_image = (PushButton *)WindowManager::getSingleton().getWindow( "button_add_background_image" );
	button_add_background_image->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cLevel_Settings::Add_Background_image, this ) );
	// delete background image button
	PushButton *button_delete_background_image = (PushButton *)WindowManager::getSingleton().getWindow( "button_delete_background_image" );
	button_delete_background_image->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cLevel_Settings::Delete_Background_image, this ) );
	// save and exit button
	PushButton *button_save = (PushButton *)WindowManager::getSingleton().getWindow( "button_save" );
	button_save->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &cLevel_Settings::Button_Save, this ) );

	// ## Background Colors

	bg_color_1 = Get_Color( pLevel->backgrounds[0]->color_1.red, pLevel->backgrounds[0]->color_1.green, pLevel->backgrounds[0]->color_1.blue, 255 );
	bg_color_2 = Get_Color( pLevel->backgrounds[0]->color_2.red, pLevel->backgrounds[0]->color_2.green, pLevel->backgrounds[0]->color_2.blue, 255 );

	// ## editbox Events
	Editbox *editbox = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_color_1_red" );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_1.red ) );
	editbox = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_color_1_green" );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_1.green ) );
	editbox = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_color_1_blue" );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_1.blue ) );

	editbox = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_color_2_red" );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_2.red ) );
	editbox = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_color_2_green" );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_2.green ) );
	editbox = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_color_2_blue" );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_Colors, this ) );
	editbox->setText( int_to_string( bg_color_2.blue ) );

	editbox = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_image_name" );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_image, this ) );

	editbox = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_image_type" );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLevel_Settings::Update_BG_image, this ) );

	Update_BG_Colors( EventArgs() );

	Listbox *listbox = (Listbox *)WindowManager::getSingleton().getWindow( "listbox_backgrounds" );
	listbox->subscribeEvent( Listbox::EventSelectionChanged, Event::Subscriber( &cLevel_Settings::Set_Background_image, this ) );

	Load_BG_imageList();
}

void cLevel_Settings :: Enter( void )
{
	Game_Mode = MODE_LEVEL_SETTINGS;

	active = 1;

	while( active )
	{
		// Clear Screen
		ResetScreen();
		// Mouse Position update
		pMouseCursor->Update_Position();
		// sprites
		text_caption->Draw();
		// background color boxes
		DrawRect( 470, 168, 30, 30, &bg_color_1 );
		DrawRect( 470, 218, 30, 30, &bg_color_2 );
		// background image preview
		background_preview->Draw();
		// background image type text
		background_type->Draw();
		
		// gui
		pGuiSystem->renderGUI();
		// draw
		SDL_GL_SwapBuffers();

		// input
		while( SDL_PollEvent( &event ) )
		{
			HandleInput();

			switch( event.type )
			{
				case SDL_QUIT:
				{
					done = 1;
					active = 0;
					break;
				}
				case SDL_KEYDOWN:
				{
					if( event.key.keysym.sym == SDLK_ESCAPE )
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

		Gui_handle_Time();
	}

	// ### Save Changes
	if( pLevel->Get_filename( 0, 0 ).compare( WindowManager::getSingleton().getWindow( "editbox_level_filename" )->getText().c_str() ) != 0 )
	{
		pLevel->Set_Levelfile( WindowManager::getSingleton().getWindow( "editbox_level_filename" )->getText().c_str() );

		debugdisplay->counter = 0; // no level saved info
	}

	pLevel->Set_Musicfile( WindowManager::getSingleton().getWindow( "editbox_music_filename" )->getText().c_str() );

	pLevel->backgrounds[0]->Set_color_1( bg_color_1 );
	pLevel->backgrounds[0]->Set_color_2( bg_color_2 );

	Game_Mode = MODE_LEVEL;
	pMouseCursor->Reset();
}

bool cLevel_Settings :: Add_Background_image( const EventArgs &event )
{
	cBackground *background = new cBackground();
	background->Set_type( BG_IMG_LEFTRIGHT );
	background->Set_image( "game/background/green_junglehills.png" );

	pLevel->backgrounds.push_back( background );

	Load_BG_imageList();

	return 1;
}

bool cLevel_Settings :: Delete_Background_image( const EventArgs &event )
{
	Listbox *listbox = (Listbox *)WindowManager::getSingleton().getWindow( "listbox_backgrounds" );
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
	Listbox *listbox = (Listbox *)WindowManager::getSingleton().getWindow( "listbox_backgrounds" );
	ListboxItem *item = listbox->getFirstSelectedItem();

	if( item ) // selected
	{
		cBackground *background = pLevel->Get_background( string_to_int( item->getText().c_str() ) );
		string background_filename = background->img_file_1;

		Editbox *editbox = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_image_name" );
		editbox->setText( background_filename.c_str() );

		editbox = (Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_image_type" );
		editbox->setText( int_to_string( background->type ).c_str() );

		// type name
		background_type->SetImage( RenderText( font_small, background->Get_Typename(), black ), 0, 1 );

		// image preview
		background_filename.insert( 0, PIXMAPS_DIR "/" );
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
	StaticText *statictext = (StaticText *)WindowManager::getSingleton().getWindow( "text_bg_color_1_red" );
	statictext->setTextColours( colour( 1, 1 - ( (float)bg_color_1.red / 255 ), 1 - ( (float)bg_color_1.red / 255 ), 1 ) );
	statictext = (StaticText *)WindowManager::getSingleton().getWindow( "text_bg_color_1_green" );
	statictext->setTextColours( colour( 1 - ( (float)bg_color_1.green / 255 ), 1, 1 - ( (float)bg_color_1.green / 255 ), 1 ) );
	statictext = (StaticText *)WindowManager::getSingleton().getWindow( "text_bg_color_1_blue" );
	statictext->setTextColours( colour( 1 - ( (float)bg_color_1.blue / 255 ), 1 - ( (float)bg_color_1.blue / 255 ), 1, 1 ) );
	// color 2
	statictext = (StaticText *)WindowManager::getSingleton().getWindow( "text_bg_color_2_red" );
	statictext->setTextColours( colour( 1, 1 - ( (float)bg_color_2.red / 255 ), 1 - ( (float)bg_color_2.red / 255 ), 1 ) );
	statictext = (StaticText *)WindowManager::getSingleton().getWindow( "text_bg_color_2_green" );
	statictext->setTextColours( colour( 1 - ( (float)bg_color_2.green / 255 ), 1, 1 - ( (float)bg_color_2.green / 255 ), 1 ) );
	statictext = (StaticText *)WindowManager::getSingleton().getWindow( "text_bg_color_2_blue" );
	statictext->setTextColours( colour( 1 - ( (float)bg_color_2.blue / 255 ), 1 - ( (float)bg_color_2.blue / 255 ), 1, 1 ) );

	return 1;
}

void cLevel_Settings :: Load_BG_imageList( void )
{
	Listbox *listbox = (Listbox *)WindowManager::getSingleton().getWindow( "listbox_backgrounds" );
	listbox->resetList();

	for( unsigned int i = 0; i < pLevel->Get_Backgroundlayers() - 1; i++ )
	{
		ListboxTextItem *item = new ListboxTextItem( int_to_string( i + 1 ) );
		listbox->addItem( (ListboxItem *)item );
	}

	PushButton *button_add = (PushButton *)WindowManager::getSingleton().getWindow( "button_add_background_image" );

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
	if( valid_file( filename ) )
	{
		GL_Surface *temp = LoadTexture( filename );
		
		if( !temp )
		{
			printf( "Warning : Couldn't open Background preview : %s\n", filename.c_str() );
		}
		else
		{
			double zoom = Get_scale( temp, 100, 100 );
			temp->w *= zoom;
			temp->h *= zoom;
			background_preview->SetImage( temp, 0, 1 );
		}
	}
	else
	{
		background_preview->SetImage( NULL );
	}
}

bool cLevel_Settings :: Update_BG_image( const EventArgs &event )
{
	string bg_filename = WindowManager::getSingleton().getWindow( "editbox_bg_image_name" )->getText().c_str();
	int bg_type = string_to_int( WindowManager::getSingleton().getWindow( "editbox_bg_image_type" )->getText().c_str() );

	Listbox *listbox = (Listbox *)WindowManager::getSingleton().getWindow( "listbox_backgrounds" );
	ListboxItem *item = listbox->getFirstSelectedItem();

	if( !item ) // clear
	{
		Clear_Layer_field();
		return 1;
	}

	cBackground *background = pLevel->Get_background( string_to_int( item->getText().c_str() ) );
	background->img_file_1 = bg_filename;
	background->type = (BackgroundType)bg_type;

	// image preview
	bg_filename.insert( 0, PIXMAPS_DIR "/" );
	Set_Background_image_Preview( bg_filename );
	// type name
	background_type->SetImage( RenderText( font_small, background->Get_Typename(), black ), 0, 1 );

	return 1;
}

void cLevel_Settings ::Clear_Layer_field( void )
{
	((Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_image_name" ))->setText( "" );
	((Editbox *)WindowManager::getSingleton().getWindow( "editbox_bg_image_type" ))->setText( "" );
	background_type->SetImage( NULL, 0, 1 );
	Set_Background_image_Preview( "" );
}