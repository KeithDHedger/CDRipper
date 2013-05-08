
include ../flagsandlibs
include ../sources

cdripper_SOURCES = $(COMMONSRC)

cdripper_CFLAGS = -O3

pixfilesdir = $(pkgdatadir)/pixmaps
pixfiles_DATA = ../resources/pixmaps/*

scriptfilesdir = $(pkgdatadir)/scripts
scriptfiles_SCRIPTS = ../resources/scripts/*

man1_MANS = ../resources/man/cdripper.1

bin_PROGRAMS = cdripper

install-exec-hook:
	cp ../resources/pixmaps/CDRipper.desktop /usr/share/applications/CDRipper.desktop
	cp ../resources/pixmaps/CDRipper.png /usr/share/pixmaps
	echo Icon=${prefix}/share/CDRipper/pixmaps/CDRipper.png >> /usr/share/applications/CDRipper.desktop
	echo Exec=${prefix}/bin/cdripper >> /usr/share/applications/CDRipper.desktop

uninstall-hook:
	rm -f /usr/share/applications/CDRipper.desktop /usr/share/pixmaps/CDRipper.png

	rm -fr $(pkgdatadir)
