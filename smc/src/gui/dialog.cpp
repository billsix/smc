/***************************************************************************
           dialog.cpp  -  Dialog class
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

/* *** *** *** *** *** *** cDialog *** *** *** *** *** *** *** *** *** *** *** */

cDialog :: cDialog( double nposx, double nposy, string nidentifier, string ntext, SpriteType dialogtype, unsigned int nmax_length, unsigned int nmin_width, bool no_cam )
: cSprite( NULL, nposx, nposy, 0, NULL, no_cam )
{
	array = ARRAY_DIALOG;
	type = dialogtype;

	if( is_valid_number( ntext ) ) 
	{
		text_number = string_to_int( ntext );
	}
	else
	{
		text_number = 0;
	}

	identifier.reserve( nidentifier.length() );
	identifier = nidentifier;
	text = ntext;

	boarder_in = 2;
	boarder_out = 2;
	text_area = 1;
	changed = 0;

	min_width = nmin_width;
	
	if( type != TYPE_DIALOG_ONLY_NUMBERS )
	{
		if( nmax_length < ntext.length() ) 
		{
			max_length = (unsigned int)ntext.length();
		}
		else
		{
			max_length = nmax_length;
		}

		text.reserve( max_length );
	}
	else
	{
		if( (int)nmax_length < text_number ) 
		{
			max_length = text_number;
		}
		else
		{
			max_length = nmax_length;
		}

		text.reserve( 20 );
	}
	
	SetColors( darkblue, blue, white, black );
	Update_Boarders();
	Update_Text();
}

cDialog :: ~cDialog( void )
{
	//
}

void cDialog :: Draw( void )
{
	if( !visible ) 
	{
		return;
	}
	
	double camerax = 0, cameray = 0;

	if( !no_camera )
	{
		camerax = cameraposx;
		cameray = cameraposy;
	}

	DrawRect( (double)rect_boarder_out.x - camerax, (double)rect_boarder_out.y - cameray, rect_boarder_out.w, rect_boarder_out.h, &boarder_out_color );
	DrawRect( (double)rect_boarder_in.x - camerax, (double)rect_boarder_in.y - cameray, rect_boarder_in.w, rect_boarder_in.h, &boarder_in_color);
	DrawRect( (double)rect_text_area.x - camerax, (double)rect_text_area.y - cameray, rect_text_area.w, rect_text_area.h, &text_area_color );

	cSprite::Draw();
}

bool cDialog :: gotChanged( void )
{
	return changed;
}

int cDialog :: Get_Value_int( bool reset_changed )
{
	if( reset_changed )
	{
		changed = 0;
	}
	
	return text_number;
}

string cDialog :: Get_Value_string( bool reset_changed )
{
	if( reset_changed ) 
	{
		changed = 0;
	}
	
	return text;
}

void cDialog :: Get_Focus( void )
{
	bool focus = 1;
	string text_old = text;

	unsigned int minwidth_old = min_width;

	pDialogManager->Draw();
	
	Color color = Get_Color( 0, 0, 0, 32 );
	DrawRect( NULL, &color );

	SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, 50 );

	while( focus )
	{
		Update();

		if( col_rect.w < min_width ) 
		{
			col_rect.w = min_width;
		}
		else if( col_rect.w > min_width ) 
		{
			min_width = (unsigned int)col_rect.w;
		}	

		SDL_GL_SwapBuffers();

		while( SDL_PollEvent( &event ) )
		{
			keys = SDL_GetKeyState( NULL );

			if( event.type == SDL_KEYDOWN )
			{
				if( event.key.keysym.sym == SDLK_ESCAPE )
				{
					text = text_old;

					if( type == TYPE_DIALOG_ONLY_NUMBERS )
					{
						text_number = string_to_int( text );
					}

					focus = 0;
				}
				else if( event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER )
				{
					focus = 0;
				}
				else if( event.key.keysym.sym == SDLK_BACKSPACE )
				{
					if( text.length() && type != TYPE_DIALOG_ONLY_NUMBERS )
					{
						text.erase( text.length() - 1, 1 );
					}
					else
					{
						string str = int_to_string( text_number );
						str.erase( str.length() - 1, 1 );

						if( str.length() > 0 )
						{
							text_number = string_to_int( str );
						}
						else
						{
							text_number = 0;
						}
					}
					
					Update_Text();
				}
				else if( event.key.keysym.sym != SDLK_ESCAPE )
				{
					if( type != TYPE_DIALOG_ONLY_NUMBERS ) 
					{
						if( event.key.keysym.unicode && text.length() < max_length )
						{
							if( type == TYPE_DIALOG_ONLY_LETTERS )
							{
								string str;
								str.insert( (string::size_type)0, (string::size_type)1, (char)event.key.keysym.unicode );

								if( !is_valid_number( str ) )
								{
									text.insert( text.length(), str );
								}
							}
							else
							{
								text.insert( text.length(), 1, (char)event.key.keysym.unicode );
							}

							Update_Text();
						}
					}
					else
					{
						if( event.key.keysym.sym == SDLK_UP )
						{
							text_number += 1;
						}
						else if( event.key.keysym.sym == SDLK_DOWN )
						{
							text_number -= 1;
						}
						else if( event.key.keysym.sym == SDLK_LEFT )
						{
							text_number -= 10;
						}
						else if( event.key.keysym.sym == SDLK_RIGHT )
						{
							text_number += 10;
						}
						else if( event.key.keysym.sym == SDLK_KP_PLUS )
						{
							text_number += 1;
						}
						else if( event.key.keysym.sym == SDLK_KP_MINUS )
						{
							text_number -= 1;
						}
						else if( event.key.keysym.sym == SDLK_PAGEUP )
						{
							text_number += 10;
						}
						else if( event.key.keysym.sym == SDLK_PAGEDOWN )
						{
							text_number -= 10;
						}
						else
						{
	 						string str;
							str.insert( (string::size_type)0, (string::size_type)1, (char)event.key.keysym.unicode );

							if( is_valid_number( str ) )
							{
								if( text_number != 0 )
								{
									str.insert( 0, int_to_string( text_number ) );
								}
								
								text_number = string_to_int( str );
							}
						}
		
						if( text_number > (int)max_length )
						{
							text_number = max_length;
						}
						else if( text_number < 0 )
						{
							text_number = 0;
						}

						Update_Text();
					}
				}
			}
		}
		
		Framerate.Update();
	}

	SDL_EnableKeyRepeat( 0, 0 );

	if( text.compare( text_old ) != 0 ) 
	{
		changed = 1;
	}

	min_width = minwidth_old;
	Update_Text();
	Framerate.Reset();
}

void cDialog :: Set_Text( string text_str, bool set_changed /* = 0  */ )
{
	if( type == TYPE_DIALOG_ONLY_NUMBERS && !is_valid_number( text_str ) )
	{
		return;
	}

	text = text_str;
	if( type == TYPE_DIALOG_ONLY_NUMBERS )
	{
		text_number = string_to_int( text );
	}

	Update_Text();

	if( set_changed )
	{
		changed = 1;
	}
}

void cDialog :: SetPos( double x, double y )
{
	cSprite::SetPos( x, y, 1 );
	Update_Boarders();
}

void cDialog :: Set_Boardersize_out( Uint8 size )
{
	boarder_out = size;

	Update_Boarders();
}

void cDialog :: Set_Boardersize_in( Uint8 size )
{
	boarder_in = size;

	Update_Boarders();
}

void cDialog :: Additional_Textareasize( Uint8 size )
{
	text_area = size;

	Update_Boarders();
}

void cDialog :: SetColors( Color outer, Color inner, Color text_area, Color text )
{
	boarder_out_color = outer;
	boarder_in_color = inner;
	text_area_color = text_area;
	text_color = text;

	Update_Text();
}

void cDialog :: Update_Boarders( void )
{
	unsigned int fwidth = (unsigned int)col_rect.w;
	unsigned int fheight = (unsigned int)col_rect.h;

	if( fheight < 26 )
	{
		fheight = 26;
	}

	if( fwidth < min_width )
	{
		fwidth = min_width;
	}


	rect_text_area.x = (Sint16)posx - text_area;
	rect_text_area.y = (Sint16)posy - text_area;
	rect_text_area.w = fwidth + text_area;
	rect_text_area.h = fheight + text_area;

	rect_boarder_in.x = rect_text_area.x - boarder_in;
	rect_boarder_in.y = rect_text_area.y - boarder_in;
	rect_boarder_in.w = rect_text_area.w + ( boarder_in * 2 );
	rect_boarder_in.h = rect_text_area.h + ( boarder_in * 2 );

	rect_boarder_out.x = rect_boarder_in.x - boarder_out;
	rect_boarder_out.y = rect_boarder_in.y - boarder_out;
	rect_boarder_out.w = rect_boarder_in.w + ( boarder_out * 2 );
	rect_boarder_out.h = rect_boarder_in.h + ( boarder_out * 2 );
}

void cDialog :: Update_Text( void )
{
	if( type == TYPE_DIALOG_ONLY_NUMBERS ) 
	{
		text = int_to_string( text_number );
	}

	SetImage( RenderText( font_normal, text, text_color ), 1, 1 );
	Update_Boarders();
}

SDL_Rect cDialog :: Get_Rect( void )
{
	SDL_Rect crect;
	
	unsigned int fwidth = (unsigned int)col_rect.w;
	unsigned int fheight = (unsigned int)col_rect.h;

	if( fheight < 26 )
	{
		fheight = 26;
	}

	if( fwidth < min_width )
	{
		fwidth = min_width;
	}

	crect.x = (int)posx - ( text_area + boarder_in + boarder_out );
	crect.y = (int)posy - ( text_area + boarder_in + boarder_out );
	crect.h = fheight + ( ( text_area + boarder_in + boarder_out ) * 2 );
	crect.w = fwidth + ( ( text_area + boarder_in + boarder_out ) * 2 );

	return crect;
}

/* *** *** *** *** *** *** *** cDialogManager *** *** *** *** *** *** *** *** *** *** */

cDialogManager :: cDialogManager( void )
{
	Objects.clear();
}

cDialogManager :: ~cDialogManager( void )
{
	DeleteAll();
}

void cDialogManager :: Add( double posx, double posy, string identifier, string ntext, SpriteType dialogtype /* = TYPE_DIALOG_ALL */, int max_length /* = 20 */, unsigned int min_width /* = 70 */, bool no_camera /* = 0 */ )
{
	if( Get( ntext ) ) // already existing
	{
		printf( "Warning : Dialog (%s) already exists\n", ntext.c_str() );
		return;
	}

	cDialog *temp = new cDialog( posx, posy, identifier, ntext, dialogtype, max_length, min_width, no_camera );
	
	Objects.push_back( temp );
}

void cDialogManager :: Delete( unsigned int num )
{
	if( num > Get_Size() )
	{
		return;
	}

	delete Objects[num];
	Objects.erase( Objects.begin() + num );
}

bool cDialogManager :: Delete( string name )
{
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( !Objects[i] )
		{
			continue;
		}
		
		if( Objects[i]->identifier.compare( name ) == 0 )
		{
			Delete( i );
			return 1;
		}
	}

	return 0;
}

int cDialogManager :: Get_num( string name )
{
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( !Objects[i] )
		{
			continue;
		}
		
		if( Objects[i]->identifier.compare( name ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

cDialog *cDialogManager :: Get( string name )
{
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( !Objects[i] )
		{
			continue;
		}
		
		if( Objects[i]->identifier.compare( name ) == 0 )
		{
			return Objects[i];
		}
	}

	return NULL;
}

void cDialogManager :: Draw( void )
{
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( !Objects[i] )
		{
			continue;
		}

		Objects[i]->Draw();
	
		if( !Objects[i]->visible ) // if Dialog has ended delete it
		{
			Delete( i );
		}
	}
}

void cDialogManager :: DeleteAll( void )
{
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( Objects[i] ) 
		{
			delete Objects[i];
		}
	}

	Objects.clear();
}

int cDialogManager :: Get_Collision( int x, int y )
{
	for( unsigned int i = 0; i < Get_Size(); i++ )
	{
		if( !Objects[i] )
		{
			continue;
		}
		
		if( Objects[i]->visible ) 
		{
			SDL_Rect rect1;
			GL_rect rect2;

			rect1 = Objects[i]->Get_Rect();

			if( Objects[i]->no_camera )
			{
				rect1.x += (Sint16)cameraposx;
				rect1.y += (Sint16)cameraposy; 
			}

			// point rect
			rect2.x = x;
			rect2.y = y;
			rect2.w = 1;
			rect2.h = 1;

			if( Col_Box( &rect1, &rect2 ) ) 
			{
				return i;
			}
		}
	}

	return -1;
}

unsigned int cDialogManager :: Get_Size( void )
{
	return (unsigned int)Objects.size();
}
