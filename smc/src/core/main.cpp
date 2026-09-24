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
// Owns every process-global singleton for the whole run, so each is freed exactly once and in one
// place (fixing the old hand-ordered Exit_Game -- including the cFramerate that used to leak). The
// extern cX* pX globals stay as borrowed aliases into these members, set in Init_Game and cleared
// in Exit_Game; the teardown order in Exit_Game is preserved from the original for now.
class cApplication {
 public:
  std::unique_ptr<cResource_Manager> m_resource_manager;
  std::unique_ptr<cVideo> m_video;
  std::unique_ptr<cAudio> m_audio;
  std::unique_ptr<cFont_Manager> m_font;
  std::unique_ptr<cFramerate> m_framerate;
  std::unique_ptr<cRenderQueue> m_renderer;
  std::unique_ptr<cRenderQueue> m_renderer_current;
  std::unique_ptr<cPreferences> m_preferences;
  std::unique_ptr<cImage_Manager> m_image_manager;
  std::unique_ptr<cSound_Manager> m_sound_manager;
  std::unique_ptr<cImage_Settings_Parser> m_settings_parser;
  std::unique_ptr<cCampaign_Manager> m_campaign_manager;
  std::unique_ptr<cLevel_Player> m_level_player;
  std::unique_ptr<cLevel_Manager> m_level_manager;
  std::unique_ptr<cHud_Manager> m_hud_manager;
  std::unique_ptr<cEditor_Level> m_level_editor;
  std::unique_ptr<cEditor_World> m_world_editor;
  std::unique_ptr<cMouseCursor> m_mouse_cursor;
  std::unique_ptr<cKeyboard> m_keyboard;
  std::unique_ptr<cJoystick> m_joystick;
  std::unique_ptr<cOverworld_Player> m_overworld_player;
  std::unique_ptr<cOverworld_Manager> m_overworld_manager;
  std::unique_ptr<cMenuCore> m_menu_core;
  std::unique_ptr<cSavegame> m_savegame;
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
  g_app->m_resource_manager = std::make_unique<cResource_Manager>();
  pResource_Manager = g_app->m_resource_manager.get();
  g_app->m_video = std::make_unique<cVideo>();
  pVideo = g_app->m_video.get();
  g_app->m_audio = std::make_unique<cAudio>();
  pAudio = g_app->m_audio.get();
  g_app->m_font = std::make_unique<cFont_Manager>();
  pFont = g_app->m_font.get();
  g_app->m_framerate = std::make_unique<cFramerate>();
  pFramerate = g_app->m_framerate.get();
  g_app->m_renderer = std::make_unique<cRenderQueue>(200);
  pRenderer = g_app->m_renderer.get();
  g_app->m_renderer_current = std::make_unique<cRenderQueue>(200);
  pRenderer_current = g_app->m_renderer_current.get();
  g_app->m_preferences = std::make_unique<cPreferences>();
  pPreferences = g_app->m_preferences.get();
  g_app->m_image_manager = std::make_unique<cImage_Manager>();
  pImage_Manager = g_app->m_image_manager.get();
  g_app->m_sound_manager = std::make_unique<cSound_Manager>();
  pSound_Manager = g_app->m_sound_manager.get();
  g_app->m_settings_parser = std::make_unique<cImage_Settings_Parser>();
  pSettingsParser = g_app->m_settings_parser.get();

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

  g_app->m_campaign_manager = std::make_unique<cCampaign_Manager>();
  pCampaign_Manager = g_app->m_campaign_manager.get();
  g_app->m_level_player = std::make_unique<cLevel_Player>(nullptr);
  pLevel_Player = g_app->m_level_player.get();
  pLevel_Player->m_disallow_managed_delete = true;
  // set the first active player available
  pActive_Player = pLevel_Player;
  g_app->m_level_manager = std::make_unique<cLevel_Manager>();
  pLevel_Manager = g_app->m_level_manager.get();
  // set the first animation manager available
  pActive_Animation_Manager = pActive_Level->m_animation_manager;
  // set the first active sprite manager available
  pLevel_Player->Set_Sprite_Manager(pActive_Level->m_sprite_manager);

  // apply preferences
  pPreferences->Apply();

  // draw generic loading screen
  Loading_Screen_Init();
  // initialize image cache
  pVideo->Init_Image_Cache(false, true);

  // Init Stage 3 - game classes
  // note : set any sprite manager as it is set again on game mode switch
  g_app->m_hud_manager =
      std::make_unique<cHud_Manager>(pActive_Level->m_sprite_manager);
  pHud_Manager = g_app->m_hud_manager.get();
  pLevel_Player->Init();
  g_app->m_level_editor = std::make_unique<cEditor_Level>(
      pActive_Level->m_sprite_manager, pActive_Level);
  pLevel_Editor = g_app->m_level_editor.get();
  /* note : set any sprite manager as cOverworld_Manager::Load sets it again 
	 * parent overworld is also set from there again
	*/
  g_app->m_world_editor =
      std::make_unique<cEditor_World>(pActive_Level->m_sprite_manager, nullptr);
  pWorld_Editor = g_app->m_world_editor.get();
  g_app->m_mouse_cursor =
      std::make_unique<cMouseCursor>(pActive_Level->m_sprite_manager);
  pMouseCursor = g_app->m_mouse_cursor.get();
  g_app->m_keyboard = std::make_unique<cKeyboard>();
  pKeyboard = g_app->m_keyboard.get();
  g_app->m_joystick = std::make_unique<cJoystick>();
  pJoystick = g_app->m_joystick.get();
  pLevel_Manager->Init();
  // note : set any sprite manager as cOverworld_Manager::Load sets it again
  g_app->m_overworld_player = std::make_unique<cOverworld_Player>(
      pActive_Level->m_sprite_manager, nullptr);
  pOverworld_Player = g_app->m_overworld_player.get();
  g_app->m_overworld_manager =
      std::make_unique<cOverworld_Manager>(pActive_Level->m_sprite_manager);
  pOverworld_Manager = g_app->m_overworld_manager.get();
  // set default overworld active
  pOverworld_Player->Set_Overworld(pOverworld_Manager->Get("World 1"));
  pOverworld_Manager->Set_Active("World 1");
  pHud_Manager->Load();
  g_app->m_menu_core = std::make_unique<cMenuCore>();
  pMenuCore = g_app->m_menu_core.get();
  g_app->m_savegame = std::make_unique<cSavegame>();
  pSavegame = g_app->m_savegame.get();

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
  pMenuCore->m_handler->m_level->Unload();

  g_app->m_audio.reset();
  pAudio = nullptr;

  g_app->m_level_player.reset();
  pLevel_Player = nullptr;

  g_app->m_hud_manager.reset();
  pHud_Manager = nullptr;

  g_app->m_sound_manager.reset();
  pSound_Manager = nullptr;

  g_app->m_level_editor.reset();
  pLevel_Editor = nullptr;

  g_app->m_world_editor.reset();
  pWorld_Editor = nullptr;

  g_app->m_preferences.reset();
  pPreferences = nullptr;

  g_app->m_savegame.reset();
  pSavegame = nullptr;

  g_app->m_mouse_cursor.reset();
  pMouseCursor = nullptr;

  g_app->m_joystick.reset();
  pJoystick = nullptr;

  g_app->m_keyboard.reset();
  pKeyboard = nullptr;

  g_app->m_campaign_manager.reset();
  pCampaign_Manager = nullptr;

  g_app->m_overworld_manager.reset();
  pOverworld_Manager = nullptr;

  g_app->m_overworld_player.reset();
  pOverworld_Player = nullptr;

  g_app->m_level_manager.reset();
  pLevel_Manager = nullptr;

  g_app->m_menu_core.reset();
  pMenuCore = nullptr;

  g_app->m_renderer.reset();
  pRenderer = nullptr;

  g_app->m_renderer_current.reset();
  pRenderer_current = nullptr;
  g_app->m_framerate.reset();
  pFramerate = nullptr;

  if (pGuiSystem) {
    CEGUI::ResourceProvider* rp = pGuiSystem->getResourceProvider();
    CEGUI::Logger* logger = CEGUI::Logger::getSingletonPtr();
    pGuiSystem->destroy();
    pGuiSystem = nullptr;
    delete rp;
    delete logger;
  }

  if (pGuiRenderer) {
    pGuiRenderer->destroy(*pGuiRenderer);
    pGuiRenderer = nullptr;
  }

  g_app->m_video.reset();
  pVideo = nullptr;

  g_app->m_image_manager.reset();
  pImage_Manager = nullptr;

  g_app->m_settings_parser.reset();
  pSettingsParser = nullptr;

  g_app->m_font.reset();
  pFont = nullptr;

  g_app->m_resource_manager.reset();
  pResource_Manager = nullptr;

  const char* last_sdl_error = SDL_GetError();
  if (strlen(last_sdl_error) > 0) {
    printf("Last known SDL Error : %s\n", last_sdl_error);
  }

  // free the (now empty) application object
  g_app.reset();

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
    pLevel_Editor->m_settings_screen->Update();
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
    pLevel_Editor->m_settings_screen->Draw();
  }

  // Mouse
  pMouseCursor->Draw();

  // update performance timer
  pFramerate->m_perf_timer[PERF_DRAW_MOUSE]->Update();
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

}  // namespace SMC
