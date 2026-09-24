/***************************************************************************
 * text_box.h
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

#ifndef SMC_TEXT_BOX_H
#define SMC_TEXT_BOX_H

#include "../core/global_basic.h"
#include "../objects/box.h"

namespace SMC {

/* *** *** *** *** *** *** *** *** cText_Box *** *** *** *** *** *** *** *** *** */

class cText_Box : public cBaseBox {
 public:
  // constructor
  cText_Box(cSprite_Manager* sprite_manager);
  // create from stream
  cText_Box(CEGUI::XMLAttributes& attributes, cSprite_Manager* sprite_manager);
  // destructor
  ~cText_Box(void) override;

  // init defaults
  void Init(void) override;
  // copy
  std::unique_ptr<cSprite> Copy(void) const override;

  // load from stream
  void Load_From_XML(CEGUI::XMLAttributes& attributes) override;
  // save to stream
  void Save_To_XML(CEGUI::XMLSerializer& stream) override;

  // Activate
  void Activate(void) override;

  // update
  void Update(void) override;

  // Set Text
  void Set_Text(const std::string& str_text);

  // editor activation
  void Editor_Activate(void) override;
  // editor text text changed event
  bool Editor_Text_Text_Changed(const CEGUI::EventArgs& event);

  // the text
  std::string m_text;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
