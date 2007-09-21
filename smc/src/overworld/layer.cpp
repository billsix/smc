/***************************************************************************
				layer.cpp  -  Overworld Layer class
                             -------------------
    copyright            : (C) 2003 - 2007 by Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
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
	pActive_Overworld->sprite_manager->Add( start );


	end = new cLayer_Line_Point( TYPE_OW_LINE_END );
	end->line = (void *)this;
	pActive_Overworld->sprite_manager->Add( end );
}

cLayer_Line :: ~cLayer_Line( void )
{
	// delete points
	pActive_Overworld->sprite_manager->Delete( start, 1 );
	pActive_Overworld->sprite_manager->Delete( end, 1 );
}

void cLayer_Line :: Draw( void )
{
	// create request
	cLineRequest *line_request = new cLineRequest();

	// drawing color
	Color color = darkgreen;

	// if active
	if( pOverworld_Player->current_line >= 0 && pActive_Overworld->pLayer->objects[pOverworld_Player->current_line] == this )
	{
		color = lightblue;
	}

	pVideo->DrawLine( start->Get_Line_posx() - pCamera->x, start->Get_Line_posy() - pCamera->y, end->Get_Line_posx() - pCamera->x, end->Get_Line_posy() - pCamera->y, 0.085f, &color, line_request );
	line_request->line_width = 6;

	// add request
	pRenderer->Add( line_request );
}

GL_line cLayer_Line :: Get_Line( void )
{
	return GL_line( start->posx + ( start->col_rect.w * 0.5f ), start->posy + ( start->col_rect.h * 0.5f ), end->posx + ( end->col_rect.w * 0.5f ), end->posy + ( end->col_rect.h * 0.5f ) );
}

cWaypoint *cLayer_Line :: Get_End_Waypoint( void )
{
	int wp_num = pActive_Overworld->Get_Waypoint_Collision( &end->col_rect );

	// no waypoint collision
	if( wp_num < 0 )
	{
		cLayer_Line *line_col = pActive_Overworld->pLayer->Get_Line_Collision_Start( &end->col_rect );

		// line collision
		if( line_col )
		{
			// follow line
			return line_col->Get_End_Waypoint();
		}
	}

	// return Waypoint
	return pActive_Overworld->Get_Waypoint( wp_num );
}

/* *** *** *** *** *** *** *** *** Line Collision *** *** *** *** *** *** *** *** *** */

cLine_collision :: cLine_collision( void )
{
	line = NULL;
	line_number = -2;
	difference = 0;
}

/* *** *** *** *** *** *** *** *** Near Line Collision *** *** *** *** *** *** *** *** *** */

cNearLine_collision :: cNearLine_collision( void )
{
	line_number = -2;
	start = 0;
}

/* *** *** *** *** *** *** *** *** Contact Collision *** *** *** *** *** *** *** *** *** */

void cContact_collision :: clear( void )
{
	contact = 0;
	line_hor = cLine_collision();
	line_ver = cLine_collision();
}

int cContact_collision :: Get_best_line( ObjectDirection dir )
{
	// favor vertical
	if( dir == DIR_LEFT || dir == DIR_RIGHT )
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
	// favor horizontal
	else if( dir == DIR_UP || dir == DIR_DOWN )
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
	Delete_All();
}

void cLayer :: Load( string filename )
{
	Delete_All();

	System::getSingleton().getXMLParser()->parseXMLFile( *this, filename.c_str(), DATA_DIR "/" GAME_SCHEMA_DIR "/World/Lines.xsd", "" );
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
	for( LayerLineList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
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

	for( LayerLineList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		// draw line
		(*itr)->Draw();
	}
}

cLayer_Line *cLayer :: Get_Line_Collision_Start( GL_rect *line_rect )
{
	for( LayerLineList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		// get pointer
		cLayer_Line *layer_line = (*itr);

		// check line 1
		if( Col_Box( line_rect, &layer_line->start->col_rect ) )
		{
			return layer_line;
		}
	}

	return NULL;
}

cLine_collision cLayer :: Get_Line_Collision_Direction( float x, float y, ObjectDirection dir, float dir_size /* = 10 */, float check_size /* = 10 */ )
{
	if( dir == DIR_UP )
	{
		y -= dir_size;
	}
	else if( dir == DIR_DOWN )
	{
		y += dir_size;
	}
	else if( dir == DIR_RIGHT )
	{
		x += dir_size;
	}
	else if( dir == DIR_LEFT )
	{
		x -= dir_size;
	}

	if( dir == DIR_LEFT || dir == DIR_RIGHT )
	{
		return pActive_Overworld->pLayer->Get_nearest( x, y, DIR_VERTICAL, check_size );
	}
	else if( dir == DIR_UP || dir == DIR_DOWN )
	{
		return pActive_Overworld->pLayer->Get_nearest( x, y, DIR_HORIZONTAL, check_size );
	}

	// invalid direction
	return cLine_collision();
}

cLine_collision cLayer :: Get_nearest( float x, float y, ObjectDirection dir /* = DIR_HORIZONTAL */, unsigned int check_size /* = 15 */, int only_origin_id /* = -1 */ )
{
	for( LayerLineList::iterator itr = objects.begin(), itr_end = objects.end(); itr != itr_end; ++itr )
	{
		// get pointer
		cLayer_Line *layer_line = (*itr);

		// line is not from waypoint
		if( only_origin_id >= 0 && only_origin_id != layer_line->origin )
		{
			continue;
		}

		cLine_collision col = Get_nearest_line( layer_line, x, y, dir, check_size );

		// found
		if( col.line )
		{
			return col;
		}
	}

	// none found
	return cLine_collision();
}

cLine_collision cLayer :: Get_nearest_line( cLayer_Line *map_layer_line, float x, float y, ObjectDirection dir /* = DIR_HORIZONTAL */, unsigned int check_size /* = 15  */ )
{
	GL_line line_1, line_2;

	// create map line
	GL_line map_line = map_layer_line->Get_Line();

	// check into both directions from inside
	for( float csize = 0; csize < check_size; csize++ )
	{
		line_1.x1 = x;
		line_1.y1 = y;
		line_1.x2 = x;
		line_1.y2 = y;
		line_2 = line_1;

		// set line size
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
			pVideo->DrawLine( line_1.x1 - pCamera->x, line_1.y1 - pCamera->y, line_1.x2 - pCamera->x, line_1.y2 - pCamera->y, 0.09f, &white );
			line_request->line_width = 2;
			// add request
			pRenderer->Add( line_request );

			// create request
			line_request = new cLineRequest();
			pVideo->DrawLine( line_2.x1 - pCamera->x, line_2.y1 - pCamera->y, line_2.x2 - pCamera->x, line_2.y2 - pCamera->y, 0.09f, &black );
			line_request->line_width = 2;
			// add request
			pRenderer->Add( line_request );
		}


		// check direction line 1
		if( Col_line( &line_1, &map_line, 1 ) )
		{
			cLine_collision col = cLine_collision();

			col.line = map_layer_line;
			col.line_number = Get_Array_num( map_layer_line );
			col.difference = csize;

			// found
			return col;
		}

		// check direction line 2
		if( Col_line( &line_2, &map_line, 1 ) )
		{
			cLine_collision col = cLine_collision();

			col.line = map_layer_line;
			col.line_number = Get_Array_num( map_layer_line );
			col.difference = -csize;

			// found
			return col;
		}
	}

	// not found
	return cLine_collision();
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
	Add( item );
}
