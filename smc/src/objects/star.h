/***************************************************************************
 * star.h
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

#ifndef SMC_STAR_H
#define SMC_STAR_H

#include "../core/global_basic.h"
#include "../objects/powerup.h"

namespace SMC {

/* *** *** *** *** *** Star *** *** *** *** *** *** *** *** *** *** *** *** */

class cjStar : public cPowerUp {
 public:
  // constructor
  cjStar(cSprite_Manager* sprite_manager);
  // create from stream
  cjStar(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cjStar(void) override;

  // init defaults
  void Init(void) override;
  // copy
  [[nodiscard]] std::unique_ptr<cSprite> Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // Activate the star
  void Activate(void);

  // update the Star
  void Update(void) override;
  // draw the Star
  void Draw(cSurface_Request* request = nullptr) override;

  // Adds Star Particles
  void Generate_Particles(float x = 0.0f, float y = 0.0f, bool random = true,
                          unsigned int quota = 2) const;

  // collision from player
  void Handle_Collision_Player(cObjectCollision* collision) override;
  // collision with massive
  void Handle_Collision_Massive(cObjectCollision* collision) override;

  // small stars animation counter
  float m_anim_counter;

  // glim animation modifier
  bool m_glim_mod;
  // glim animation counter
  float m_glim_counter;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
