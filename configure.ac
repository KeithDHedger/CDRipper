#
#
AC_INIT([CDRipper],[0.5.0],[PROJ],[CDRipper])

AC_CONFIG_HEADER([config.h])
AC_PROG_CXX

AM_INIT_AUTOMAKE

AC_PROG_CC
AC_HEADER_STDC

#AC_SUBST(CFLAGS)
#AC_SUBST(CPPFLAGS)
#AC_SUBST(LDFLAGS)
#
#CFLAGS=-Wall
#AC_SUBST(CFLAGS)

GLIB_MODULES="glib-2.0 >= 2.30.0"
PKG_CHECK_MODULES(GLIB, $GLIB_MODULES)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

CDDB_MODULES="libcddb >= 0.9.5"
PKG_CHECK_MODULES(CDDB, $CDDB_MODULES)
AC_SUBST(CDDB_CFLAGS)
AC_SUBST(CDDB_LIBS)

GTK_MODULES="gtk+-2.0 >= 2.24.0"
PKG_CHECK_MODULES(GTK, $GTK_MODULES)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

GTHREAD_MODULES="gthread-2.0 >= 2.24.0"
PKG_CHECK_MODULES(GTHREAD, $GTHREAD_MODULES)
AC_SUBST(GTHREAD_CFLAGS)
AC_SUBST(GTHREAD_LIBS)

GDK_MODULES="gdk-2.0 >= 2.24.13"
PKG_CHECK_MODULES(GDK, $GDK_MODULES)
AC_SUBST(GDK_CFLAGS)
AC_SUBST(GDK_LIBS)

AC_CHECK_PROG(KUTE, kute ,true,false)
if test $KUTE = false; then
	AC_MSG_NOTICE([No kute program found!])
	AC_MSG_NOTICE([You can download it from here:])
	AC_MSG_ERROR([https://github.com/KeithDHedger/Kute])
fi

AC_CHECK_PROG(CDDA2WAV, cdda2wav ,true,false)
if test $CDDA2WAV = false; then
	AC_MSG_ERROR([No cdda2wav program found!])
fi

AC_CHECK_PROG(FLAC, flac ,true,false)
if test $FLAC = false; then
	AC_MSG_ERROR([No flac program found!])
fi

AC_CHECK_PROG(FFMPEG, ffmpeg ,true,false)
if test $FFMPEG = false; then
	AC_MSG_ERROR([No ffmpeg program found!])
fi

AC_CHECK_PROG(CURL, curl ,true,false)
if test $CURL = false; then
	AC_MSG_ERROR([No curl program found!])
fi

AC_OUTPUT(Makefile CDRipper/app/Makefile)

