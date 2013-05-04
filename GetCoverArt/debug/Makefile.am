

include ../flagsandlibs
include ../sources

getcoverartdebug_SOURCES = $(COMMONSRC)

getcoverartdebug_CFLAGS = -ggdb -O0

bin_PROGRAMS = getcoverartdebug

install-exec-local:
	cd $(DESTDIR)$(bindir) && \
	rm getcoverartdebug

