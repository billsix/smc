AC_PREREQ(2.57)
AC_INIT(smc, 0.99.7)
AC_CANONICAL_TARGET
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE(foreign)

# Check for programs
AC_PROG_CXX
AC_LANG(C++)

# Check for Boost Filesystem
AC_CHECK_LIB(boost_filesystem, main, ,
	AC_MSG_ERROR([Unable to find Boost Filesystem library]))

# Check for OpenGL, GLU
AC_CHECK_LIB(GL, main, , has_gl=no)
AC_CHECK_LIB(GLU, main, , has_glu=no)
if test "x$has_gl" = "xno" -o "x$has_glu" = "xno"; then
	AC_MSG_ERROR(Unable to find the OpenGL library)
fi

# Check for SDL
SDL_VERSION=1.2.10
AM_PATH_SDL($SDL_VERSION, ,
	AC_MSG_ERROR([Unable to find SDL $SDL_VERSION]))
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

# Check for SDL_image library
AC_CHECK_LIB(SDL_image, IMG_LoadPNG_RW, ,
	AC_MSG_ERROR([Unable to find the SDL_image library with PNG support]))

# Check for SDL_mixer library
AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio, ,
	AC_MSG_ERROR([Unable to find the SDL_mixer library]))

# Check for SDL_ttf library
AC_CHECK_LIB(SDL_ttf, TTF_Init, ,
	AC_MSG_ERROR([Unable to find the SDL_ttf library]))

# Check for CEGUI
PKG_CHECK_MODULES(CEGUI, CEGUI-OPENGL >= 0.5.0)
CPPFLAGS="$CPPFLAGS $CEGUI_CFLAGS"
LIBS="$LIBS $CEGUI_LIBS"

# Set paths
test "x${prefix}" = "xNONE" && prefix=${ac_default_prefix}
test "x${exec_prefix}" = "xNONE" && exec_prefix=${prefix}
test "x${datadir}" = 'x${datarootdir}' && datadir=${prefix}/share
packagedatadir=${datadir}/${PACKAGE}

AC_DEFINE_UNQUOTED(SAVE_DIR, "savegames", [savegames directory])
AC_DEFINE_UNQUOTED(SCREENSHOT_DIR, "screenshots", [screenshots directory])
AC_DEFINE_UNQUOTED(OVERWORLD_DIR, "${packagedatadir}/world", [overworld directory])
AC_DEFINE_UNQUOTED(LEVEL_DIR, "${packagedatadir}/levels", [level directory])
AC_DEFINE_UNQUOTED(PIXMAPS_DIR, "${packagedatadir}/pixmaps", [pixmaps directory])
AC_DEFINE_UNQUOTED(SOUNDS_DIR, "${packagedatadir}/sounds", [sounds directory])
AC_DEFINE_UNQUOTED(MUSIC_DIR, "${packagedatadir}/music", [music directory])
AC_DEFINE_UNQUOTED(EDITOR_DIR, "${packagedatadir}/editor", [editor directory])
AC_DEFINE_UNQUOTED(ICON_DIR, "${packagedatadir}/icon", [icon directory])
AC_DEFINE_UNQUOTED(SCHEMA_DIR, "${packagedatadir}/schema", [schema directory])
AC_DEFINE_UNQUOTED(GUI_SCHEME_DIR, "${packagedatadir}/gui/schemes", [GUI scheme directory])
AC_DEFINE_UNQUOTED(GUI_IMAGESET_DIR, "${packagedatadir}/gui/imagesets", [GUI imageset directory])
AC_DEFINE_UNQUOTED(GUI_FONT_DIR, "${packagedatadir}/gui/font", [GUI font directory])
AC_DEFINE_UNQUOTED(GUI_LAYOUT_DIR, "${packagedatadir}/gui/layout", [GUI layout directory])
AC_DEFINE_UNQUOTED(GUI_LOOKNFEEL_DIR, "${packagedatadir}/gui/looknfeel", [GUI looknfeel directory])

AC_CONFIG_FILES([Makefile src/Makefile data/Makefile])
AC_OUTPUT

echo
echo "Install paths:"
echo "  prefix:  `eval echo ${prefix}`"
echo "  bindir:  `eval echo ${bindir}`"
echo "  datadir: `eval echo ${datadir}`"
echo
