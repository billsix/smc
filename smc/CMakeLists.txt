# CMake build for Secret Maryo Chronicles.
#
#   cmake -S smc -B <build> -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
#   cmake --build <build> && cmake --install <build>
#
# The container builds with entrypoint/build.sh (build tree /opt/smc-build,
# outside the bind-mounted repo).
#
# Dependencies come from pkg-config / CMake packages: SDL2 (+ image, mixer, ttf),
# CEGUI 0.7 (the vendored third_party/cegui-0.7.9, built and installed first by
# entrypoint/build.sh, found through PKG_CONFIG_PATH), OpenGL/GLU, X11, gettext.
# Third-party headers arrive through imported targets, so CMake passes them as
# -isystem and their warnings never trip -Werror.
cmake_minimum_required(VERSION 3.25)
project(smc VERSION 2.0 LANGUAGES CXX)

include(GNUInstallDirs)

option(SMC_RUN_IN_PLACE "Run from the source tree: data/ is found relative to the working directory" OFF)
option(SMC_DEBUG "Debug build of the game logic (defines _DEBUG=1)" OFF)
option(SMC_WERROR "Treat warnings as errors (the project's warning gate)" ON)

# Default to an optimized build with debug info, keeping assert() live (no
# -DNDEBUG): -O2 -g, the flags the game has always been built with.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2>=2.0.0 SDL2_image SDL2_mixer SDL2_ttf)
pkg_check_modules(CEGUI REQUIRED IMPORTED_TARGET CEGUI-OPENGL>=0.7.2)
find_library(CEGUI_NULL_RENDERER CEGUINullRenderer HINTS ${CEGUI_LIBRARY_DIRS} REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL GLX)
find_package(X11 REQUIRED)
find_package(Intl REQUIRED)

add_executable(smc
  src/audio/audio.cpp
  src/audio/random_sound.cpp
  src/audio/sound_manager.cpp
  src/core/camera.cpp
  src/core/campaign_manager.cpp
  src/core/collision.cpp
  src/core/editor.cpp
  src/core/file_parser.cpp
  src/core/filesystem/filesystem.cpp
  src/core/filesystem/resource_manager.cpp
  src/core/framerate.cpp
  src/core/game_core.cpp
  src/core/i18n.cpp
  src/core/main.cpp
  src/core/math/utilities.cpp
  src/core/math/vector.cpp
  src/core/property_helper.cpp
  src/core/sprite_manager.cpp
  src/enemies/bosses/turtle_boss.cpp
  src/enemies/eato.cpp
  src/enemies/enemy.cpp
  src/enemies/flyon.cpp
  src/enemies/furball.cpp
  src/enemies/gee.cpp
  src/enemies/krush.cpp
  src/enemies/rokko.cpp
  src/enemies/spika.cpp
  src/enemies/spikeball.cpp
  src/enemies/static.cpp
  src/enemies/thromp.cpp
  src/enemies/turtle.cpp
  src/gui/generic.cpp
  src/gui/hud.cpp
  src/gui/menu.cpp
  src/gui/menu_data.cpp
  src/gui/spinner.cpp
  src/input/joystick.cpp
  src/input/keyboard.cpp
  src/input/mouse.cpp
  src/level/level_background.cpp
  src/level/level.cpp
  src/level/level_editor.cpp
  src/level/level_manager.cpp
  src/level/level_player.cpp
  src/level/level_settings.cpp
  src/objects/animated_sprite.cpp
  src/objects/ball.cpp
  src/objects/bonusbox.cpp
  src/objects/box.cpp
  src/objects/enemystopper.cpp
  src/objects/goldpiece.cpp
  src/objects/level_entry.cpp
  src/objects/level_exit.cpp
  src/objects/moving_platform.cpp
  src/objects/movingsprite.cpp
  src/objects/path.cpp
  src/objects/powerup.cpp
  src/objects/spinbox.cpp
  src/objects/sprite.cpp
  src/objects/star.cpp
  src/objects/text_box.cpp
  src/overworld/overworld.cpp
  src/overworld/world_editor.cpp
  src/overworld/world_layer.cpp
  src/overworld/world_manager.cpp
  src/overworld/world_player.cpp
  src/overworld/world_sprite_manager.cpp
  src/overworld/world_waypoint.cpp
  src/user/preferences.cpp
  src/user/savegame.cpp
  src/video/animation.cpp
  src/video/font.cpp
  src/video/gl_surface.cpp
  src/video/img_manager.cpp
  src/video/img_settings.cpp
  src/video/renderer.cpp
  src/video/video.cpp
)

target_link_libraries(smc PRIVATE
  PkgConfig::SDL2
  PkgConfig::CEGUI
  ${CEGUI_NULL_RENDERER}
  OpenGL::GL
  OpenGL::GLU
  OpenGL::GLX
  X11::X11
  Intl::Intl
)

# The raised warning set the project gates on (gcc and clang).
target_compile_options(smc PRIVATE
  -Wall -Wextra -Wpedantic -Wshadow -Wvla -Wformat=2 -Wsign-conversion
  -Wno-unused-parameter
  $<$<BOOL:${SMC_WERROR}>:-Werror>
)

if(SMC_DEBUG)
  target_compile_definitions(smc PRIVATE _DEBUG=1)
endif()

# Installed builds look for their data in <datadir>/smc; a run-in-place build
# keeps the source default ("data", relative to the working directory).
set(SMC_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/smc")
if(NOT SMC_RUN_IN_PLACE)
  target_compile_definitions(smc PRIVATE "DATA_DIR=\"${SMC_DATA_DIR}\"")
endif()

install(TARGETS smc RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY
    data/campaign data/editor data/gui data/icon data/levels data/music
    data/pixmaps data/schema data/sounds data/translations data/world
  DESTINATION ${CMAKE_INSTALL_DATADIR}/smc
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
