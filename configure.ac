# Process this file with autoconf to produce a configure script.
AC_INIT([src/main.c])
AC_PREREQ(2.53)
AM_INIT_AUTOMAKE(ayttm, 0.3.0)
RELEASE=11

# This is to check modules/core binary compat
# Bump it up when necessary only (query_callbacks change, struct change etc)
CORE_VERSION=4

AM_CONFIG_HEADER(config.h)

if test "x$host_alias" = "xmingw32"; then
	# EB_CFLAGS="-mno-cygwin -fnative-struct -I/target/include -I/usr/include/mingw -I/usr/include/w32api -I/target/include/glib -I/target/include/glib-2.0 -I/usr/local/include -I/opt/include -I../ -I/usr/include/w32api -I/usr/include -I../mingw32 -D_REENTRANT -Wall -g"
	EB_LDADD="$LDADD"
	EB_LIBS="-Wl,-subsystem,windows -Wl,--export-dynamic /target/lib/intl.lib /target/lib/iconv.lib ../libltdl/libltdl.a /target/lib/dirent.lib -L/target/lib -lgdk -lgtk -lglib-2.0 -L../libproxy -lproxy -lwinmm -lws2_32 -lwst -lmsvcrt40"
fi

# Checks for programs.
AC_PROG_CC
AC_ISC_POSIX
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

# add some platform-specific flags
case "$host" in
*-darwin*)
        CFLAGS="$CFLAGS -no-cpp-precomp"
        ;;
esac

# Checks for libraries.
AC_CHECK_LIB([audiofile], [_af_ulaw2linear])
AC_CHECK_LIB([ltdl], [lt_dlopen])

# Checks for header files.
AC_PATH_XTRA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS([errno.h fcntl.h langinfo.h libintl.h limits.h locale.h stdlib.h string.h strings.h unistd.h iconv.h])
if test "x$host_alias" != "xmingw32"; then
AC_CHECK_HEADERS([alloca.h argz.h arpa/inet.h malloc.h netdb.h netinet/in.h paths.h sgtty.h stddef.h sys/file.h sys/ioctl.h sys/param.h sys/poll.h sys/socket.h sys/time.h syslog.h termios.h utmp.h getopt.h])
else
AC_CHECK_HEADERS([sys/utsname.h])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_STRUCT_TM
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_ALLOCA
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit bzero dcgettext dup2 getcwd gethostbyname gethostname gettimeofday getwd inet_ntoa isascii memchr memmove mempcpy memset mkdir modf munmap nl_langinfo pow select setlocale socket stpcpy strcasecmp strchr strcspn strdup strerror strncasecmp strpbrk strrchr strspn strstr strtol strtoul uname snprintf])
if test "x$host_alias" != "xmingw32"; then
AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify getopt_long getopt])
fi

ALL_LINGUAS="fr en_GB"
AM_GNU_GETTEXT

if test "x$host_alias" = "xmingw32"; then
	cat confdefs.h > T
	grep -v HAVE_ARGZ_H T > confdefs.h
fi

AC_MSG_CHECKING(how to include pthread)
AC_MSG_RESULT([])
AC_MSG_CHECKING(for -lpthread compiler option)
TMP_LIBS="$LIBS"
LIBS="$LIBS -lpthread"
AC_TRY_LINK_FUNC(pthread_create, [AC_MSG_RESULT(yes)
	have_l_pthread=yes], [AC_MSG_RESULT(no)
	have_l_pthread=no] )
if test "$have_l_pthread" = no; then
  AC_MSG_CHECKING(for -pthread compiler option)
  LIBS="$TMP_LIBS"
  TMP_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -pthread"

  AC_TRY_LINK_FUNC(pthread_create,
  	[AC_MSG_RESULT(yes)
	have_l_pthread=yes],
	[AC_MSG_RESULT(no)
	AC_MSG_ERROR(No pthread capability found)])
fi

AC_SEARCH_LIBS(libiconv_open, iconv)

AC_CHECK_PROG(have_ispell, ispell, yes, no)
if test $have_ispell = yes; then
  AC_DEFINE(HAVE_ISPELL, , Define if we have ispell)
fi

AC_ARG_ENABLE(esd, AC_HELP_STRING([--disable-esd],
		[compile without ESD soundserver]))

dnl want crash dialog
AC_SUBST(REQUIRES)
REQUIRES=
AC_ARG_ENABLE(crash-dialog, AC_HELP_STRING([--disable-crash-dialog],
		[Disable crash dialog]),
	[enable_crash_dialog=$enableval], [enable_crash_dialog=yes])
if test "$enable_crash_dialog" != "no"; then
dnl check if GDB is somewhere
	AC_CHECK_PROG(enable_crash_dialog, gdb, yes, no)
	AC_MSG_CHECKING([whether to use crash dialog])
	if test "$enable_crash_dialog" != "no"; then
		AC_DEFINE(CRASH_DIALOG, 1, Pop up crash dialog)
		REQUIRES="gdb"
	fi
	AC_MSG_RESULT($enable_crash_dialog)
fi

AC_MSG_CHECKING(whether we want ESD support)
AC_MSG_RESULT($enable_esd)
if test "$enable_esd" != no ; then
    AM_PATH_GESD
    if test "$no_esd" != yes ; then
        AC_DEFINE(ESD_SOUND, , Define if we use esd)
	EB_CFLAGS="$EB_CFLAGS $ESD_CFLAGS"
	EB_LIBS="$EB_LIBS $ESD_LIBS"
    fi
fi

AM_PATH_GARTS
if test "$arts" = yes ; then
    AC_DEFINE(ARTS_SOUND, , Define if we use arts)
    EB_CFLAGS="$EB_CFLAGS $ARTS_CFLAGS"
    EB_LIBS="$EB_LIBS $ARTS_LIBS"
fi

# Hack it out of mingw32
if test "x$host_alias" != "xmingw32"; then
AM_PATH_GLIB(1.2.0, [AC_DEFINE(HAVE_GLIB, 1, [Define if you have glib])],
	AC_MSG_ERROR([Unable to find glib-devel version 1.2.0 or higher]))
AM_PATH_GTK(1.2.0, [AC_DEFINE(HAVE_GTK, 1, [Define if you have gtk])],
	AC_MSG_ERROR([Unable to find gtk-devel version 1.2.0 or higher]))

AC_ARG_ENABLE(gdk_pixbuf, AC_HELP_STRING([--disable-gdk-pixbuf],
		[Do not use gdk-pix-buf (webcam support)]))
if test "$enable_gdk_pixbuf" != "no"; then
    AM_PATH_GDK_PIXBUF(, [AC_DEFINE(HAVE_GDK_PIXBUF, 1, [Define if you have gdk-pixbuf])],
        [AC_MSG_WARN([Unable to find gdk-pixbuf-devel, you won't be able to view webcams])])
fi
else
 	GTK_CFLAGS="-mno-cygwin -fnative-struct -I/target/include -I/usr/include/mingw -I/usr/include/w32api -I/target/include/glib -I/target/include/glib-2.0 -I/usr/local/include -I/opt/include -I../ -I/usr/include/w32api -I/usr/include -D_REENTRANT -Wall -g -I../mingw32"
fi

EB_CFLAGS="$EB_CFLAGS $GLIB_CFLAGS $GTK_CFLAGS $GDK_PIXBUF_CFLAGS"
EB_LIBS="$EB_LIBS $GLIB_LIBS $GTK_LIBS $GDK_PIXBUF_LIBS"

AC_ARG_ENABLE(japanese, AC_HELP_STRING([--enable-japanese],
		[compile with Japanese support]))
if test "$enable_japanese" = "yes"; then
    AC_DEFINE(JAPANESE, , Define if we compile for japanese support)
fi


###############################################################################
#
#       Check for the MIT-SCREEN-SAVER server extension.
#  
###############################################################################

have_mit=no
with_mit_req=unspecified
      
AC_CHECK_HEADER(X11/extensions/scrnsaver.h, [have_mit=yes])
      
# Now check to see if it's really in the library; XF86Free-3.3 ships  
# scrnsaver.h, but doesn't include the code in libXext.a, the idiots!
#
if test "$have_mit" = yes; then
  AC_CHECK_LIB(Xext, XScreenSaverRegister, [true], [have_mit=no], -L/usr/X11R6/lib -lX11)

  if test "$have_mit" = no; then
    # Looks like XF86Free-3.3 actually puts it in XExExt instead
    # of in Xext.
    AC_CHECK_LIB(XExExt, XScreenSaverRegister,
                   [have_mit=yes; EB_LIBS="$EB_LIBS -lXExExt"],
                   [true], -L/usr/X11R6/lib -lX11 -lXext)
  fi

  if test "$have_mit" = no; then
    # Looks like some versions of XFree86 (whichever version
    # it is that comes with RedHat Linux 2.0 -- I can't find a version
    # number) put this garbage in Xss instead of Xext.
    AC_CHECK_LIB(Xss, XScreenSaverRegister,
                   [have_mit=yes; EB_LIBS="$EB_LIBS -lXss"],
                   [true], -L/usr/X11R6/lib -lX11 -lXext)
  fi
      
  if test "$have_mit" = yes; then
    AC_DEFINE(HAVE_MIT_SAVER_EXTENSION, , Define if we have mit screensaver server extensions)
  fi
      
fi


###########################################################################
# Check needed for the Xft    stuff                                       #
###########################################################################
have_xft=no
AC_ARG_ENABLE(xft, AC_HELP_STRING([--enable-xft],
		[compile with anti-aliased font support]))
if test "$enable_xft" = yes ; then
    XFT_CFLAGS=`xft-config --cflags`
    XFT_LDFLAGS=`xft-config --libs`
    CFLAGS="$CFLAGS $XFT_CFLAGS"
    LDFLAGS="$LDFLAGS $XFT_LDFLAGS"
    AC_CHECK_HEADER(X11/Xft/Xft.h, [
	AC_CHECK_LIB(Xft, XftFontOpen,have_xft=yes;[EB_LIBS="$EB_LIBS $XFT_LDFLAGS"],, -L/usr/X11R6/lib -lX11)
    ])
  if test "$have_xft" = yes; then
    AC_DEFINE(HAVE_LIBXFT, , Define if we are to use AA)
  fi
fi

###########################################################################
# Check needed for the Jabber stuff                                       #
###########################################################################

AC_CHECK_LIB(nsl, gethostent)
if test "x$host_alias" != "xmingw32" ; then
AC_CHECK_FUNC(connect,AC_DEFINE(HAVE_LIBSOCKET,, Define if we have libsocket),[
      AC_CHECK_LIB(socket, connect, , AC_MSG_ERROR([
	  *************************************************************
	  *  You do not seem to have the socket call available as a   *
	  *  function or in the socket library.  This problem must be *
	  *  fixed before you can continue.                           *
	  *************************************************************]))])
fi


### Begin Optional Service related options ######################################

AC_ARG_ENABLE(oscar, AC_HELP_STRING([--enable-oscar],
		[include oscar service]))
AC_ARG_ENABLE(workwizu, AC_HELP_STRING([--enable-workwizu],
		[include workwizu service]))
AC_ARG_ENABLE(broken_icq, AC_HELP_STRING([--enable-broken-icq],
		[include broken icq service instead of working one]))
AC_ARG_ENABLE(aim_toc, AC_HELP_STRING([--disable-aim-toc],
		[exclude aim-toc service]))
AC_ARG_ENABLE(icq_toc, AC_HELP_STRING([--disable-icq-toc],
		[exclude icq-toc service]))
AC_ARG_ENABLE(icq, AC_HELP_STRING([--disable-icq],
		[exclude icq service]))
AC_ARG_ENABLE(irc, AC_HELP_STRING([--disable-irc],
		[exclude irc service]))
AC_ARG_ENABLE(jabber, AC_HELP_STRING([--disable-jabber],
		[exclude jabber service]))
AC_ARG_ENABLE(msn, AC_HELP_STRING([--disable-msn],
		[exclude msn service]))
AC_ARG_ENABLE(libmsn2_debug, AC_HELP_STRING([--enable-libmsn2-debug],
		[enable debugging of libmsn2]))
AC_ARG_ENABLE(yahoo, AC_HELP_STRING([--disable-yahoo],
		[exclude yahoo service]))

OPTIONAL_PLUG_DIR=""
DEBUG_LIBMSN=""
if test "$enable_oscar" == yes; then
 	OPTIONAL_PLUG_DIR="$OPTIONAL_PLUG_DIR aim-oscar"
fi
if test "$enable_msn" != no; then
	OPTIONAL_PLUG_DIR="$OPTIONAL_PLUG_DIR msn2"
fi
if test "$enable_workwizu" = yes; then
	have_gmp=no;
	OPTIONAL_PLUG_DIR="$OPTIONAL_PLUG_DIR workwizu"
	GMP_LDFLAGS="-lgmp"
	LDFLAGS="$LDFLAGS $GMP_LDFLAGS"
	AC_CHECK_HEADER(gmp.h, [
	AC_CHECK_LIB(gmp, __gmpz_init,have_gmp=yes;[EB_LIBS="$EB_LIBS $GMP_LDFLAGS"],, -lgmp)])
	if test "$have_gmp" = no; then
	 echo "You need gmp for workwizu"
	 echo "http://www.swox.com/gmp/"
	 exit
	fi;
fi
if test "$enable_libmsn2_debug" = yes; then
	DEBUG_LIBMSN="-DDEBUG=1"
fi
if test "$enable_yahoo" != no; then
	OPTIONAL_PLUG_DIR="$OPTIONAL_PLUG_DIR yahoo2"
fi
if test "$enable_irc" != no; then
	OPTIONAL_PLUG_DIR="$OPTIONAL_PLUG_DIR irc"
fi
if test "$enable_jabber" != no; then
	OPTIONAL_PLUG_DIR="$OPTIONAL_PLUG_DIR jabber"
fi
if test "$enable_broken_icq" == yes; then
	OPTIONAL_PLUG_DIR="$OPTIONAL_PLUG_DIR icq"
fi
if test "$enable_aim_toc" != no; then
	OPTIONAL_PLUG_DIR="$OPTIONAL_PLUG_DIR aim-toc"
fi
if test "$enable_icq_toc" != no; then
	OPTIONAL_PLUG_DIR="$OPTIONAL_PLUG_DIR icq-toc"
fi

### End Optional Service  options ######################################

### Begin Zephyr-related options ######################################

with_krb4_dir=/usr/kerberos

AC_ARG_ENABLE(zephyr, AC_HELP_STRING([--enable-zephyr],
		[include Zephyr service]))
AC_ARG_ENABLE(krb4, AC_HELP_STRING([--disable-krb4],
		[disable KerberosIV in Zephyr]))
AC_ARG_WITH(krb4-dir, AC_HELP_STRING([--with-krb4-dir=dir],
		[specify top dir of KerberosIV installation (default: /usr/kerberos]))

#echo "DEBUG: zephyr_val = $zephyr_val"
if test "$enable_zephyr" = yes; then
  AC_MSG_CHECKING(support for Zephyr)
#  echo "DEBUG: krb4_val = $krb4_val"
#  echo "DEBUG: krb4_dir = $krb4_dir"
  OPTIONAL_PLUG_DIR="$OPTIONAL_PLUG_DIR zephyr"
  ZEPHYR_CFLAGS=""
  ZEPHYR_LIBS=""
  if test "$enable_krb4" = yes; then
    ZEPHYR_CFLAGS="$ZEPHYR_CFLAGS -I$krb4_dir/include -I$krb4_dir/include/kerberosIV"
    ZEPHYR_LIBS="$ZEPHYR_LIBS -L$krb4_dir/lib"
  fi
  AC_SUBST(ZEPHYR_CFLAGS)
  AC_SUBST(ZEPHYR_LIBS)
  if test "$enable_krb4" = yes; then
    AC_CHECK_HEADERS(krb.h krb_err.h)
    AC_CHECK_LIB(krb4, krb_mk_req, [ZEPHYR_LIBS="$ZEPHYR_LIBS -lkrb4"
	AC_DEFINE(HAVE_KRB4, , Define if we use kerberos 4 for Zephyr)], 
	[AC_MSG_WARN(
	[*** krb4 not found -- Zephyr service will be unauthenticated ***])])
  fi
fi

AC_ARG_ENABLE(all-warnings, AC_HELP_STRING([--enable-all-warnings],
		[enable all compiler warnings - for developers]))

if test "$enable_all_warnings" = yes; then
	CFLAGS="$CFLAGS -Wall"
	CXXFLAGS="$CXXFLAGS -Wall"
fi

CFLAGS="$CFLAGS -I\$(top_srcdir)/src -DCORE_VERSION=$CORE_VERSION"
CXXFLAGS="$CXXFLAGS -DCORE_VERSION=$CORE_VERSION"


AC_SUBST_FILE(CHANGELOG)
CHANGELOG="ChangeLog"
AC_SUBST(RELEASE)
AC_DEFINE_UNQUOTED(RELEASE, "$RELEASE", [Release number of package])

AC_SUBST(EB_CFLAGS)
AC_SUBST(EB_LIBS)
AC_SUBST(EB_LDFLAGS)
AC_SUBST(OPTIONAL_PLUG_DIR)
AC_SUBST(DEBUG_LIBMSN)

AC_CONFIG_FILES([Makefile ayttm.spec
                 doc/Makefile
                 intl/Makefile
                 libproxy/Makefile
                 po/Makefile.in
                 m4/Makefile 
		 modules/Makefile
                 modules/aim-toc/Makefile
                 modules/aim-toc/libtoc/Makefile
                 modules/icq-toc/Makefile
                 modules/icq-toc/libtoc/Makefile
                 modules/aim-oscar/Makefile
                 modules/aim-oscar/libfaim/Makefile
                 modules/icq/Makefile
                 modules/icq/libicq/Makefile
                 modules/importers/Makefile
                 modules/irc/Makefile
                 modules/jabber/Makefile
                 modules/jabber/libjabber/Makefile
                 modules/jabber/libjabber/include/Makefile
                 modules/jabber/libjabber/include/jabber/Makefile
                 modules/jabber/libxode/Makefile
                 modules/jabber/libxode/include/Makefile
                 modules/msn2/Makefile
                 modules/msn2/libmsn2/Makefile
                 modules/utility/Makefile
                 modules/yahoo2/Makefile
                 modules/yahoo2/libyahoo2/Makefile
                 modules/workwizu/Makefile
                 pixmaps/Makefile
                 sounds/Makefile
                 src/Makefile
		 src/gtk/Makefile
		 mingw32/Makefile])

if test "x$host_alias" = "xmingw32" ; then
	echo "editing src/Makefile final time"
        cat src/Makefile | sed 's/^LIBS = .*/LIBS = \$(EB_LIBS)/' > .ACE
	cp -f .ACE src/Makefile
fi

AC_OUTPUT
