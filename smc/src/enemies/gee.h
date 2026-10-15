/***************************************************************************
 * gee.h
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

#ifndef SMC_GEE_H
#define SMC_GEE_H

#include "../enemies/enemy.h"

namespace SMC {

/* *** *** *** *** *** cGee *** *** *** *** *** *** *** *** *** *** *** *** */
/* Shocks you with Electro, Lava or Gift
*/
class cGee : public cEnemy {
 public:
  // constructor
  cGee(cSprite_Manager* sprite_manager);
  // create from stream
  cGee(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cGee(void) override;

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
  // Set Max Distance
  void Set_Max_Distance(int nmax_distance);
  // set color
  void Set_Color(DefaultColor col);

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
  // draw
  void Draw(cSurface_Request* request = nullptr) override;

  // Start Movement
  void Activate(void);
  // Stop Movement
  void Stop(void);

  // generate small cloud particles ( for moving )
  void Generate_Particles(unsigned int amount = 4) const;

  // Check if position is beyond the max distance
  bool Is_At_Max_Distance(void) const;

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
  // collision from player
  void Handle_Collision_Player(cObjectCollision* collision) override;
  // collision with massive
  void Handle_Collision_Massive(cObjectCollision* collision) override;

  // editor activation
  void Editor_Activate(void) override;
  // editor direction option selected event
  bool Editor_Direction_Select(const CEGUI::EventArgs& event);
  // editor max distance text changed event
  bool Editor_Max_Distance_Text_Changed(const CEGUI::EventArgs& event);
  // editor always fly option selected event
  bool Editor_Always_Fly_Select(const CEGUI::EventArgs& event);
  // editor wait time text changed event
  bool Editor_Wait_Time_Text_Changed(const CEGUI::EventArgs& event);
  // editor fly distance text changed event
  bool Editor_Fly_Distance_Text_Changed(const CEGUI::EventArgs& event);

  // color
  DefaultColor m_color_type;

  // flying velocity
  float m_speed_fly;
  // maximum distance from the startposition
  int m_max_distance;
  // always fly around
  bool m_always_fly;
  // time to wait until next movement
  float m_wait_time;
  // fly distance
  int m_fly_distance;

  // wait time counter
  float m_wait_time_counter;
  // fly distance counter
  float m_fly_distance_counter;
  // clouds particle counter
  float m_clouds_counter;

 private:
  // Create the Name from the current settings
  void Create_Name(void);
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
