/***************************************************************************
 * box.h
 *
 * Copyright (C) 2003 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_BOX_H
#define SMC_BOX_H

#include "../core/global_basic.h"
#include "../objects/animated_sprite.h"

namespace SMC {

/* *** *** *** *** *** *** *** *** Box invisible types *** *** *** *** *** *** *** *** *** */

enum Box_Invisible_Type {
  // always visible
  BOX_VISIBLE = 0,
  // visible after activation
  BOX_INVISIBLE_MASSIVE = 1,
  // only visible in ghost mode
  BOX_GHOST = 2,
  // visible after activation and only touchable in the activation direction
  BOX_INVISIBLE_SEMI_MASSIVE = 3
};

/* *** *** *** *** *** *** *** *** cBaseBox *** *** *** *** *** *** *** *** *** */

class cBaseBox : public cAnimated_Sprite {
 public:
  cBaseBox(cSprite_Manager* sprite_manager);
  ~cBaseBox(void) override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // load from savegame
  void Load_From_Savegame(cSave_Level_Object* save_object) override;
  // save to savegame
  [[nodiscard]] std::unique_ptr<cSave_Level_Object> Save_To_Savegame(
      void) override;

  /* Set the Animation Type
	 * new_anim_type can be : Bonus, Default or Power
	*/
  void Set_Animation_Type(const std::string& new_anim_type);
  // sets the count this object can be activated
  virtual void Set_Useable_Count(int count, bool new_startcount = false);
  // Set invisible type
  void Set_Invisible(Box_Invisible_Type type);

  // activates collision movement
  void Activate_Collision(ObjectDirection col_direction);
  // updates the collision movement
  void Update_Collision(void);

  // check for collision with objects
  void Check_Collision(ObjectDirection col_direction);
  // collision with the given enemy
  void Col_Enemy(cSprite* obj);

  // activate
  virtual void Activate(void);

  // update
  void Update(void) override;
  // draw
  void Draw(cSurface_Request* request = nullptr) override;

  // Generate activation Particles
  void Generate_Activation_Particles(void);

  // if update is valid for the current state
  bool Is_Update_Valid(void) override;
  // if draw is valid for the current state and position
  bool Is_Draw_Valid(void) override;

  /* Validate the given collision object
	 * returns 0 if not valid
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
  Col_Valid_Type Validate_Collision(cSprite* obj) override;
  // handle the basic box player collision
  void Handle_Collision_Player(cObjectCollision* collision) override;
  // handle the basic box enemy collision
  void Handle_Collision_Enemy(cObjectCollision* collision) override;

  // editor activation
  void Editor_Activate(void) override;
  // editor useable count text changed event
  bool Editor_Useable_Count_Text_Changed(const CEGUI::EventArgs& event);
  // editor invisible option selected event
  bool Editor_Invisible_Select(const CEGUI::EventArgs& event);

  // animation type
  std::string m_anim_type;
  // box type
  SpriteType box_type;

  // leveleditor item image
  cGL_Surface* m_item_image;

  // moving direction when activated
  ObjectDirection m_move_col_dir;
  // current moving counter ( if activated )
  float m_move_counter;
  // if object is moving back to the original position
  bool m_move_back;
  /* number of times this box object can be activated
	 * the box is visible if useable_count != start_useable_count
	 * if set to -1 it is infinite (and is set to -2 if activated)
	*/
  int m_start_useable_count;
  int m_useable_count;

  // box invisible type
  Box_Invisible_Type m_box_invisible;

  // active particle animation counter
  float m_particle_counter_active;

 protected:
  // Create the Name from the current settings
  void Create_Name(void);
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
