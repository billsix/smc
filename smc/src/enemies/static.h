/***************************************************************************
 * static.h
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

#ifndef SMC_STATIC_ENEMY_H
#define SMC_STATIC_ENEMY_H

#include "../enemies/enemy.h"
#include "../objects/path.h"

namespace SMC {

/* *** *** *** *** *** *** cStaticEnemy *** *** *** *** *** *** *** *** *** *** *** */
/* It doesn't move but will hit you if you touch it.
 */
class cStaticEnemy : public cEnemy {
 public:
  // constructor
  cStaticEnemy(cSprite_Manager* sprite_manager);
  // create from stream
  cStaticEnemy(CEGUI::XMLAttributes& attributes,
               cSprite_Manager* sprite_manager);
  // destructor
  ~cStaticEnemy(void) override;

  // init defaults
  void Init(void) override;
  /* late initialization
	 * this needs linked objects to be already loaded
	*/
  void Init_Links(void) override;
  // copy
  cStaticEnemy* Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // Set the parent sprite manager
  void Set_Sprite_Manager(cSprite_Manager* sprite_manager) override;

  // load from savegame
  void Load_From_Savegame(cSave_Level_Object* save_object) override;
  // save to savegame
  cSave_Level_Object* Save_To_Savegame(void) override;

  // Set the static image
  void Set_Static_Image(const std::string& filename);
  // Set the rotation speed
  void Set_Rotation_Speed(float speed);
  // Set the movement speed
  void Set_Speed(float speed);
  // Set the path identifier
  void Set_Path_Identifier(const std::string& path);

  /* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
  void DownGrade(bool force = 0) override;
  // dying animation update
  void Update_Dying(void) override;

  // update
  void Update(void) override;
  // draw
  void Draw(cSurface_Request* request /* = NULL */) override;

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

  // leveleditor activation
  void Editor_Activate(void) override;
  // editor image text changed event
  bool Editor_Image_Text_Changed(const CEGUI::EventArgs& event);
  // editor rotation speed text changed event
  bool Editor_Rotation_Speed_Text_Changed(const CEGUI::EventArgs& event);
  // editor path identifier text changed event
  bool Editor_Path_Identifier_Text_Changed(const CEGUI::EventArgs& event);
  // editor speed text changed event
  bool Editor_Speed_Text_Changed(const CEGUI::EventArgs& event);
  // editor fire resistant option selected event
  bool Editor_Fire_Resistant_Select(const CEGUI::EventArgs& event);
  // editor ice resistance text changed event
  bool Editor_Ice_Resistance_Text_Changed(const CEGUI::EventArgs& event);

  // image filename
  std::string m_img_filename;
  // rotation speed
  float m_rotation_speed;
  // movement speed if using path
  float m_speed;

  // path state if linked to a path
  cPath_State m_path_state;

 private:
  // Create the Name from the current settings
  void Create_Name(void);
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
