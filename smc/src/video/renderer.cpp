/***************************************************************************
           renderer.cpp  -  Render Queueing
                             -------------------
    copyright            :	(C) 2006 - 2007 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../video/renderer.h"
#include "../core/globals.h"
#include "../core/game_core.h"
#include "../core/camera.h"

/* *** *** *** *** *** *** cRenderRequest *** *** *** *** *** *** *** *** *** *** *** */

cRenderRequest :: cRenderRequest( void )
{
	type = REND_NOTHING;
	globalscale = 1;
	no_camera = 1;

	pos_z = 0;
	blend_sfactor = GL_SRC_ALPHA;
	blend_dfactor = GL_ONE_MINUS_SRC_ALPHA;

	shadow_pos = 0;
	shadow_color = (Uint8)0;

	combine_type = 0;
	combine_col[0] = 0;
	combine_col[1] = 0;
	combine_col[2] = 0;

	render_count = 1;
}

cRenderRequest :: ~cRenderRequest( void )
{

}

void cRenderRequest :: Draw( void )
{
	// virtual
}

void cRenderRequest :: Begin_render( void )
{
	glLoadIdentity();

	// global scale
	if( globalscale && ( global_scalex != 1 || global_scaley != 1 ) )
	{
		glScalef( global_scalex, global_scaley, 1 );
	}

	// blend factor
	if( blend_sfactor != GL_SRC_ALPHA || blend_dfactor != GL_ONE_MINUS_SRC_ALPHA )
	{
		glBlendFunc( blend_sfactor, blend_dfactor );
	}
}

void cRenderRequest :: End_render( void )
{
	// clear blend factor
	if( blend_sfactor != GL_SRC_ALPHA || blend_dfactor != GL_ONE_MINUS_SRC_ALPHA )
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	// if debug build check for errors
#ifdef _DEBUG
	GLenum error = glGetError();
	if( error != GL_NO_ERROR )
	{
		printf( "RenderRequest : GL Error found : %s\n", gluErrorString( error ) );
	}
#endif
}

void cRenderRequest :: Begin_color_combine( void )
{
	// Color Combine
	if( combine_type != 0 && ( combine_col[0] != 0 || combine_col[1] != 0 || combine_col[2] != 0 ) )
	{
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
		glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, combine_type );
		glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT );
		glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, combine_col );
		glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE );
	}
}

void cRenderRequest :: End_color_combine( void )
{
	// clear color modifications
	if( combine_type != 0 && ( combine_col[0] != 0 || combine_col[1] != 0 || combine_col[2] != 0 ) )
	{
		float col[3] = { 0.0f, 0.0f, 0.0f };
		glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col );
		glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	}
}

/* *** *** *** *** *** *** cGradientRequest *** *** *** *** *** *** *** *** *** *** *** */

cGradientRequest :: cGradientRequest( void )
: cRenderRequest()
{
	type = REND_GRADIENT;
	rect = GL_rect( 0, 0, 0, 0 );
	dir = DIR_UNDEFINED;
	color_1 = (Uint8)0;
	color_2 = (Uint8)0;
}

cGradientRequest :: ~cGradientRequest( void )
{

}

void cGradientRequest :: Draw( void )
{
	Begin_render();

	if( !no_camera )
	{
		rect.x -= pCamera->x;
		rect.y -= pCamera->y;
	}

	// z position
	if( pos_z != 0 )
	{
		glTranslatef( 0, 0, pos_z );
	}

	if( glIsEnabled( GL_TEXTURE_2D ) )
	{
		glDisable( GL_TEXTURE_2D );
	}

	Begin_color_combine();

	if( dir == DIR_VERTICAL )
	{
		glBegin( GL_POLYGON );
			glColor4ub( color_1.red, color_1.green, color_1.blue, color_1.alpha );
			glVertex2f( rect.x, rect.y );
			glVertex2f( rect.x + rect.w, rect.y );
			glColor4ub( color_2.red, color_2.green, color_2.blue, color_2.alpha );
			glVertex2f( rect.x + rect.w, rect.y + rect.h );
			glVertex2f( rect.x, rect.y + rect.h );
		glEnd();
	}
	else if( dir == DIR_HORIZONTAL )
	{
		glBegin( GL_POLYGON );
			glColor4ub( color_1.red, color_1.green, color_1.blue, color_1.alpha );
			glVertex2f( rect.x , rect.y );
			glVertex2f( rect.x + rect.w, rect.y );
			glColor4ub( color_2.red, color_2.green, color_2.blue, color_2.alpha );
			glVertex2f( rect.x + rect.w, rect.y + rect.h );
			glVertex2f( rect.x, rect.y + rect.h );
		glEnd();
	}

	// clear color
	glColor4ub( 255, 255, 255, 255 );

	// set back position
	if( render_count > 1 && !no_camera )
	{
		rect.x += pCamera->x;
		rect.y += pCamera->y;
	}

	End_color_combine();
	End_render();
}

/* *** *** *** *** *** *** cLineRequest *** *** *** *** *** *** *** *** *** *** *** */

cLineRequest :: cLineRequest( void )
: cRenderRequest()
{
	type = REND_LINE;
	color = (Uint8)0;
	line = GL_line( 0, 0, 0, 0 );
	line_width = 1;
}

cLineRequest :: ~cLineRequest( void )
{

}

void cLineRequest :: Draw( void )
{
	Begin_render();

	if( !no_camera )
	{
		line.x1 -= pCamera->x;
		line.y1 -= pCamera->y;
		line.x2 -= pCamera->x;
		line.y2 -= pCamera->y;
	}

	// z position
	if( pos_z != 0 )
	{
		glTranslatef( 0, 0, pos_z );
	}

	Begin_color_combine();

	// color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4ub( color.red, color.green, color.blue, color.alpha );
	}

	if( glIsEnabled( GL_TEXTURE_2D ) )
	{
		glDisable( GL_TEXTURE_2D );
	}

	if( line_width != 1 )
	{
		glLineWidth( line_width );
	}

	glBegin( GL_LINES );
		glVertex2f( line.x1, line.y1 );
		glVertex2f( line.x2, line.y2 );
	glEnd();

	// clear line width
	if( line_width != 1 )
	{
		glLineWidth( 1 );
	}

	// clear color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4ub( 255, 255, 255, 255 );
	}

	// set back position
	if( render_count > 1 && !no_camera )
	{
		line.x1 += pCamera->x;
		line.y1 += pCamera->y;
		line.x2 += pCamera->x;
		line.y2 += pCamera->y;
	}

	End_color_combine();
	End_render();
}

/* *** *** *** *** *** *** cRectRequest *** *** *** *** *** *** *** *** *** *** *** */

cRectRequest :: cRectRequest( void )
: cRenderRequest()
{
	type = REND_RECT;
	color = (Uint8)0;
	rect = GL_rect( 0, 0, 0, 0 );
	filled = 1;
}

cRectRequest :: ~cRectRequest( void )
{

}

void cRectRequest :: Draw( void )
{
	Begin_render();

	if( !no_camera )
	{
		rect.x -= pCamera->x;
		rect.y -= pCamera->y;
	}

	// z position
	if( pos_z != 0 )
	{
		glTranslatef( 0, 0, pos_z );
	}

	Begin_color_combine();

	// color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4ub( color.red, color.green, color.blue, color.alpha );
	}

	if( glIsEnabled( GL_TEXTURE_2D ) )
	{
		glDisable( GL_TEXTURE_2D );
	}

	if( filled )
	{
		glBegin( GL_POLYGON );
	}
	else
	{
		glBegin( GL_LINE_LOOP );
	}
		glVertex2f( rect.x, rect.y );
		glVertex2f( rect.x + rect.w, rect.y );
		glVertex2f( rect.x + rect.w, rect.y + rect.h );
		glVertex2f( rect.x, rect.y + rect.h );
	glEnd();

	
	// clear color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4ub( 255, 255, 255, 255 );
	}

	// set back position
	if( render_count > 1 && !no_camera )
	{
		rect.x += pCamera->x;
		rect.y += pCamera->y;
	}

	End_color_combine();
	End_render();
}

/* *** *** *** *** *** *** cSurfaceRequest *** *** *** *** *** *** *** *** *** *** *** */

cSurfaceRequest :: cSurfaceRequest( void )
: cRenderRequest()
{
	type = REND_SURFACE;
	texture_id = 0;

	pos_x = 0;
	pos_y = 0;

	w = 0;
	h = 0;

	rotx = 0;
	roty = 0;
	rotz = 0;

	scale_x = 1;
	scale_y = 1;
	scale_z = 1;

	color = (Uint8)255;

	delete_texture = 0;
}

cSurfaceRequest :: ~cSurfaceRequest( void )
{
	if( delete_texture && glIsTexture( texture_id ) )
	{
		glDeleteTextures( 1, &texture_id );
	}
}

void cSurfaceRequest :: Draw( void )
{
	if( !no_camera )
	{
		pos_x -= pCamera->x;
		pos_y -= pCamera->y;
	}

	// draw shadow
	if( shadow_pos )
	{
		// shadow position
		pos_x += shadow_pos;
		pos_y += shadow_pos;
		pos_z -= 0.000001f;

		// save data
		Color temp_color = color;
		float temp_shadow_pos = shadow_pos;
		GLint temp_combine_type = combine_type;
		float temp_combine_col[3];
		temp_combine_col[0] = combine_col[0];
		temp_combine_col[1] = combine_col[1];
		temp_combine_col[2] = combine_col[2];

		shadow_pos = 0;
		// shadow as a white texture
		color = black;
		// keep shadow_color alpha
		color.alpha = shadow_color.alpha;
		// combine color
		combine_type = GL_REPLACE;
		combine_col[0] = (float)shadow_color.red / 260;
		combine_col[1] = (float)shadow_color.green / 260;
		combine_col[2] = (float)shadow_color.blue / 260;

		// draw shadow
		Draw();

		// set back data
		shadow_pos = temp_shadow_pos;
		color = temp_color;
		combine_type = temp_combine_type;
		combine_col[0] = temp_combine_col[0];
		combine_col[1] = temp_combine_col[1];
		combine_col[2] = temp_combine_col[2];
		pos_z += 0.000001f;

		// move back to original position
		pos_x -= shadow_pos;
		pos_y -= shadow_pos;
	}

	Begin_render();

	// position
	if( pos_x != 0 || pos_y != 0 || pos_z != 0 )
	{
		float final_pos_x = pos_x + ( ( w / 2 ) * scale_x );
		float final_pos_y = pos_y + ( ( h / 2 ) * scale_y );

		glTranslatef( final_pos_x, final_pos_y, pos_z );
	}

	// scale
	if( scale_x != 1 || scale_y != 1 || scale_z != 1 )
	{
		glScalef( scale_x, scale_y, scale_z );
	}

	// rotation
	if( rotx != 0 )
	{
		glRotatef( rotx, 1, 0, 0 );
	}
	if( roty != 0 )
	{
		glRotatef( roty, 0, 1, 0 );
	}
	if( rotz != 0 )
	{
		glRotatef( rotz, 0, 0, 1 );
	}

	Begin_color_combine();

	// color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4ub( color.red, color.green, color.blue, color.alpha );
	}

	if( !glIsEnabled( GL_TEXTURE_2D ) )
	{
		glEnable( GL_TEXTURE_2D );
	}

	// bind texture id
	GLint last_bind_texture = 0;
	glGetIntegerv( GL_TEXTURE_BINDING_2D, &last_bind_texture );
	// only bind if not the same texture
	if( (GLuint)last_bind_texture != texture_id )
	{
		glBindTexture( GL_TEXTURE_2D, texture_id );
	}

	// rectangle
	glBegin( GL_QUADS );
		// top left
		glTexCoord2f( 0, 0 );
		glVertex2f( -w / 2, -h / 2);
		// top right
		glTexCoord2f( 1, 0 );
		glVertex2f( w / 2, -h / 2 );
		// bottom right
		glTexCoord2f( 1, 1 );
		glVertex2f( w / 2, h / 2 );
		// bottom left
		glTexCoord2f( 0, 1 );
		glVertex2f( -w / 2, h / 2 );
	glEnd();

	
	// clear color
	if( color.red != 255 || color.green != 255 || color.blue != 255 || color.alpha != 255 )
	{
		glColor4ub( 255, 255, 255, 255 );
	}

	// set back position
	if( render_count > 1 && !no_camera )
	{
		pos_x += pCamera->x;
		pos_y += pCamera->y;
	}

	End_color_combine();
	End_render();
}

/* *** *** *** *** *** *** cRenderQueue *** *** *** *** *** *** *** *** *** *** *** */

cRenderQueue :: cRenderQueue( unsigned int reserve_items )
{
	renderdata.reserve( reserve_items );
}

cRenderQueue :: ~cRenderQueue( void )
{
	Clear();
}

void cRenderQueue :: Add( cRenderRequest *obj )
{
	if( !obj )
	{
		return;
	}

	// if no type
	if( obj->type == REND_NOTHING )
	{
		delete obj;
		return;
	}

	renderdata.push_back( obj );
}

void cRenderQueue :: Render( bool clear /* = 1 */ )
{
	// z position sort
	sort( renderdata.begin(), renderdata.end(), zpos_sort() );

	// draw
	for( RenderList::iterator itr = renderdata.begin(), itr_end = renderdata.end(); itr != itr_end; ++itr )
	{
		// get object pointer
		cRenderRequest *obj = (*itr);

		obj->Draw();
		obj->render_count--;
	}

	// clear
	if( clear )
	{
		Clear( 0 );
	}
}

void cRenderQueue :: Clear( bool force /* = 1 */ )
{
	for( RenderList::iterator itr = renderdata.begin(); itr != renderdata.end(); )
	{
		// get object pointer
		cRenderRequest *obj = (*itr);

		// if forced or finished rendering
		if( force || obj->render_count <= 0 )
		{
			itr = renderdata.erase( itr );
			delete obj;
		}
		// increment
		else
		{
			++itr;
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cRenderQueue *pRenderer = NULL;
cRenderQueue *pRenderer_GUI = NULL;
