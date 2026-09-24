/***************************************************************************
 * world_editor.h
 *
 * Copyright (C) 2007 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_WORLD_EDITOR_H
#define SMC_WORLD_EDITOR_H

#include "../core/editor.h"

namespace SMC {

/* *** *** *** *** *** *** *** cEditor_World *** *** *** *** *** *** *** *** *** *** */

class cEditor_World : public cEditor {
 public:
  cEditor_World(cSprite_Manager* sprite_manager, cOverworld* overworld);
  ~cEditor_World(void) override;

  // Initialize
  void Init(void) override;

  // Enable
  void Enable(void) override;
  /* Disable
	 * native_mode : if unset the current game mode isn't altered
 	*/
  void Disable(bool native_mode = false) override;

  /* handle key down event
	 * returns true if the key was processed
	*/
  bool Key_Down(SDLKey key) override;

  // Set the parent overworld
  void Set_Overworld(cOverworld* overworld);
  // Set Active Menu Entry
  void Activate_Menu_Item(cEditor_Menu_Object* entry) override;
  // return the sprite object
  cSprite* Get_Object(const CEGUI::String& element,
                      CEGUI::XMLAttributes& attributes,
                      int engine_version) override;

  // Menu functions
  bool Function_New(void) override;
  void Function_Load(void) override;
  void Function_Save(bool with_dialog = false) override;
  //virtual void Function_Save_as( void );
  void Function_Reload(void) override;
  //void Function_Settings( void );

  // parent overworld
  cOverworld* m_overworld;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The World Editor
extern cEditor_World* pWorld_Editor;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
