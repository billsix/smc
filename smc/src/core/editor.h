/***************************************************************************
			editor.h  -  header for the corresponding cpp file
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

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "../core/globals.h"
#include "../objects/sprite.h"
#include "../gui/hud.h"
#include "../video/img_settings.h"

/* *** *** *** *** *** *** *** *** cEditor_Item_Object *** *** *** *** *** *** *** *** *** */

class cEditor_Item_Object : public ListboxItem
{
public:
	cEditor_Item_Object( string text );
	virtual ~cEditor_Item_Object( void );

	// Initialize
	void Init( void );

	/*!
	\brief
		Return the rendered pixel size of this list box item.
	\return
		Size object describing the size of the list box item in pixels.
	*/
	virtual	Size getPixelSize( void ) const;
	// draw
	void draw( const Vector3 &position, float alpha, const Rect &clipper ) const;
	void draw( RenderCache &cache,const Rect &targetRect, float zBase, float alpha, const Rect *clipper) const;
	// draw image
	void Draw_Image( void );

	// Text
	ListboxTextItem *list_text;
	// sprite
	cSprite *sprite_obj;
	// preview image scale
	float preview_scale;
};

/* *** *** *** *** *** *** *** *** cEditor_Menu_Object *** *** *** *** *** *** *** *** *** */

class cEditor_Menu_Object : public ListboxTextItem
{
public:
	cEditor_Menu_Object( string text );
	virtual ~cEditor_Menu_Object( void );

	// Initialize
	void Init( void );

	// Name
	string name;
	// tags or function name if function
	string tags;

	// if type is a function
	bool bfunction;

	// if Object is an Header
	bool header;
};

/* *** *** *** *** *** *** *** cEditor *** *** *** *** *** *** *** *** *** *** */

class cEditor : public XMLHandler
{
public:
	cEditor( void );
	virtual ~cEditor( void );

	// Initialize Editor
	virtual void Init( void );
	// Unload Editor
	virtual void Unload( void );

	// Toggle
	void Toggle( void );
	// Enable
	virtual void Enable( void );
	/* Disable
	 * native_mode : if unset the current game mode isn't altered
 	*/
	virtual void Disable( bool native_mode = 1 );

	// Update Editor
	virtual void Update( void );
	// Draw the Editor Menus
	virtual void Draw( void );

	// Function : Process_Input
	// static input handler
	void Process_Input( void );
	// Handle Input event
	virtual bool Handle_Event( SDL_Event *ev );
	/* handle key down event
	 * returns true if the key was processed
	*/
	virtual bool Key_Down( SDLKey key );

	// Add Object to Object Manager
	virtual void Add_Object( cSprite *object );

	// ##### Main Menu

	// Add Menu Entry
	void Add_Menu_Object( string name, string tags, colour normal_color = white.Get_cegui_colour() );
	// Set Active Menu Entry
	virtual void Activate_Menu( cEditor_Menu_Object *entry );

	// ##### Item Menu
	// Load an defined Menu
	virtual bool Load_Item_Menu( string item_tag );
	// Unload the Menu
	void Unload_Item_Menu( void );
	/* Add an Object to the Item list
	 * if nName is set it will not use the object name
	 * if image is set the default object image is not used
	 */
	void Add_Item_Object( cSprite *sprite, string new_name = "", GL_Surface *image = NULL );
	// Loads all Image Items
	void Load_Image_Items( string dir );
	// Active Item Entry
	virtual void Activate_Item( cEditor_Item_Object *entry );

	// #### Editor Functions
	// push the object into the front or back
	void Change_DrawPosition( cSprite *obj, bool push_back );
	/* copy the given object(s) next to itself into the given direction
	 * if offset is given it will be used instead of the auto calculated direction size
	 * returns the new object(s)
	*/
	SpriteList Copy_Direction( SpriteList objects, ObjectDirection dir );
	cSprite *Copy_Direction( cSprite *obj, ObjectDirection dir, int offset = 0 );

	// CEGUI events
	bool Editor_Mouse_Enter( const EventArgs &event ); // Mouse entered Window
	bool Menu_Select( const EventArgs &event ); // Menu selected item
	bool Item_Select( const EventArgs &event ); // Item selected item

	// Menu functions
	void Function_Exit( void );
	virtual bool Function_New( void ) { return 0; };
	virtual void Function_Load( void ) {};
	virtual void Function_Save( bool with_dialog = 0 ) {};
	virtual void Function_Save_as( void ) {};
	virtual void Function_Delete( void ) {};
	virtual void Function_Reload( void ) {};
	virtual void Function_Clear( void ) {};
	virtual void Function_Settings( void ) {};


	// true if editor is active
	bool enabled;

	// Editor filenames
	string menu_filename;
	string items_filename;

	// Required item tag
	string editor_item_tag;
	// editor Camera speed
	float camera_speed;

	// Timer until the Menu will be minimized
	float menu_timer;

	// Objects with tags
	typedef vector<cImage_settings_data *> TaggedItemImageSettingsList;
	TaggedItemImageSettingsList tagged_item_images;
	typedef vector<cSprite *> TaggedItemObjectsList;
	TaggedItemObjectsList tagged_item_objects;

	// CEGUI window
	Window *editor_window;

protected:
	// Check if the given tag is available in the string
	bool is_tag_available( string str, string tag, unsigned int search_pos = 0 );

	// Draw Editor Help Box
    void Draw_Editor_Help( void );
	/* Add a Help Text Line to the Help Box
	 * spacing : can be used f.e. a header
	 * pos_x : the line start position x
	*/
    void Add_Help_Line( string key_text, string text = "", float spacing = 0, float pos_x = 60 );

	// if true show the editor help
    bool show_editor_help;
   	// help sprites
	HudSpriteList help_sprites;
private:
	// XML element start
    virtual void elementStart( const String &element, const XMLAttributes &attributes );
	// XML element end
    virtual void elementEnd( const String &element );

	void Handle_Item( const XMLAttributes &attributes );
	void Handle_Menu( const XMLAttributes &attributes );

	// XML element Item Tag list
	XMLAttributes xml_attributes;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#endif
