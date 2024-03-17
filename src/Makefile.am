bin_PROGRAMS    = reflex
reflex_CPPFLAGS = \
        -I$(top_srcdir)/include \
        -DPLATFORM=\"$(PLATFORM)\"
reflex_SOURCES  = \
        reflex.cpp \
        $(top_srcdir)/include/convert.h \
        $(top_srcdir)/include/debug.h \
        $(top_srcdir)/include/error.h \
        $(top_srcdir)/include/input.h \
        $(top_srcdir)/include/pattern.h \
        $(top_srcdir)/include/utf8.h
reflex_LDADD    = \
        $(top_builddir)/lib/libreflex.a
