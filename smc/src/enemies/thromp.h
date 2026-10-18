/***************************************************************************
 * thromp.h
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

#ifndef SMC_THROMP_H
#define SMC_THROMP_H

#include "../enemies/enemy.h"

namespace SMC {

/* *** *** *** *** *** cThromp *** *** *** *** *** *** *** *** *** *** *** *** */
/* The falling Stone
*/
class cThromp : public cEnemy {
 public:
  // constructor
  cThromp(cSprite_Manager* sprite_manager);
  // create from stream
  cThromp(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cThromp(void) override;

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
  // save to savegame
  [[nodiscard]] std::unique_ptr<cSave_Level_Object> Save_To_Savegame(
      void) override;

  // Set the image directory
  void Set_Image_Dir(std::string filename);
  // Set Direction
  // clang attributes -Woverloaded-virtual to this derived decl (gcc attributes it to the base, which
  // is suppressed there); this narrowing of cMovingSprite::Set_Direction(ObjectDirection, bool=false)
  // is intentional (maintainer, 2026-10-18; tasks/smc-overloaded-virtual.md).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
  void Set_Direction(const ObjectDirection dir);
#pragma GCC diagnostic pop
  // Set Max Distance
  void Set_Max_Distance(float nmax_distance);
  // Set the Speed
  void Set_Speed(float val);

  // Move to destination direction
  void Activate(void);
  // Move back to the original position
  bool Move_Back(void);

  /* downgrade state ( if already the weakest state it dies )
	 * force : usually dies or a complete downgrade
	*/
  void DownGrade(bool force = false) override;
  // dying animation update
  void Update_Dying(void) override;

  // update
  void Update(void) override;
  // draw
  void Draw(cSurface_Request* request = nullptr) override;

  // update images
  void Update_Images(void);
  // update destination velocity from the speed
  void Update_Dest_Vel(void);
  // update the distance rect
  void Update_Distance_Rect(void);
  // Get the final distance rect
  GL_rect Get_Final_Distance_Rect(void) const;

  // Generates Smoke Particles
  void Generate_Smoke(unsigned int amount = 20) const;

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
  // collision from an enemy
  void Handle_Collision_Enemy(cObjectCollision* collision) override;
  // collision with massive
  void Handle_Collision_Massive(cObjectCollision* collision) override;
  // handle moved out of Level event
  void Handle_out_of_Level(ObjectDirection dir) override;

  // editor activation
  void Editor_Activate(void) override;
  // editor direction option selected event
  bool Editor_Direction_Select(const CEGUI::EventArgs& event);
  // editor image dir text changed event
  bool Editor_Image_Dir_Text_Changed(const CEGUI::EventArgs& event);
  // editor max distance text changed event
  bool Editor_Max_Distance_Text_Changed(const CEGUI::EventArgs& event);
  // editor speed text changed event
  bool Editor_Speed_Text_Changed(const CEGUI::EventArgs& event);

  // image directory
  std::string m_img_dir;
  // speed
  float m_speed;
  // destination direction velocity
  float m_dest_velx, m_dest_vely;
  // maximum distance from the startposition
  float m_max_distance;

  // moving back to the original position
  bool m_move_back;
  // distance rect to end position
  GL_rect m_distance_rect;

 private:
  // Create the Name from the current settings
  void Create_Name(void);
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
