AC_INIT(re-flex,1.1)
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER(config.h)

AC_HEADER_TIME

AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

AC_PROG_CXX
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_INSTALL

AC_PROG_YACC
AC_SUBST(YACC)

AC_CANONICAL_HOST
PLATFORM=${host}
AC_SUBST(PLATFORM)

AC_ARG_ENABLE(examples,
[AS_HELP_STRING([--enable-examples],
	        [build examples @<:@default=no@:>@])],
[case "${enableval}" in
  yes) enable_examples=true ;;
  no) enable_examples=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-examples) ;;
esac],[enable_examples=false])
AM_CONDITIONAL(ENABLE_EXAMPLES, test x"$enable_examples" = x"true")
AC_SUBST(ENABLE_EXAMPLES)

if test x"$enable_examples" = x"true"; then
   EXAMPLESDIR="examples"
else
   EXAMPLESDIR=
fi
AC_SUBST(EXAMPLESDIR)

AC_PATH_PROG(DOXYGEN, doxygen, NO_DOXYGEN)
if test "$DOXYGEN" = NO_DOXYGEN; then
    AC_MSG_CHECKING([for Doxygen.app])
    IFS=:
    if test -d "/Applications/Doxygen.app"; then
        DOXYGEN="/Applications/Doxygen.app/Contents/Resources/doxygen"
        ac_cv_path_DOXYGEN="/Applications/Doxygen.app/Contents/Resources/doxygen"
    fi
    IFS=$as_save_IFS
    if test "$DOXYGEN" = NO_DOXYGEN; then
        AC_MSG_RESULT([not found])
    else
        AC_MSG_RESULT($DOXYGEN)
    fi
fi
if test "$DOXYGEN" = NO_DOXYGEN; then
    AC_MSG_NOTICE([Couldn't find Doxygen -- documentation updates cannot be built without Doxygen])
else
    AC_CONFIG_FILES([doc/Doxyfile])
fi
DOXYGEN_OUTPUT_DIRECTORY="doc/html"
AC_SUBST(DOXYGEN_OUTPUT_DIRECTORY)
  
AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile tests/Makefile examples/Makefile])

AC_OUTPUT
