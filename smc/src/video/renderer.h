/***************************************************************************
			renderer.h  -  header for the corresponding cpp file
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

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "../core/globals.h"
#include "../video/video.h"
#include "../core/math/rect.h"

/* *** *** *** *** *** *** *** Render Types *** *** *** *** *** *** *** *** *** *** */

enum RenderType
{
	REND_NOTHING = 0,
	REND_RECT = 1,
	REND_GRADIENT = 2,
	REND_SURFACE = 3,
	REND_TEXT = 4, // todo
	REND_LINE = 5
};

/* *** *** *** *** *** *** cRenderRequest *** *** *** *** *** *** *** *** *** *** *** */

class cRenderRequest
{
public:
	cRenderRequest( void );
	virtual ~cRenderRequest( void );

	// Draw the Render Request
	virtual void Draw( void );

	// start Rendering
	void Begin_render( void );
	// end Rendering
	void End_render( void );

	// add color combine
	void Begin_color_combine( void );
	// remove color combine
	void End_color_combine( void );

	// render type
	RenderType type;

	// globalscale
	bool globalscale;
	// if not set camera position is subtracted
	bool no_camera;

	// Z position
	float pos_z;
	// blending
    GLenum blend_sfactor;
    GLenum blend_dfactor;
	// shadow position
	float shadow_pos;
	// shadow color
	Color shadow_color;

	// combine type
	GLint combine_type;
	// combine color
	float combine_col[3];

	// times to render until deletion
	int render_count;
};

typedef vector<cRenderRequest *> RenderList;

/* *** *** *** *** *** *** cGradientRequest *** *** *** *** *** *** *** *** *** *** *** */

class cGradientRequest : public cRenderRequest
{
public:
	cGradientRequest( void );
	virtual ~cGradientRequest( void );

	// draw
	virtual void Draw( void );

	// rect
	GL_rect rect;
	// direction
	ObjectDirection dir;
	// colors
	Color color_1;
	Color color_2;
};

/* *** *** *** *** *** *** cLineRequest *** *** *** *** *** *** *** *** *** *** *** */

class cLineRequest : public cRenderRequest
{
public:
	cLineRequest( void );
	virtual ~cLineRequest( void );

	// draw
	virtual void Draw( void );

	// color
	Color color;
	// line
	GL_line line;
	// line width
	float line_width;
};

/* *** *** *** *** *** *** cRectRequest *** *** *** *** *** *** *** *** *** *** *** */

class cRectRequest : public cRenderRequest
{
public:
	cRectRequest( void );
	virtual ~cRectRequest( void );

	// draw
	virtual void Draw( void );
	// color
	Color color;
	// rect
	GL_rect rect;
	// rect is filled
	bool filled;
};

/* *** *** *** *** *** *** cSurfaceRequest *** *** *** *** *** *** *** *** *** *** *** */

class cSurfaceRequest : public cRenderRequest
{
public:
	cSurfaceRequest( void );
	virtual ~cSurfaceRequest( void );

	// Draw
	virtual void Draw( void );

	// texture id
	GLuint texture_id;
	// position
	float pos_x, pos_y;
	// rotation
	float rotx, roty, rotz;
	// size
	float w, h;
	// scale
	float scale_x;
	float scale_y;
	float scale_z;

	// color
	Color color;

	// delete texture after request finished
	bool delete_texture;
};

/* *** *** *** *** *** *** cRenderQueue *** *** *** *** *** *** *** *** *** *** *** */

class cRenderQueue
{
public:
	cRenderQueue( unsigned int reserve_items );
	~cRenderQueue( void );

	/* Add a Render Request
	 * 
	*/
	void Add( cRenderRequest *obj );

	/*
	 * if clear is not set doesn't clear the data
	*/
	void Render( bool clear = 1 );

	/* clear the render data
	 * if force is given all objects will be removed
	*/
	void Clear( bool force = 1 );

	// renderdata array
	RenderList renderdata;

	// Z position sort
	struct zpos_sort
	{
		bool operator()( const cRenderRequest *a, const cRenderRequest *b ) const
        {
            return a->pos_z < b->pos_z;
        }
	};
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Renderer class
extern cRenderQueue *pRenderer;
// Renderer after GUI was drawn
extern cRenderQueue *pRenderer_GUI;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
