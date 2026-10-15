/***************************************************************************
 * level_editor.h
 *
 * Copyright (C) 2006 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_LEVEL_EDITOR_H
#define SMC_LEVEL_EDITOR_H

#include "../core/editor.h"
#include "../level/level_settings.h"

namespace SMC {

/* *** *** *** *** *** *** *** cEditor_Level *** *** *** *** *** *** *** *** *** *** */

class cEditor_Level : public cEditor {
 public:
  cEditor_Level(cSprite_Manager* sprite_manager, cLevel* level);
  ~cEditor_Level(void) override;

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

  // Set the parent level
  void Set_Level(cLevel* level);
  // Set the parent sprite manager
  void Set_Sprite_Manager(cSprite_Manager* sprite_manager) override;

  // Set Active Menu Entry
  void Activate_Menu_Item(cEditor_Menu_Object* entry) override;
  // return the sprite object
  [[nodiscard]] std::unique_ptr<cSprite> Get_Object(
      const CEGUI::String& element, CEGUI::XMLAttributes& attributes,
      int engine_version) override;

  // #### editor Functions
  /* switch the object state of the given object
	 * returns true if successful
	*/
  bool Switch_Object_State(cSprite* obj) const;

  // Menu functions
  bool Function_New(void) override;
  void Function_Load(void) override;
  void Function_Save(bool with_dialog = false) override;
  void Function_Save_as(void) override;
  void Function_Delete(void) override;
  void Function_Reload(void) override;
  void Function_Settings(void) override;

  // parent level
  cLevel* m_level;
  // Level Settings (owned value)
  cLevel_Settings m_settings_screen;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// The Level Editor
extern cEditor_Level* pLevel_Editor;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
