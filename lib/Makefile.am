reflexincludedir = $(includedir)/reflex

reflexinclude_HEADERS = \
        $(top_srcdir)/include/reflex/abslexer.h \
        $(top_srcdir)/include/reflex/absmatcher.h \
        $(top_srcdir)/include/reflex/bits.h \
        $(top_srcdir)/include/reflex/boostmatcher.h \
        $(top_srcdir)/include/reflex/convert.h \
        $(top_srcdir)/include/reflex/debug.h \
        $(top_srcdir)/include/reflex/error.h \
        $(top_srcdir)/include/reflex/flexlexer.h \
        $(top_srcdir)/include/reflex/input.h \
        $(top_srcdir)/include/reflex/matcher.h \
        $(top_srcdir)/include/reflex/pattern.h \
        $(top_srcdir)/include/reflex/posix.h \
        $(top_srcdir)/include/reflex/ranges.h \
        $(top_srcdir)/include/reflex/setop.h \
        $(top_srcdir)/include/reflex/simd.h \
        $(top_srcdir)/include/reflex/stdmatcher.h \
        $(top_srcdir)/include/reflex/timer.h \
        $(top_srcdir)/include/reflex/traits.h \
        $(top_srcdir)/include/reflex/unicode.h \
        $(top_srcdir)/include/reflex/utf8.h

lib_LIBRARIES = \
        libreflex.a \
        libreflexmin.a

libreflex_a_CPPFLAGS = \
        -I$(top_srcdir)/include \
        $(SIMD_FLAGS)
libreflex_a_SOURCES = \
        convert.cpp \
        debug.cpp \
        error.cpp \
        input.cpp \
        matcher.cpp \
        matcher_avx2.cpp \
        matcher_avx512bw.cpp \
        pattern.cpp \
        posix.cpp \
        simd.cpp \
        simd_avx2.cpp \
        simd_avx512bw.cpp \
        unicode.cpp \
        utf8.cpp \
        $(top_srcdir)/unicode/block_scripts.cpp \
        $(top_srcdir)/unicode/language_scripts.cpp \
        $(top_srcdir)/unicode/letter_scripts.cpp \
        $(top_srcdir)/unicode/letter_case.cpp \
        $(top_srcdir)/unicode/composer.cpp

libreflexmin_a_CPPFLAGS = \
        -I$(top_srcdir)/include \
        $(SIMD_FLAGS)
libreflexmin_a_SOURCES = \
        debug.cpp \
        error.cpp \
        input.cpp \
        matcher.cpp \
        matcher_avx2.cpp \
        matcher_avx512bw.cpp \
        pattern.cpp \
        simd.cpp \
        simd_avx2.cpp \
        simd_avx512bw.cpp \
        utf8.cpp

# separately compile matcher_avx2.cpp and matcher_avx512bw with AVX2 optimizations enabled
libreflex_a-matcher_avx2.$(OBJEXT)        : CXXFLAGS += $(SIMD_AVX2_FLAGS)
libreflex_a-matcher_avx512bw.$(OBJEXT)    : CXXFLAGS += $(SIMD_AVX512BW_FLAGS)
libreflexmin_a-matcher_avx2.$(OBJEXT)     : CXXFLAGS += $(SIMD_AVX2_FLAGS)
libreflexmin_a-matcher_avx512bw.$(OBJEXT) : CXXFLAGS += $(SIMD_AVX512BW_FLAGS)

# separately compile simd_avx2.cpp and simd_avx512bw.cpp with AVX512BW optimizations enabled
libreflex_a-simd_avx2.$(OBJEXT)        : CXXFLAGS += $(SIMD_AVX2_FLAGS)
libreflex_a-simd_avx512bw.$(OBJEXT)    : CXXFLAGS += $(SIMD_AVX512BW_FLAGS)
libreflexmin_a-simd_avx2.$(OBJEXT)     : CXXFLAGS += $(SIMD_AVX2_FLAGS)
libreflexmin_a-simd_avx512bw.$(OBJEXT) : CXXFLAGS += $(SIMD_AVX512BW_FLAGS)

# removed to avoid Max OS X libtool issues, alas...
# lib_LTLIBRARIES       = libreflex.la libreflexmin.la
#
# libreflex_la_CPPFLAGS = -I$(top_srcdir)/include $(SIMD_FLAGS)
# libreflex_la_SOURCES  = convert.cpp debug.cpp error.cpp input.cpp matcher.cpp matcher_avx2.cpp matcher_avx512bw.cpp pattern.cpp posix.cpp simd_avx2.cpp simd_avx512bw.cpp unicode.cpp utf8.cpp $(top_srcdir)/unicode/block_scripts.cpp $(top_srcdir)/unicode/language_scripts.cpp $(top_srcdir)/unicode/letter_scripts.cpp $(top_srcdir)/unicode/letter_case.cpp $(top_srcdir)/unicode/composer.cpp
#
# libreflexmin_la_CPPFLAGS = -I$(top_srcdir)/include
# libreflexmin_la_SOURCES  = debug.cpp error.cpp input.cpp matcher.cpp matcher_avx2.cpp matcher_avx512bw.cpp pattern.cpp simd_avx2.cpp simd_avx512bw.cpp utf8.cpp
