/***************************************************************************
 * eato.h
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

#ifndef SMC_EATO_H
#define SMC_EATO_H

#include "../enemies/enemy.h"

namespace SMC {

/* *** *** *** *** *** cEato *** *** *** *** *** *** *** *** *** *** *** *** */
/* Eats your Butt !
 * Secret attack: Merges visually with the background and becomes an anti-maryo mine.
*/
class cEato : public cEnemy {
 public:
  // constructor
  cEato(cSprite_Manager* sprite_manager);
  // create from stream
  cEato(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cEato(void) override;

  // init defaults
  void Init(void) override;
  // copy
  cEato* Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // Set the image directory
  void Set_Image_Dir(std::string dir);
  // Set direction
  void Set_Direction(const ObjectDirection dir);

  /* downgrade state ( if already weakest state : dies )
	 * force : usually dies or a complete downgrade
	*/
  void DownGrade(bool force = 0) override;
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

  // editor activation
  void Editor_Activate(void) override;
  // editor direction option selected event
  bool Editor_Direction_Select(const CEGUI::EventArgs& event);
  // editor image dir text changed event
  bool Editor_Image_Dir_Text_Changed(const CEGUI::EventArgs& event);

  // image directory
  std::string m_img_dir;

 private:
  // Create the Name from the current settings
  void Create_Name(void);
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
