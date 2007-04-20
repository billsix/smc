/***************************************************************************
				layer.cpp  -  Overworld Layer class
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
 
#include "../overworld/layer.h"
#include "../video/renderer.h"
#include "../core/game_core.h"
#include "../core/camera.h"
#include "../overworld/overworld.h"


/* *** *** *** *** *** *** *** *** Layer Line Point *** *** *** *** *** *** *** *** *** */

cLayer_Line_Point :: cLayer_Line_Point( SpriteType ntype )
: cSprite()
{
	sprite_array = ARRAY_PASSIVE;
	type = ntype;
	massivetype = MASS_PASSIVE;
	posz = 0.087f;

	if( type == TYPE_OW_LINE_START )
	{
		posz += 0.001f;
		color = orange;
		name = "Line Start Point";
	}
	else
	{
		color = red;
		name = "Line End Point";
	}

	rect.w = 4;
	rect.h = 4;
	col_rect.w = rect.w;
	col_rect.h = rect.h;
	start_rect.w = rect.w;
	start_rect.h = rect.h;

	Update_Position_Rect();

	player_range = 0;
}

cLayer_Line_Point :: ~cLayer_Line_Point( void )
{

}

void cLayer_Line_Point :: Draw( cSurfaceRequest *request /* = NULL */ )
{
	if( !pActive_Overworld->pLayer->draw )
	{
		return;
	}

	// point rect
	pVideo->Draw_Rect( col_rect.x - pCamera->x, col_rect.y - pCamera->y, col_rect.w, col_rect.h, posz, &color );
}

void cLayer_Line_Point :: Editor_Activate( void )
{
	WindowManager &wmgr = WindowManager::getSingleton();

	// origin
	Editbox *editbox = static_cast<Editbox *>(wmgr.createWindow( "TaharezLook/Editbox", "layer_line_origin" ));
	editbox->setTooltipText( "Waypoint origin" );
	Editor_Add( editbox, 100 );

	editbox->setText( int_to_string( static_cast<cLayer_Line *>(line)->origin ) );
	editbox->subscribeEvent( Editbox::EventKeyUp, Event::Subscriber( &cLayer_Line_Point::Editor_Origin_Key, this ) );

	// set position
	Editor_pos_update();
}

bool cLayer_Line_Point :: Editor_Origin_Key( const EventArgs &event )
{
	const WindowEventArgs &windowEventArgs = static_cast<const WindowEventArgs&>( event );
	string str_text = static_cast<Editbox *>( windowEventArgs.window )->getText().c_str();

	static_cast<cLayer_Line *>(line)->origin = string_to_int( str_text );

	return 1;
}

float cLayer_Line_Point :: Get_Line_posx( void )
{
	return posx + ( col_rect.w * 0.5f );
}

float cLayer_Line_Point :: Get_Line_posy( void )
{
	return posy + ( col_rect.h * 0.5f );
}

/* *** *** *** *** *** *** *** *** Layer Line *** *** *** *** *** *** *** *** *** */

cLayer_Line :: cLayer_Line( void )
{
	anim_type = 0;
	origin = 0;

	start = new cLayer_Line_Point( TYPE_OW_LINE_START );
	start->line = (void *)this;
	pActive_Overworld->object_manager->Add( start );


	end = new cLayer_Line_Point( TYPE_OW_LINE_END );
	end->line = (void *)this;
	pActive_Overworld->object_manager->Add( end );
}

cLayer_Line :: ~cLayer_Line( void )
{
	// delete points
	pActive_Overworld->object_manager->Delete( start, 1 );
	pActive_Overworld->object_manager->Delete( end, 1 );
}

void cLayer_Line :: Draw( void )
{
	// create request
	cLineRequest *line_request = new cLineRequest();

	// drawing color
	Color color = darkgreen;

	// if active
	if( pOverworld_Player->current_line >= 0 && pActive_Overworld->pLayer->lines[pOverworld_Player->current_line] == this )
	{
		color = lightblue;
	}

	pVideo->DrawLine( start->Get_Line_posx() - pCamera->x, start->Get_Line_posy() - pCamera->y, end->Get_Line_posx() - pCamera->x, end->Get_Line_posy() - pCamera->y, 0.085f, &color, line_request );
	line_request->line_width = 6;

	// add request
	pRenderer->Add( line_request );
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
	Unload();
}

void cLayer :: Load( string filename )
{
	Unload();

	System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), SCHEMA_DIR "/World/Lines.xsd", "" );
}

void cLayer :: Unload( void )
{
	for( LayerLineList::iterator itr = lines.begin(), itr_end = lines.end(); itr != itr_end; ++itr )
	{
		delete *itr;
	}

	lines.clear();
}

bool cLayer :: Save( string filename )
{
	ofstream file( filename.c_str(), ios::out | ios::trunc );

	if( !file )
	{
		debugdisplay->Set_Text( "Couldn't save world layer " + filename );
		return 0;
	}

	// xml info
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	// begin layer
	file << "<layer>" << std::endl;

	// lines
	for( LayerLineList::iterator itr = lines.begin(), itr_end = lines.end(); itr != itr_end; ++itr )
	{
		cLayer_Line *line = (*itr);

		// begin line
		file << "\t<line>" << std::endl;
			// start
			file << "\t\t<Property name=\"X1\" value=\"" << (int)line->start->Get_Line_posx() << "\" />" << std::endl;
			file << "\t\t<Property name=\"Y1\" value=\"" << (int)line->start->Get_Line_posy() << "\" />" << std::endl;
			// end
			file << "\t\t<Property name=\"X2\" value=\"" << (int)line->end->Get_Line_posx() << "\" />" << std::endl;
			file << "\t\t<Property name=\"Y2\" value=\"" << (int)line->end->Get_Line_posy() << "\" />" << std::endl;
			// origin
			file << "\t\t<Property name=\"origin\" value=\"" << line->origin << "\" />" << std::endl;
		// end line
		file << "\t</line>" << std::endl;
	}

	// end layer
	file << "</layer>" << std::endl;
	file.close();

	return 1;
}

void cLayer :: Draw( void )
{
	if( !draw )
	{
		return;
	}

	for( LayerLineList::iterator itr = lines.begin(), itr_end = lines.end(); itr != itr_end; ++itr )
	{
		// get pointer
		cLayer_Line *layer_line = (*itr);

		// draw line
		layer_line->Draw();
	}
}

cLine_collision cLayer :: Get_diff( float x, float y, ObjectDirection dir /* = DIR_HORIZONTAL */, unsigned int check_size /* = 15 */ )
{
	cLine_collision col;
	col.clear();

	for( unsigned int i = 0; i < lines.size(); i++ )
	{
		// get pointer
		cLayer_Line *layer_line = lines[i];

		// line is not from waypoint
		if( pOverworld_Player->line_waypoint != layer_line->origin )
		{
			continue;
		}

		GL_line line_1, line_2;

		// check into both directions from inside
		for( float csize = 0; csize < check_size; csize++ )
		{
			line_1.x1 = x;
			line_1.y1 = y;
			line_1.x2 = x;
			line_1.y2 = y;
			line_2 = line_1;

			if( dir == DIR_HORIZONTAL )
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
			if( pOverworld_manager->debugmode && draw )
			{
				// create request
				cLineRequest *line_request = new cLineRequest();
				pVideo->DrawLine( line_1.x1 - pCamera->x, line_1.y1 - pCamera->y, line_1.x2 - pCamera->x, line_1.y2 - pCamera->y, 0.087f, &white );
				line_request->line_width = 2;
				// add request
				pRenderer->Add( line_request );

				// create request
				line_request = new cLineRequest();
				pVideo->DrawLine( line_2.x1 - pCamera->x, line_2.y1 - pCamera->y, line_2.x2 - pCamera->x, line_2.y2 - pCamera->y, 0.087f, &black );
				line_request->line_width = 2;
				// add request
				pRenderer->Add( line_request );
			}

			// create layer line
			GL_line check_line = GL_line( layer_line->start->Get_Line_posx(), layer_line->start->Get_Line_posy(), layer_line->end->Get_Line_posx(), layer_line->end->Get_Line_posy() );

			// check line 1
			if( Col_line( &line_1, &check_line ) )
			{
				// if this line is closer to maryo
				if( csize > col.difference )
				{
					col.line_number = i;
					col.difference = csize;
				}
				break;
			}

			// check line 2
			if( Col_line( &line_2, &check_line ) )
			{
				// if this line is closer to maryo
				if( csize > col.difference )
				{
					col.line_number = i;
					col.difference = -csize;
				}
				break;
			}
		}
	}

	return col;
}

int cLayer :: Get_Line_num( cLayer_Line *obj ) 
{
	// invalid
	if( !obj )
	{
		return -1;
	}

	for( unsigned int i = 0; i < lines.size(); i++ )
	{
		// if the same
		if( lines[i] == obj )
		{
			return i;
		}
	}

	// not found
	return -1;
}

// XML element start
void cLayer :: elementStart( const String &element, const XMLAttributes &attributes )
{
	// Property of an Element
    if( element == "Property" )
    {
		xml_attributes.add( attributes.getValueAsString( "name" ), attributes.getValueAsString( "value" ) );
    }
}

// XML element end
void cLayer :: elementEnd( const String &element )
{
	if( element != "Property" )
	{
		if( element == "line" )
		{
			handle_line( xml_attributes );
		}
		else if( element == "layer" )
		{
			// ignore
		}
		else if( element.length() )
		{
			printf( "Warning : Overworld Layer Unknown element : %s\n", element.c_str() );
		}

		// clear
		xml_attributes = XMLAttributes();
	}
}

void cLayer :: handle_line( const XMLAttributes &attributes )
{
	// create
	cLayer_Line *item = new cLayer_Line();
	
	// Start
	item->start->Set_Pos( (float)attributes.getValueAsInteger( "X1" ) - 2, (float)attributes.getValueAsInteger( "Y1" ) - 2, 1 );
	// End
	item->end->Set_Pos( (float)attributes.getValueAsInteger( "X2" ) - 2, (float)attributes.getValueAsInteger( "Y2" ) - 2, 1 );
	// origin
	item->origin = attributes.getValueAsInteger( "origin" );

	// add
	lines.push_back( item );
}
