AC_PREREQ(2.57)
AC_INIT(smc, 1.2)
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

AC_DEFINE_UNQUOTED(DATA_DIR, "${packagedatadir}", [data directory])

AC_CONFIG_FILES([Makefile src/Makefile data/Makefile])
AC_OUTPUT

echo
echo "Install paths:"
echo "  prefix:  `eval echo ${prefix}`"
echo "  bindir:  `eval echo ${bindir}`"
echo "  datadir: `eval echo ${datadir}`"
echo
