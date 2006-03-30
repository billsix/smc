/***************************************************************************
    layer.cpp  -  class for the Overworld Layer
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

/* *** *** *** *** *** *** *** *** Layer Line *** *** *** *** *** *** *** *** *** */

cLayer_Line :: cLayer_Line( void ) : GL_line()
{
	collision_test = 0;
	type = 0;
}

cLayer_Line :: ~cLayer_Line( void )
{

}

void cLayer_Line :: Draw( void )
{
	glLineWidth( 3 );
	DrawLine( x1 - cameraposx, y1 - cameraposy, x2 - cameraposx, y2 - cameraposy, &white );
	glLineWidth( 1 );
}

/* *** *** *** *** *** *** *** *** Line Collision *** *** *** *** *** *** *** *** *** */

void cLine_collision :: clear( void )
{
	line_number = -2;
	difference = 0;
}

/* *** *** *** *** *** *** *** *** Near Line Collision *** *** *** *** *** *** *** *** *** */

void cNearLine_collision :: clear( void )
{
	line_number = -2;
	start = 0;
}

/* *** *** *** *** *** *** *** *** Contact Collision *** *** *** *** *** *** *** *** *** */

void cContact_collision :: clear( void )
{
	contact = 0;
	line_hor.clear();
	line_ver.clear();
}

int cContact_collision :: Get_best_line( ObjectDirection dir )
{
	if( dir == DIR_LEFT || dir == DIR_RIGHT ) // favor vertical
	{
		if( line_ver.line_number >= 0 && line_ver.difference < line_hor.difference )
		{
			return line_ver.line_number;
		}
		else if( line_hor.line_number >= 0 )
		{
			return line_hor.line_number;
		}
		else
		{
			return line_ver.line_number;
		}
	}
	else if( dir == DIR_UP || dir == DIR_DOWN ) // favor horizontal
	{
		if( line_hor.line_number >= 0 && line_hor.difference < line_ver.difference )
		{
			return line_hor.line_number;
		}
		else if( line_ver.line_number >= 0 )
		{
			return line_ver.line_number;
		}
		else
		{
			return line_hor.line_number;
		}
	}

	return -2;
}

/* *** *** *** *** *** *** *** *** Layer *** *** *** *** *** *** *** *** *** */

cLayer :: cLayer( void )
{
	draw = 0;
}

cLayer :: ~cLayer( void )
{
	for( unsigned int i = 0; i < lines.size() ; i++ )
	{
		delete lines[i];
	}

	lines.clear();
}

void cLayer :: Load( string filename )
{
	System::getSingleton().getXMLParser()->parseXMLFile( *this, filename, "data/schemes/WorldLineScheme.xsd", "" );
}

// XML element start
void cLayer :: elementStart( const String &element, const XMLAttributes &attributes )
{
    if( element == "Line" )
    {
        handle_line( attributes );
    }
}

// XML element end
void cLayer :: elementEnd( const String &element )
{
	
}

void cLayer :: Draw( void )
{
	if( !draw )
	{
		return;
	}

	glLineWidth( 2 );

	for( unsigned int i = 0; i < lines.size() ;i++ )
	{
		Color color = (Uint8)0;

		if( lines[i]->collision_test )
		{
			color = darkgreen;
		}
		else
		{
			color = lightblue;
		}

		DrawLine( lines[i]->x1 - cameraposx, lines[i]->y1 - cameraposy, lines[i]->x2 - cameraposx, lines[i]->y2 - cameraposy, &color );
		DrawRect( lines[i]->x1 - 2 - cameraposx, lines[i]->y1 - 2 - cameraposy, 4, 4, &orange ); // start point
		DrawRect( lines[i]->x2 - 2 - cameraposx, lines[i]->y2 - 2 - cameraposy, 4, 4, &red ); // end point
	}

	glLineWidth( 1 );
}

cLine_collision cLayer :: Get_diff( double x, double y, ObjectDirection directions /* = DIR_HORIZONTAL */, unsigned int check_size /* = 10 */ )
{
	cLine_collision col;
	col.clear();

	for( unsigned int i = 0; i < lines.size(); i++ )
	{
		if( lines[i]->collision_test )
		{
			GL_line line_1, line_2;

			// check into both directions from inside
			for( double csize = 0; csize < check_size; csize++ )
			{
				line_1.x1 = x;
				line_1.y1 = y;
				line_1.x2 = x;
				line_1.y2 = y;
				line_2 = line_1;

				if( directions == DIR_HORIZONTAL )
				{
					line_1.x1 += csize;
					line_2.x2 -= csize;
				}
				else // vertical
				{
					line_1.y1 += csize;
					line_2.y2 -= csize;
				}

				// debug drawing
				if( pOverWorld->debugmode && draw )
				{
					glLineWidth( 2 );
					DrawLine( line_1.x1 - cameraposx, line_1.y1 - cameraposy, line_1.x2 - cameraposx, line_1.y2 - cameraposy, &white );
					DrawLine( line_2.x1 - cameraposx, line_2.y1 - cameraposy, line_2.x2 - cameraposx, line_2.y2 - cameraposy, &black );
					glLineWidth( 1 );
				}

				if( Col_line( &line_1, (GL_line *)lines[i] ) )
				{
					// if this line is closer to maryo
					if( csize > col.difference )
					{
						col.line_number = i;
						col.difference = csize;

						// debug drawing
						if( pOverWorld->debugmode && draw )
						{
							lines[i]->Draw();
							SDL_GL_SwapBuffers();
						}
					}
					break;
				}

				if( Col_line( &line_2, (GL_line *)lines[i] ) )
				{
					// if this line is closer to maryo
					if( csize > col.difference )
					{
						col.line_number = i;
						col.difference = -csize;

						// debug drawing
						if( pOverWorld->debugmode && draw )
						{
							lines[i]->Draw();
							SDL_GL_SwapBuffers();
						}
					}
					break;
				}
			}
		}
	}

	return col;
}

void cLayer :: handle_line( const XMLAttributes& attributes )
{
	int start_x = attributes.getValueAsInteger( "Start_X" );
	int start_y = attributes.getValueAsInteger( "Start_Y" );
	int end_x = attributes.getValueAsInteger( "End_X" );
	int end_y = attributes.getValueAsInteger( "End_Y" );

	cLayer_Line *item = new cLayer_Line();

	item->x1 = start_x;
	item->y1 = GAME_RES_H - start_y;
	item->x2 = end_x;
	item->y2 = GAME_RES_H - end_y;
	item->collision_test = 1;

	lines.push_back( item );
}