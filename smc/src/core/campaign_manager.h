/***************************************************************************
 * campaign_manager.h
 *
 * Copyright (C) 2010 - 2011 Florian Richter
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMC_CAMPAIGN_MANAGER_H
#define SMC_CAMPAIGN_MANAGER_H

#include "../core/global_basic.h"
#include "../core/global_game.h"
#include "../core/obj_manager.h"
// CEGUI
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"

namespace SMC {

/* *** *** *** *** *** cCampaign *** *** *** *** *** *** *** *** *** *** *** *** */

class cCampaign {
 public:
  cCampaign(void);
  ~cCampaign(void);

  // name
  std::string m_name;
  // target
  std::string m_target;
  // if not set it is a world
  bool m_is_target_level;
  // description
  std::string m_description;
  // last save time
  time_t m_last_saved;
  /* 0 if only in game directory
	 * 1 if only in user directory
	 * 2 if in both
	*/
  int m_user;
};

/* *** *** *** *** *** cCampaign_Manager *** *** *** *** *** *** *** *** *** *** *** *** */

class cCampaign_Manager : public cObject_Manager<cCampaign> {
 public:
  cCampaign_Manager(void);
  ~cCampaign_Manager(void) override;

  // load all campaigns
  void Load(void);
  // load a campaign (the caller owns the result; empty on failure)
  [[nodiscard]] std::unique_ptr<cCampaign> Load_Campaign(const std::string& filename);

  // Get campaign from name
  cCampaign* Get_from_Name(const std::string& name);
};

/* *** *** *** *** *** *** *** cCampaign_XML_Handler *** *** *** *** *** *** *** *** *** *** */

class cCampaign_XML_Handler : public CEGUI::XMLHandler {
 public:
  cCampaign_XML_Handler(const CEGUI::String& filename);
  ~cCampaign_XML_Handler(void) override;

  // XML element start
  void elementStart(const CEGUI::String& element,
                    const CEGUI::XMLAttributes& attributes) override;
  // XML element end
  void elementEnd(const CEGUI::String& element) override;

  // XML attributes list
  CEGUI::XMLAttributes m_xml_attributes;

  // object we are constructing (owned until Load_Campaign takes it)
  std::unique_ptr<cCampaign> m_campaign;
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

// Campaign Manager
extern cCampaign_Manager* pCampaign_Manager;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC

#endif
