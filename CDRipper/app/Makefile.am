AUTOMAKE_OPTIONS = subdir-objects

include ../flagsandlibs
include ../sources

cdripper_SOURCES = $(COMMONSRC)

cdripper_CFLAGS = -O3

bin_PROGRAMS = cdripper

install-exec-hook:
	install -D ../resources/pixmaps/CDRipper.png $(DESTDIR)${prefix}/share/pixmaps/CDRipper.png
	install -D ../resources/pixmaps/CDRipper.desktop $(DESTDIR)${prefix}/share/applications/CDRipper.desktop
	install -D ../resources/pixmaps/CDRipper.png $(DESTDIR)${prefix}/share/icons/hicolor/256x256/apps/CDRipper.png
	gtk-update-icon-cache --ignore-theme-index --force $(DESTDIR)${prefix}/share/icons/hicolor

uninstall-hook:
	rm -f $(DESTDIR)${prefix}/share/applications/CDRipper.desktop $(DESTDIR)${prefix}/share/pixmaps/CDRipper.png  $(DESTDIR)${prefix}/share/icons/hicolor/256x256/apps/CDRipper.png
	rm -fr $(pkgdatadir)
