/***************************************************************************
 * main.cpp  -  main routines and initialization
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
#include "../core/game_core.h"
#include "../core/main.h"
#include "../core/filesystem/resource_manager.h"
#include "../core/filesystem/filesystem.h"
#include "../level/level.h"
#include "../gui/menu.h"
#include "../core/framerate.h"
#include "../video/font.h"
#include "../user/preferences.h"
#include "../audio/sound_manager.h"
#include "../audio/audio.h"
#include "../level/level_editor.h"
#include "../overworld/world_editor.h"
#include "../input/joystick.h"
#include "../overworld/world_manager.h"
#include "../overworld/overworld.h"
#include "../core/campaign_manager.h"
#include "../input/mouse.h"
#include "../user/savegame.h"
#include "../input/keyboard.h"
#include "../video/renderer.h"
#include "../core/i18n.h"
#include "../gui/generic.h"
#include <memory>

#ifdef __APPLE__
// needed for datapath detection
#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
#endif

// CEGUI
#include "CEGUIDefaultLogger.h"

namespace SMC {
// A singleton slot: the owning pointer plus the `extern cX* pX` alias every call site uses. The
// alias is set when the object is created and cleared when it dies, so no alias ever dangles.
template <class T>
class cOwned {
 public:
  explicit cOwned(T*& alias) : m_alias(alias) {}
  ~cOwned(void) {
    m_ptr.reset();
    m_alias = nullptr;
  }
  cOwned(const cOwned&) = delete;
  cOwned& operator=(const cOwned&) = delete;

  // Create the object (forwarding the constructor arguments) and point the alias at it
  template <class... Args>
  void Create(Args&&... args) {
    m_ptr = std::make_unique<T>(std::forward<Args>(args)...);
    m_alias = m_ptr.get();
  }

 private:
  std::unique_ptr<T> m_ptr;
  T*& m_alias;
};

// Owns every process-global singleton for the whole run. Destruction is the implicit destructor's
// job: members die in reverse declaration order, so they are declared in the REVERSE of the
// teardown order the original hand-written Exit_Game used (audio first ... resource manager last).
// The CEGUI system, which that order destroyed between the framerate and the video subsystem,
// dies in ~cVideo.
class cApplication {
 public:
  cOwned<cResource_Manager> m_resource_manager{pResource_Manager};
  cOwned<cFont_Manager> m_font{pFont};
  cOwned<cImage_Settings_Parser> m_settings_parser{pSettingsParser};
  cOwned<cImage_Manager> m_image_manager{pImage_Manager};
  cOwned<cVideo> m_video{pVideo};
  cOwned<cFramerate> m_framerate{pFramerate};
  cOwned<cRenderQueue> m_renderer_current{pRenderer_current};
  cOwned<cRenderQueue> m_renderer{pRenderer};
  cOwned<cMenuCore> m_menu_core{pMenuCore};
  cOwned<cLevel_Manager> m_level_manager{pLevel_Manager};
  cOwned<cOverworld_Player> m_overworld_player{pOverworld_Player};
  cOwned<cOverworld_Manager> m_overworld_manager{pOverworld_Manager};
  cOwned<cCampaign_Manager> m_campaign_manager{pCampaign_Manager};
  cOwned<cKeyboard> m_keyboard{pKeyboard};
  cOwned<cJoystick> m_joystick{pJoystick};
  cOwned<cMouseCursor> m_mouse_cursor{pMouseCursor};
  cOwned<cSavegame> m_savegame{pSavegame};
  cOwned<cPreferences> m_preferences{pPreferences};
  cOwned<cEditor_World> m_world_editor{pWorld_Editor};
  cOwned<cEditor_Level> m_level_editor{pLevel_Editor};
  cOwned<cSound_Manager> m_sound_manager{pSound_Manager};
  cOwned<cHud_Manager> m_hud_manager{pHud_Manager};
  cOwned<cLevel_Player> m_level_player{pLevel_Player};
  cOwned<cAudio> m_audio{pAudio};
};
}  // namespace SMC

// SMC namespace is set later to exclude main() from it
using namespace SMC;

// owns the singletons; the pX globals are borrowed aliases into it
static std::unique_ptr<cApplication> g_app;

// SDLmain defines this for Win32 applications but under debug we use the console
#if defined(__WIN32__) && defined(_DEBUG)
#undef main
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char** argv) {
// todo : remove this apple hack
#ifdef __APPLE__
  // dynamic datapath detection for OS X
  // change CWD to point inside bundle so it finds its data (if necessary)
  char path[1024];
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  assert(mainBundle);
  CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
  assert(mainBundleURL);
  CFStringRef cfStringRef =
      CFURLCopyFileSystemPath(mainBundleURL, kCFURLPOSIXPathStyle);
  assert(cfStringRef);
  CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);
  CFRelease(mainBundleURL);
  CFRelease(cfStringRef);

  std::string contents = std::string(path) + std::string("/Contents");
  std::string datapath;

  if (contents.find(".app") != std::string::npos) {
    // executable is inside an app bundle, use app bundle-relative paths
    datapath = contents + std::string("/Resources/data/");
  } else if (contents.find("/bin") != std::string::npos) {
    // executable is installed Unix-way
    datapath = contents.substr(0, contents.find("/bin")) + "/share/smc";
  } else {
    std::cerr << "Warning: Could not determine installation type\n";
  }

  if (!datapath.empty()) {
    std::cout << "setting CWD to " << datapath.c_str() << std::endl;
    if (chdir(datapath.c_str()) != 0) {
      std::cerr << "Warning: Failed changing CWD\n";
    }
  }
#endif

  // convert arguments to a vector string
  vector<std::string> arguments(argv, argv + argc);

  if (argc >= 2) {
    for (unsigned int i = 1; i < arguments.size(); i++) {
      // help
      if (arguments[i] == "--help" || arguments[i] == "-h") {
        printf("Usage: %s [OPTIONS]\n", arguments[0].c_str());
        printf("Where OPTIONS is one of the following:\n");
        printf("-h, --help\tDisplay this message\n");
        printf("-v, --version\tShow the version of %s\n", CAPTION);
        printf(
            "-d, --debug\tEnable debug modes with the options : game performance\n");
        printf("-l, --level\tLoad the given level\n");
        printf("-w, --world\tLoad the given world\n");
        return EXIT_SUCCESS;
      }
      // version
      else if (arguments[i] == "--version" || arguments[i] == "-v") {
        printf("%s %d.%d.%d\n", CAPTION, SMC_VERSION_MAJOR, SMC_VERSION_MINOR,
               SMC_VERSION_PATCH);
        return EXIT_SUCCESS;
      }
      // debug
      else if (arguments[i] == "--debug" || arguments[i] == "-d") {
        // no value
        if (i + 1 >= arguments.size()) {
          printf("%s requires a value\n", arguments[i].c_str());
          return EXIT_FAILURE;
        }
        // with value
        else {
          // consume the following tokens as debug options (was `option = i + i`
          // with the wrong loop counter, so it re-read arguments[2*i] forever)
          for (unsigned int option = i + 1; option < arguments.size();
               option++) {
            std::string option_str = arguments[option];

            if (option_str == "game") {
              game_debug = true;
            } else if (option_str == "performance") {
              game_debug_performance = true;
            } else {
              printf("Unknown debug option %s\n", option_str.c_str());
              return EXIT_FAILURE;
            }
            // consume this token so the outer loop does not re-parse it
            i = option;
          }
        }
      }
      // level loading is handled later
      else if (arguments[i] == "--level" || arguments[i] == "-l") {
        // skip
      }
      // world loading is handled later
      else if (arguments[i] == "--world" || arguments[i] == "-w") {
        // skip
      }
      // unknown argument
      else if (arguments[i].substr(0, 1) == "-") {
        printf("Unknown argument %s\nUse -h to list all possible arguments\n",
               arguments[i].c_str());
        return EXIT_FAILURE;
      }
    }
  }

  try {
    // initialize everything
    Init_Game();
  } catch (const std::exception& e) {
    printf("Initialization: Exception raised: %s\n", e.what());
    return EXIT_FAILURE;
  }

  // command line level entering
  if (argc > 2 && (arguments[1] == "--level" || arguments[1] == "-l") &&
      !arguments[2].empty()) {
    Game_Action = GA_ENTER_LEVEL;
    Game_Mode_Type = MODE_TYPE_LEVEL_CUSTOM;
    Game_Action_Data_Middle.add("load_level", arguments[2]);
  }
  // command line world entering
  else if (argc > 2 && (arguments[1] == "--world" || arguments[1] == "-w") &&
           !arguments[2].empty()) {
    Game_Action = GA_ENTER_WORLD;
    Game_Action_Data_Middle.add("enter_world", arguments[2]);
  }
  // enter main menu
  else {
    Game_Action = GA_ENTER_MENU;
    Game_Action_Data_Middle.add("load_menu", int_to_string(MENU_MAIN));
  }

  Game_Action_Data_Start.add(
      "screen_fadeout", CEGUI::PropertyHelper::intToString(EFFECT_OUT_BLACK));
  Game_Action_Data_Start.add("screen_fadeout_speed", "3");
  Game_Action_Data_End.add("screen_fadein",
                           CEGUI::PropertyHelper::intToString(EFFECT_IN_BLACK));
  Game_Action_Data_End.add("screen_fadein_speed", "3");

  // game loop
  while (!game_exit) {
    // update
    Update_Game();
    // draw
    Draw_Game();

    // render
    pVideo->Render();

    // update speedfactor
    pFramerate->Update();
  }

  Exit_Game();
  return EXIT_SUCCESS;
}

// namespace is set here to exclude main() from it
namespace SMC {

void Init_Game(void) {
  // init random number generator
  srand(static_cast<unsigned int>(time(nullptr)));

  // the application object owns all the singletons; the pX globals are borrowed aliases into it
  g_app = std::make_unique<cApplication>();

  // Init Stage 1 - core classes
  g_app->m_resource_manager.Create();
  g_app->m_video.Create();
  g_app->m_audio.Create();
  g_app->m_font.Create();
  g_app->m_framerate.Create();
  g_app->m_renderer.Create(200);
  g_app->m_renderer_current.Create(200);
  g_app->m_preferences.Create();
  g_app->m_image_manager.Create();
  g_app->m_sound_manager.Create();
  g_app->m_settings_parser.Create();

  // Init Stage 2 - set preferences and init audio and the video screen
  /* Set default user directory
	 * can get overridden later from the preferences
	*/
  pResource_Manager->Set_User_Directory(Get_User_Directory());
  /* Initialize the fake CEGUI renderer and system for the pPreferences XMLParser,
	 * because CEGUI creates the system normally with the OpenGL-Renderer and OpenGL calls may 
	 * only be made with a valid OpenGL-context, which we would get only by setting 
	 * the videomode first. That would mean we need to init the videomode twice.
	 *
	 * The XMLParser can not be used without an initialized CEGUI::System because the XMLParser
	 * uses the CEGUI::System internally. Tested with CEGUI 0.7.4.
	*/
  pVideo->Init_CEGUI_Fake();
  // load user data
  pPreferences->Load();
  // set game language
  I18N_Set_Language(pPreferences->m_language);
  // init translation support
  I18N_Init();
  // delete CEGUI System fake
  pVideo->Delete_CEGUI_Fake();

  // init user dir directory
  pResource_Manager->Init_User_Directory();
  // video init
  pVideo->Init_SDL();
  pVideo->Init_Video();
  pVideo->Init_CEGUI();
  pVideo->Init_CEGUI_Data();
  pFont->Init();
  // framerate init ( must be after SDL init because of SDL_GetTicks() )
  pFramerate->Init();
  // audio init
  pAudio->Init();

  g_app->m_campaign_manager.Create();
  g_app->m_level_player.Create(nullptr);
  pLevel_Player->m_disallow_managed_delete = true;
  // set the first active player available
  pActive_Player = pLevel_Player;
  g_app->m_level_manager.Create();
  // set the first animation manager available
  pActive_Animation_Manager = &pActive_Level->m_animation_manager;
  // set the first active sprite manager available
  pLevel_Player->Set_Sprite_Manager(&pActive_Level->m_sprite_manager);

  // apply preferences
  pPreferences->Apply();

  // draw generic loading screen
  Loading_Screen_Init();
  // initialize image cache
  pVideo->Init_Image_Cache(false, true);

  // Init Stage 3 - game classes
  // note : set any sprite manager as it is set again on game mode switch
  g_app->m_hud_manager.Create(&pActive_Level->m_sprite_manager);
  pLevel_Player->Init();
  g_app->m_level_editor.Create(&pActive_Level->m_sprite_manager, pActive_Level);
  /* note : set any sprite manager as cOverworld_Manager::Load sets it again 
	 * parent overworld is also set from there again
	*/
  g_app->m_world_editor.Create(&pActive_Level->m_sprite_manager, nullptr);
  g_app->m_mouse_cursor.Create(&pActive_Level->m_sprite_manager);
  g_app->m_keyboard.Create();
  g_app->m_joystick.Create();
  pLevel_Manager->Init();
  // note : set any sprite manager as cOverworld_Manager::Load sets it again
  g_app->m_overworld_player.Create(&pActive_Level->m_sprite_manager, nullptr);
  g_app->m_overworld_manager.Create(&pActive_Level->m_sprite_manager);
  // set default overworld active
  pOverworld_Player->Set_Overworld(pOverworld_Manager->Get("World 1"));
  pOverworld_Manager->Set_Active("World 1");
  pHud_Manager->Load();
  g_app->m_menu_core.Create();
  g_app->m_savegame.Create();

  // cache
  Preload_Images(true);
  Preload_Sounds(true);
  Loading_Screen_Exit();
}

void Exit_Game(void) {
  if (pPreferences) {
    pPreferences->Save();
  }

  pLevel_Manager->Unload();
  pMenuCore->m_handler.m_level.Unload();

  // every singleton dies here, in cApplication's declaration order (the CEGUI system inside
  // ~cVideo); each alias is cleared as its object goes
  g_app.reset();

  const char* last_sdl_error = SDL_GetError();
  if (strlen(last_sdl_error) > 0) {
    printf("Last known SDL Error : %s\n", last_sdl_error);
  }

  // unload the sdl_image preloaded libraries
  IMG_Quit();

  SDL_Quit();
}

bool Handle_Input_Global(SDL_Event* ev) {
  switch (ev->type) {
    case SDL_QUIT: {
      game_exit = true;
      Clear_Input_Events();

      // handle on all handlers ?
      return false;
    }
    case SDL_WINDOWEVENT: {
      if (ev->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        pGuiSystem->notifyDisplaySizeChanged(
            CEGUI::Size(static_cast<float>(ev->window.data1),
                        static_cast<float>(ev->window.data2)));
      }
      // lost focus : wait (SDL 1.2 SDL_ACTIVEEVENT with gain == 0)
      else if (ev->window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        bool music_paused = false;
        // pause music
        if (pAudio->Is_Music_Playing()) {
          pAudio->Pause_Music();
          music_paused = true;
        }
        SDL_WaitEvent(nullptr);
        // resume if music got paused
        if (music_paused) {
          pAudio->Resume_Music();
        }
        return true;
      }
      break;
    }
    case SDL_TEXTINPUT: {
      if (pKeyboard->Text_Input(ev->text.text)) {
        return true;
      }
      break;
    }
    case SDL_KEYDOWN: {
      if (pKeyboard->Key_Down(ev->key.keysym.sym)) {
        return true;
      }
      break;
    }
    case SDL_KEYUP: {
      if (pKeyboard->Key_Up(ev->key.keysym.sym)) {
        return true;
      }
      break;
    }
    case SDL_JOYBUTTONDOWN: {
      if (pJoystick->Handle_Button_Down_Event(ev)) {
        return true;
      }
      break;
    }
    case SDL_JOYBUTTONUP: {
      if (pJoystick->Handle_Button_Up_Event(ev)) {
        return true;
      }
      break;
    }
    case SDL_JOYHATMOTION: {
      pJoystick->Handle_Hat(ev);
      break;
    }
    case SDL_JOYAXISMOTION: {
      pJoystick->Handle_Motion(ev);
      break;
    }
    default:  // other events
    {
      // mouse
      if (pMouseCursor->Handle_Event(ev)) {
        return true;
      }

      // send events
      if (Game_Mode == MODE_LEVEL) {
        // editor events
        if (pLevel_Editor->m_enabled) {
          if (pLevel_Editor->Handle_Event(ev)) {
            return true;
          }
        }
      } else if (Game_Mode == MODE_OVERWORLD) {
        // editor events
        if (pWorld_Editor->m_enabled) {
          if (pWorld_Editor->Handle_Event(ev)) {
            return true;
          }
        }
      } else if (Game_Mode == MODE_MENU) {
        if (pMenuCore->Handle_Event(ev)) {
          return true;
        }
      }
      break;
    }
  }

  return false;
}

void Update_Game(void) {
  // do not update if exiting
  if (game_exit) {
    return;
  }

  // if in menu and vsync is disabled then limit the fps to reduce the load for CPU/GPU
  if (Game_Mode == MODE_MENU && !pPreferences->m_video_vsync) {
    Correct_Frame_Time(100);
  }
  // if fps limit is set
  else if (pPreferences->m_video_fps_limit) {
    Correct_Frame_Time(pPreferences->m_video_fps_limit);
  }

  if (Game_Action != GA_NONE) {
    pVideo->Render_Finish();
  }

  // ## game events
  Handle_Game_Events();

  // ## input
  while (SDL_PollEvent(&input_event)) {
    // handle
    Handle_Input_Global(&input_event);
  }

  pMouseCursor->Update();

  // ## audio
  pAudio->Resume_Music();
  pAudio->Update();

  // performance measuring
  pFramerate->m_perf_last_ticks = SDL_GetTicks();

  // ## update
  if (Game_Mode == MODE_LEVEL) {
    pLevel_Manager->Update();
  } else if (Game_Mode == MODE_OVERWORLD) {
    pActive_Overworld->Update();
  } else if (Game_Mode == MODE_MENU) {
    pMenuCore->Update();
  } else if (Game_Mode == MODE_LEVEL_SETTINGS) {
    pLevel_Editor->m_settings_screen.Update();
  }

  // gui
  Gui_Handle_Time();
}

void Draw_Game(void) {
  // don't draw if exiting
  if (game_exit) {
    return;
  }

  // performance measuring
  pFramerate->m_perf_last_ticks = SDL_GetTicks();

  if (Game_Mode == MODE_LEVEL) {
    pLevel_Manager->Draw();
  } else if (Game_Mode == MODE_OVERWORLD) {
    pActive_Overworld->Draw();
  } else if (Game_Mode == MODE_MENU) {
    pMenuCore->Draw();
  } else if (Game_Mode == MODE_LEVEL_SETTINGS) {
    pLevel_Editor->m_settings_screen.Draw();
  }

  // Mouse
  pMouseCursor->Draw();

  // update performance timer
  pFramerate->m_perf_timer[PERF_DRAW_MOUSE].Update();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
