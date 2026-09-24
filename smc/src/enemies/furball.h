/***************************************************************************
 * furball.h
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

#ifndef SMC_FURBALL_H
#define SMC_FURBALL_H

#include "../enemies/enemy.h"

namespace SMC {

/* *** *** *** *** *** *** cFurball *** *** *** *** *** *** *** *** *** *** *** */
/* Secret attacks: Lulls you into a false sense of security so that you will
 * forget to do anything and just stupidly run into them.
 * Don't fall for it! Take them very seriously! Pay attention!
 */
class cFurball : public cEnemy {
 public:
  // constructor
  cFurball(cSprite_Manager* sprite_manager);
  // create from stream
  cFurball(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cFurball(void) override;

  // init defaults
  void Init(void) override;
  // copy
  cFurball* Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // load from savegame
  void Load_From_Savegame(cSave_Level_Object* save_object) override;

  // maximum downgrades until death
  void Set_Max_Downgrade_Count(int max_downgrade_count);
  // Set if the level ends when killed
  void Set_Level_Ends_If_Killed(bool level_ends_if_killed);

  // Set Direction
  void Set_Direction(const ObjectDirection dir);
  /* set color
	 * brown = normal, blue = ice, black = boss
	*/
  void Set_Color(const DefaultColor& col);

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

  // Generates Star Particles (only used if boss)
  void Generate_Smoke(unsigned int amount = 1,
                      float particle_scale = 0.4f) const;

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
  // editor max downgrades text changed event
  bool Editor_Max_Downgrade_Count_Text_Changed(const CEGUI::EventArgs& event);
  // editor level ends if killed
  bool Editor_Level_Ends_If_Killed(const CEGUI::EventArgs& event);

  // Color
  DefaultColor m_color_type;

 private:
  // Create the Name from the current settings
  void Create_Name(void);

  // counter if hit (only used if boss)
  float m_counter_hit;
  // counter if running (only used if boss)
  float m_counter_running;
  // particle counter if running (only used if boss)
  float m_running_particle_counter;

  // times downgraded (only used if boss)
  int m_downgrade_count;
  // maximum downgrades until death (only used if boss)
  int m_max_downgrade_count;
  // if the level ends when killed (only used if boss)
  bool m_level_ends_if_killed;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
