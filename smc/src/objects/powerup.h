/***************************************************************************
 * powerup.h
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

#ifndef SMC_POWERUP_H
#define SMC_POWERUP_H

#include "../core/global_basic.h"
#include "../objects/animated_sprite.h"

namespace SMC {

/* *** *** *** *** *** cPowerUp *** *** *** *** *** *** *** *** *** *** *** *** */

class cPowerUp : public cAnimated_Sprite {
 public:
  // constructor
  cPowerUp(cSprite_Manager* sprite_manager);
  // destructor
  ~cPowerUp(void) override;

  // load from savegame
  void Load_From_Savegame(cSave_Level_Object* save_object) override;
  // save to savegame
  [[nodiscard]] std::unique_ptr<cSave_Level_Object> Save_To_Savegame(
      void) override;

  /* draw
	 * a spawned powerup doesn't draw in editor mode
	*/
  void Draw(cSurface_Request* request = nullptr) override;

  // if update is valid for the current state
  bool Is_Update_Valid(void) override;

  /* Validate the given collision object
	 * returns 0 if not valid
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
  Col_Valid_Type Validate_Collision(cSprite* obj) override;
  // handle moved out of Level event
  void Handle_out_of_Level(ObjectDirection dir) override;

  float m_counter;
};

/* *** *** *** *** *** cMushroom *** *** *** *** *** *** *** *** *** *** *** *** */

class cMushroom : public cPowerUp {
 public:
  // constructor
  cMushroom(cSprite_Manager* sprite_manager);
  // create from stream
  cMushroom(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cMushroom(void) override;

  // init defaults
  void Init(void) override;
  // copy
  [[nodiscard]] std::unique_ptr<cSprite> Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // Set the Mushroom Type
  void Set_Type(SpriteType new_type);

  // Activates the item
  virtual void Activate(void);

  // update
  void Update(void) override;

  // collision from player
  void Handle_Collision_Player(cObjectCollision* collision) override;
  // collision with massive
  void Handle_Collision_Massive(cObjectCollision* collision) override;
  // collision from a box
  void Handle_Collision_Box(ObjectDirection cdirection, GL_rect* r2) override;

  // glim animation modifier
  bool m_glim_mod;
};

/* *** *** *** *** *** cFirePlant *** *** *** *** *** *** *** *** *** *** *** *** */

class cFirePlant : public cPowerUp {
 public:
  // constructor
  cFirePlant(cSprite_Manager* sprite_manager);
  // create from stream
  cFirePlant(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cFirePlant(void) override;

  // init defaults
  void Init(void) override;
  // copy
  [[nodiscard]] std::unique_ptr<cSprite> Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // Activates the item
  virtual void Activate(void);

  // update
  void Update(void) override;

  // collision from player
  void Handle_Collision_Player(cObjectCollision* collision) override;

  float m_particle_counter;
};

/* *** *** *** *** *** cMoon *** *** *** *** *** *** *** *** *** *** *** *** */

class cMoon : public cPowerUp {
 public:
  // constructor
  cMoon(cSprite_Manager* sprite_manager);
  // create from stream
  cMoon(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cMoon(void) override;

  // init defaults
  void Init(void) override;
  // copy
  [[nodiscard]] std::unique_ptr<cSprite> Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // Activates the item
  virtual void Activate(void);

  // update
  void Update(void) override;

  // collision from player
  void Handle_Collision_Player(cObjectCollision* collision) override;

  float m_particle_counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
