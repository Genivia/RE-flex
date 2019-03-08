#  Copyright (c) 2018, Michael B Tegtmeyer
#  All rights reserved.
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#      * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#      * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#      * Neither the name of the author nor the names of its contributors may
#        be used to endorse or promote products derived from this software
#        without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
#  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
#  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

AC_PREREQ([2.69])
AC_INIT([reflex],[1.1.0],[github.com/tegtmeye/re-flex])
AC_CONFIG_SRCDIR([Makefile.am])
AM_CONFIG_HEADER(config.h)
AC_CONFIG_AUX_DIR(config)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign tar-pax])
AC_CONFIG_MACRO_DIR([m4])


# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

# Check for C++11 support
AX_CXX_COMPILE_STDCXX([11],[noext],[mandatory])

AC_PROG_YACC
AC_SUBST([LEX])

AC_PROG_LIBTOOL

AC_HEADER_TIME

AC_CANONICAL_HOST

PLATFORM=${host}
AC_SUBST(PLATFORM)

AC_ARG_WITH([examples],
[AS_HELP_STRING([--with-examples],
        [build examples @<:@default=no@:>@])],
[case "${withval}" in
  yes) ;;
  no) ;;
  *) AC_MSG_ERROR([bad value ${withval} for --with-examples]) ;;
esac],[with_examples=no])
AM_CONDITIONAL([EXAMPLES], [test x"$with_examples" = x"yes"])


AC_CHECK_PROG([HAVE_DOT],[dot],[YES],[NO])




AC_PATH_PROG(DOXYGEN, doxygen)
if test -z "$DOXYGEN"; then
   AC_MSG_WARN([Doxygen-generated documentation will not be built])
fi
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doc/Doxyfile])])


# DX_HTML_FEATURE(ON)
# DX_CHM_FEATURE(OFF)
# DX_CHI_FEATURE(OFF)
# DX_MAN_FEATURE(OFF)
# DX_RTF_FEATURE(OFF)
# DX_XML_FEATURE(OFF)
# DX_PDF_FEATURE(OFF)
# DX_PS_FEATURE(OFF)
# DX_INIT_DOXYGEN(reflex, $(top_srcdir)/doc/Doxyfile, doc/reflex-doc)


AC_CONFIG_FILES( \
  Makefile \
  include/Makefile \
  src/Makefile \
  doc/Makefile \
  examples/Makefile)


AC_OUTPUT

