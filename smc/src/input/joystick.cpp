/***************************************************************************
 * joystick.cpp  -  Joystick handling class
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

#include "../core/global_basic.h"
#include "../input/keyboard.h"
#include "../input/joystick.h"
#include "../user/preferences.h"
#include "../core/game_core.h"
#include "../level/level_player.h"
#include "../gui/hud.h"
#include "../gui/menu.h"
#include "../level/level.h"
#include "../overworld/overworld.h"

namespace SMC {

/* *** *** *** *** *** *** cJoystick *** *** *** *** *** *** *** *** *** *** *** */

cJoystick ::cJoystick(void) {
  m_joystick = nullptr;

  m_joystick_open = false;

  m_current_joystick = 0;
  m_num_buttons = 0;
  m_num_axes = 0;
  m_num_balls = 0;

  m_debug = false;

  Reset_keys();

  Init();
}

cJoystick ::~cJoystick(void) { Close(); }

int cJoystick ::Init(void) {
  // if not enabled
  if (!pPreferences->m_joy_enabled) {
    return 0;
  }

  int const joy_count = SDL_NumJoysticks();

  // no joystick available
  if (joy_count <= 0) {
    printf("No joysticks available\n");
    pPreferences->m_joy_enabled = false;
    return 0;
  }

  if (m_debug) {
    printf("Joysticks found : %d\n\n", joy_count);
  }

  unsigned int default_joy = 0;

  // if default joy name is given
  if (!pPreferences->m_joy_name.empty()) {
    vector<std::string> joy_names = Get_Names();

    for (unsigned int i = 0; i < joy_names.size(); i++) {
      // found default joy
      if (joy_names[i].compare(pPreferences->m_joy_name) == 0) {
        default_joy = i;
        break;
      }
    }
  }

  // setup
  SDL_JoystickEventState(SDL_ENABLE);
  Stick_Open(default_joy);

  if (m_debug) {
    printf("Joypad System Initialized\n");
  }

  return 1;
}

void cJoystick ::Close(void) { Stick_Close(); }

bool cJoystick ::Stick_Open(unsigned int index) {
  // if a joystick is already opened close it first
  if (m_joystick_open) {
    Stick_Close();
  }

  m_joystick = SDL_JoystickOpen(static_cast<int>(index));

  if (!m_joystick) {
    printf("Couldn't open joystick %d\n", index);
    m_joystick_open = false;
    return false;
  }

  m_current_joystick = static_cast<int>(index);

  m_num_buttons = static_cast<unsigned int>(SDL_JoystickNumButtons(m_joystick));
  m_num_axes = static_cast<unsigned int>(SDL_JoystickNumAxes(m_joystick));
  m_num_balls = static_cast<unsigned int>(SDL_JoystickNumBalls(m_joystick));

  // setup available buttons
  m_buttons.assign(m_num_buttons, false);

  if (m_debug) {
    printf("Opened Joystick %d\n", m_current_joystick);
    printf("Name: %s\n", Get_Name().c_str());
    printf("Number of Buttons: %d\n", m_num_buttons);
    printf("Number of Axes: %d\n", m_num_axes);
    printf("Number of Balls: %d\n\n", m_num_balls);
  }

  m_joystick_open = true;
  return true;
}

void cJoystick ::Stick_Close(void) {
  // not available
  if (!m_joystick) {
    return;
  }

  SDL_JoystickClose(m_joystick);
  m_joystick = nullptr;

  Reset_keys();

  m_num_buttons = 0;
  m_num_axes = 0;
  m_num_balls = 0;

  m_buttons.clear();
  m_joystick_open = false;

  if (m_debug) {
    printf("Joystick %d closed\n", m_current_joystick);
  }

  m_current_joystick = 0;
}

void cJoystick ::Reset_keys(void) {
  // clear buttons
  std::fill(m_buttons.begin(), m_buttons.end(), false);

  m_left = false;
  m_right = false;
  m_up = false;
  m_down = false;
}

void cJoystick ::Handle_Hat(SDL_Event* ev) {
  // up
  if (ev->jhat.value == SDL_HAT_UP) {
    if (!m_up) {
      pKeyboard->Key_Down(pPreferences->m_key_up);
      m_up = true;
    }

    if (m_down) {
      pKeyboard->Key_Up(pPreferences->m_key_down);
      m_down = false;
    }
  } else {
    if (m_up) {
      pKeyboard->Key_Up(pPreferences->m_key_up);
      m_up = false;
    }
  }

  // down
  if (ev->jhat.value == SDL_HAT_DOWN) {
    if (!m_down) {
      pKeyboard->Key_Down(pPreferences->m_key_down);
      m_down = true;
    }

    if (m_up) {
      pKeyboard->Key_Up(pPreferences->m_key_up);
      m_up = false;
    }
  } else {
    if (m_down) {
      pKeyboard->Key_Up(pPreferences->m_key_down);
      m_down = false;
    }
  }

  // left
  if (ev->jhat.value == SDL_HAT_LEFT) {
    if (!m_left) {
      pKeyboard->Key_Down(pPreferences->m_key_left);
      m_left = true;
    }

    if (m_right) {
      pKeyboard->Key_Up(pPreferences->m_key_right);
      m_right = false;
    }
  } else {
    if (m_left) {
      pKeyboard->Key_Up(pPreferences->m_key_left);
      m_left = false;
    }
  }

  // right
  if (ev->jhat.value == SDL_HAT_RIGHT) {
    if (!m_right) {
      pKeyboard->Key_Down(pPreferences->m_key_right);
      m_right = true;
    }

    if (m_left) {
      pKeyboard->Key_Up(pPreferences->m_key_left);
      m_left = false;
    }
  } else {
    if (m_right) {
      pKeyboard->Key_Up(pPreferences->m_key_right);
      m_right = false;
    }
  }
}

void cJoystick ::Handle_Motion(SDL_Event* ev) {
  // Vertical Axis
  if (ev->jaxis.axis == pPreferences->m_joy_axis_ver) {
    // Up
    if (ev->jaxis.value < -pPreferences->m_joy_axis_threshold) {
      if (m_debug) {
        printf("Joystick %d : Up Button pressed\n", m_current_joystick);
      }

      if (!m_up) {
        pKeyboard->Key_Down(pPreferences->m_key_up);
        m_up = true;
      }

      if (m_down) {
        pKeyboard->Key_Up(pPreferences->m_key_down);
        m_down = false;
      }
    }
    // Down
    else if (ev->jaxis.value > pPreferences->m_joy_axis_threshold) {
      if (m_debug) {
        printf("Joystick %d : Down Button pressed\n", m_current_joystick);
      }

      if (!m_down) {
        pKeyboard->Key_Down(pPreferences->m_key_down);
        m_down = true;
      }

      if (m_up) {
        pKeyboard->Key_Up(pPreferences->m_key_up);
        m_up = false;
      }
    }
    // No Down/Left
    else {
      if (m_down) {
        pKeyboard->Key_Up(pPreferences->m_key_down);
        m_down = false;
      }

      if (m_up) {
        pKeyboard->Key_Up(pPreferences->m_key_up);
        m_up = false;
      }
    }
  }
  // Horizontal Axis
  else if (ev->jaxis.axis == pPreferences->m_joy_axis_hor) {
    // Left
    if (ev->jaxis.value < -pPreferences->m_joy_axis_threshold) {
      if (m_debug) {
        printf("Joystick %d : Left Button pressed\n", m_current_joystick);
      }

      if (!m_left) {
        pKeyboard->Key_Down(pPreferences->m_key_left);
        m_left = true;
      }

      if (m_right) {
        pKeyboard->Key_Up(pPreferences->m_key_right);
        m_right = false;
      }
    }
    // Right
    else if (ev->jaxis.value > pPreferences->m_joy_axis_threshold) {
      if (m_debug) {
        printf("Joystick %d : Right Button pressed\n", m_current_joystick);
      }

      if (!m_right) {
        pKeyboard->Key_Down(pPreferences->m_key_right);
        m_right = true;
      }

      if (m_left) {
        pKeyboard->Key_Up(pPreferences->m_key_left);
        m_left = false;
      }
    }
    // No Left/Right
    else {
      if (m_left) {
        pKeyboard->Key_Up(pPreferences->m_key_left);
        m_left = false;
      }

      if (m_right) {
        pKeyboard->Key_Up(pPreferences->m_key_right);
        m_right = false;
      }
    }
  }
}

bool cJoystick ::Handle_Button_Down_Event(SDL_Event* ev) {
  // not enabled or opened
  if (!pPreferences->m_joy_enabled || !m_joystick_open) {
    return false;
  }

  Set_Button(ev->jbutton.button, true);

  // handle button in the current mode
  if (Game_Mode == MODE_LEVEL) {
    // processed by the level
    if (pActive_Level->Joy_Button_Down(ev->jbutton.button)) {
      return true;
    }
  } else if (Game_Mode == MODE_OVERWORLD) {
    // processed by the overworld
    if (pActive_Overworld->Joy_Button_Down(ev->jbutton.button)) {
      return true;
    }
  } else if (Game_Mode == MODE_MENU) {
    // processed by the menu
    if (pMenuCore->Joy_Button_Down(ev->jbutton.button)) {
      return true;
    }
  }

  if (ev->jbutton.button < m_buttons.size()) {
    // Jump
    if (ev->jbutton.button == pPreferences->m_joy_button_jump) {
      //
    }
    // Shoot
    else if (ev->jbutton.button == pPreferences->m_joy_button_shoot) {
      pKeyboard->Key_Down(pPreferences->m_key_shoot);
      return true;
    }
    // Request Itembox Item
    else if (ev->jbutton.button == pPreferences->m_joy_button_item) {
      // not handled
      return true;
    }
    // Interaction
    else if (ev->jbutton.button == pPreferences->m_joy_button_action) {
      pKeyboard->Key_Down(pPreferences->m_key_action);
      return true;
    }
    // Exit
    else if (ev->jbutton.button == pPreferences->m_joy_button_exit) {
      pKeyboard->Key_Down(SDLK_ESCAPE);
      return true;
    }
    // Pause
    else if (ev->jbutton.button == 9) {
      pKeyboard->Key_Down(SDLK_PAUSE);
      return true;
    }
  }

  return false;
}

bool cJoystick ::Handle_Button_Up_Event(SDL_Event* ev) {
  // not enabled or opened
  if (!pPreferences->m_joy_enabled || !m_joystick_open) {
    return false;
  }

  Set_Button(ev->jbutton.button, false);

  // handle button in the current mode
  if (Game_Mode == MODE_LEVEL) {
    // processed by the level
    if (pActive_Level->Joy_Button_Up(ev->jbutton.button)) {
      return true;
    }
  } else if (Game_Mode == MODE_OVERWORLD) {
    // processed by the overworld
    if (pActive_Overworld->Joy_Button_Up(ev->jbutton.button)) {
      return true;
    }
  } else if (Game_Mode == MODE_MENU) {
    // processed by the menu
    if (pMenuCore->Joy_Button_Up(ev->jbutton.button)) {
      return true;
    }
  }

  if (ev->jbutton.button < m_buttons.size()) {
    if (ev->jbutton.button == pPreferences->m_joy_button_jump) {
      pKeyboard->Key_Up(pPreferences->m_key_jump);
      return true;
    } else if (ev->jbutton.button == pPreferences->m_joy_button_shoot) {
      pKeyboard->Key_Up(pPreferences->m_key_shoot);
      return true;
    } else if (ev->jbutton.button == pPreferences->m_joy_button_item) {
      // not handled
    } else if (ev->jbutton.button == pPreferences->m_joy_button_action) {
      pKeyboard->Key_Up(pPreferences->m_key_action);
      return true;
    } else if (ev->jbutton.button == pPreferences->m_joy_button_exit) {
      pKeyboard->Key_Up(SDLK_ESCAPE);
      return true;
    }
  }

  return false;
}

std::string cJoystick ::Get_Name(void) const {
  return SDL_JoystickNameForIndex(m_current_joystick);
}

vector<std::string> cJoystick ::Get_Names(void) const {
  vector<std::string> names;
  // get joy count
  int const joy_count = SDL_NumJoysticks();

  // joystick names
  for (int i = 0; i < joy_count; i++) {
    names.emplace_back(SDL_JoystickNameForIndex(i));
  }

  return names;
}

void cJoystick ::Set_Button(Uint8 num, bool pressed) {
  // not available
  if (num >= m_buttons.size()) {
    return;
  }

  if (m_debug) {
    if (pressed) {
      printf("Joystick %d : Joy Button %d pressed\n", m_current_joystick, num);
    } else {
      printf("Joystick %d : Joy Button %d released\n", m_current_joystick, num);
    }
  }

  m_buttons[num] = pressed;
}

bool cJoystick ::Left(void) const {
  if (pPreferences->m_joy_enabled && input_event.type == SDL_JOYAXISMOTION &&
      input_event.jaxis.value < -pPreferences->m_joy_axis_threshold &&
      input_event.jaxis.axis == pPreferences->m_joy_axis_hor) {
    return true;
  }

  return false;
}

bool cJoystick ::Right(void) const {
  if (pPreferences->m_joy_enabled && input_event.type == SDL_JOYAXISMOTION &&
      input_event.jaxis.value > pPreferences->m_joy_axis_threshold &&
      input_event.jaxis.axis == pPreferences->m_joy_axis_hor) {
    return true;
  }

  return false;
}

bool cJoystick ::Up(void) const {
  if (pPreferences->m_joy_enabled && input_event.type == SDL_JOYAXISMOTION &&
      input_event.jaxis.value < -pPreferences->m_joy_axis_threshold &&
      input_event.jaxis.axis == pPreferences->m_joy_axis_ver) {
    return true;
  }

  return false;
}

bool cJoystick ::Down(void) const {
  if (pPreferences->m_joy_enabled && input_event.type == SDL_JOYAXISMOTION &&
      input_event.jaxis.value > pPreferences->m_joy_axis_threshold &&
      input_event.jaxis.axis == pPreferences->m_joy_axis_ver) {
    return true;
  }

  return false;
}

bool cJoystick ::Button(Uint8 num) {
  // if available and pressed
  if (num < m_buttons.size() && m_buttons[num]) {
    return true;
  }

  return false;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

cJoystick* pJoystick = nullptr;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
