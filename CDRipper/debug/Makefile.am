

include ../flagsandlibs
include ../sources

cdripperdebug_SOURCES = $(COMMONSRC)

cdripperdebug_CFLAGS = -ggdb -O0

bin_PROGRAMS = cdripperdebug

install-exec-local:
	cd $(DESTDIR)$(bindir) && \
	rm cdripperdebug

