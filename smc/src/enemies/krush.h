/***************************************************************************
 * krush.h
 *
 * Copyright (C) 2004 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_KRUSH_H
#define SMC_KRUSH_H

#include "../enemies/enemy.h"

namespace SMC {

/* *** *** *** *** *** cKrush *** *** *** *** *** *** *** *** *** *** *** *** */
/* Walking all Day and doesn't stop even if you hit him the first time :O
 * but the second time he's out.
 * Colors : Red
 */
class cKrush : public cEnemy {
 public:
  // constructor
  cKrush(cSprite_Manager* sprite_manager);
  // create from stream
  cKrush(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cKrush(void) override;

  // init defaults
  void Init(void) override;
  // copy
  [[nodiscard]] std::unique_ptr<cSprite> Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // load from savegame
  void Load_From_Savegame(cSave_Level_Object* save_object) override;

  // Set Direction
  void Set_Direction(const ObjectDirection dir);

  /* Move into the opposite Direction
	 * if col_dir is given only turns around if the collision direction is in front
	 */
  void Turn_Around(ObjectDirection col_dir = DIR_UNDEFINED) override;

  /* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
  void DownGrade(bool force = false) override;
  // dying animation update
  void Update_Dying(void) override;

  // set the moving state
  void Set_Moving_State(Moving_state new_state);

  // update
  void Update(void) override;

  // update maximum velocity values
  void Update_Velocity_Max(void);

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

  // editor activation
  void Editor_Activate(void) override;
  // editor direction option selected event
  bool Editor_Direction_Select(const CEGUI::EventArgs& event);
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
