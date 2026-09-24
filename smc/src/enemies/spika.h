/***************************************************************************
 * spika.h
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

#ifndef SMC_SPIKA_H
#define SMC_SPIKA_H

#include "../enemies/enemy.h"

namespace SMC {

/* *** *** *** *** *** *** cSpika *** *** *** *** *** *** *** *** *** *** *** */
/* Waits for you and wheel you if you can't run fast enough :D
 * 
 * Colors : Orange, Green and Grey
 */
class cSpika : public cEnemy {
 public:
  // constructor
  cSpika(cSprite_Manager* sprite_manager);
  // create from stream
  cSpika(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cSpika(void) override;

  // init defaults
  void Init(void) override;
  // copy
  std::unique_ptr<cSprite> Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // set color
  void Set_Color(DefaultColor col);

  /* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
  void DownGrade(bool force = false) override;
  // dying animation update
  void Update_Dying(void) override;

  // update
  void Update(void) override;

  // if update is valid for the current state
  bool Is_Update_Valid(void) override;

  /* Validate the given collision object
	 * returns 0 if not valid
	 * returns 1 if an internal collision with this object is valid
	 * returns 2 if the given object collides with this object (blocking)
	*/
  Col_Valid_Type Validate_Collision(cSprite* obj) override;
  // collision from player
  void Handle_Collision_Player(cObjectCollision* collision) override;
  // collision from an enemy
  void Handle_Collision_Enemy(cObjectCollision* collision) override;
  // collision with massive
  void Handle_Collision_Massive(cObjectCollision* collision) override;
  // collision from a box
  void Handle_Collision_Box(ObjectDirection cdirection, GL_rect* r2) override;

  // Color
  DefaultColor m_color_type;
  // default speed
  float m_speed;
  // default detection size
  float m_detection_size;

  // counter for walking
  float m_walk_count;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
