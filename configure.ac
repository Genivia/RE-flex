AC_INIT([re-flex],3.1)
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h])
AC_COPYRIGHT([Copyright (C) 2016-2021 Robert van Engelen, Genivia Inc.])

AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# if CXXFLAGS is undefined, set it to our preferred default flags
: ${CXXFLAGS="-Wall -Wextra -Wunused -O2"}

AC_LANG([C++])
AC_PROG_CXX
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_INSTALL

AC_CHECK_HEADERS([sys/time.h])

# removed to avoid Max OS X libtool issues, alas...
# LT_INIT

AC_PROG_YACC
AC_SUBST(YACC)

AC_CANONICAL_HOST
PLATFORM=${host}
AC_SUBST(PLATFORM)

AC_ARG_ENABLE(sse2,
  [AS_HELP_STRING([--disable-sse2],
                  [disable SSE2 and AVX optimizations])],
  [with_no_sse2="yes"],
  [with_no_sse2="no"])
AC_MSG_CHECKING(for --disable-sse2)
if test "x$with_no_sse2" = "xno"; then
  AC_MSG_RESULT(no)
  AC_MSG_CHECKING([whether ${CXX} supports SSE2 intrinsics])
  save_CXXFLAGS=$CXXFLAGS
  CXXFLAGS="-march=native"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <emmintrin.h>]], [[__m128i n = _mm_set1_epi8(42);]])],
                    [msse2_ok=yes],
                    [msse2_ok=no])
  CXXFLAGS=$save_CXXFLAGS
  AC_MSG_RESULT($msse2_ok)
  if test "x$msse2_ok" = "xyes"; then
    SIMD_FLAGS="-msse2 -DHAVE_SSE2"
    AC_ARG_ENABLE(avx,
      [AS_HELP_STRING([--disable-avx],
                      [disable AVX optimizations])],
      [with_no_avx="yes"],
      [with_no_avx="no"])
    AC_MSG_CHECKING(for --disable-avx)
    if test "x$with_no_avx" = "xno"; then
      AC_MSG_RESULT(no)
      AC_MSG_CHECKING([whether ${CXX} supports AVX intrinsics])
      save_CXXFLAGS=$CXXFLAGS
      CXXFLAGS="-march=native"
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <immintrin.h>]], [[__m512 n = _mm512_set1_epi8(42); (void)_mm512_cmpeq_epi8_mask(n, n);]])],
                        [mavx_ok=yes],
                        [mavx_ok=no])
      if test "x$mavx_ok" = "xyes"; then
        SIMD_FLAGS="-msse2 -DHAVE_AVX512BW"
        SIMD_AVX2_FLAGS="-mavx2"
        SIMD_AVX512BW_FLAGS="-mavx512bw"
      else
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <immintrin.h>]], [[__m256i n = _mm256_set1_epi8(42); (void)_mm256_movemask_epi8(_mm256_and_si256(n, n));]])],
                          [mavx_ok=yes],
                          [mavx_ok=no])
        if test "x$mavx_ok" = "xyes"; then
          SIMD_FLAGS="-msse2 -DHAVE_AVX2"
          SIMD_AVX2_FLAGS="-mavx2"
        fi
      fi
      CXXFLAGS=$save_CXXFLAGS
      AC_MSG_RESULT($mavx_ok)
    else
      AC_MSG_RESULT(yes)
    fi
  fi
else
  AC_MSG_RESULT(yes)
  SIMD_FLAGS=
fi

if test "x$SIMD_FLAGS" = "x"; then
AC_ARG_ENABLE(neon,
  [AS_HELP_STRING([--disable-neon],
                  [disable ARM NEON/AArch64 optimizations])],
  [with_no_neon="yes"],
  [with_no_neon="no"])
AC_MSG_CHECKING(for --disable-neon)
if test "x$with_no_neon" = "xno"; then
  AC_MSG_RESULT(no)
  AC_MSG_CHECKING([whether ${CXX} supports ARM NEON/AArch64 intrinsics])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]], [[uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0);]])],
                    [mneon_ok=yes],
                    [mneon_ok=no])
  if test "x$mneon_ok" = "xyes"; then
    SIMD_FLAGS="-DHAVE_NEON"
  else
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="-march=native"
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]], [[uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0);]])],
                      [mneon_ok=yes],
                      [mneon_ok=no])
    if test "x$mneon_ok" = "xyes"; then
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]], [[uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0);]])],
                        [mneon_ok=yes],
                        [mneon_ok=no])
      if test "x$mneon_ok" = "xyes"; then
        SIMD_FLAGS="-march=native -DHAVE_NEON"
      else
        CXXFLAGS="-march=native -mfpu=neon"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]], [[uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0);]])],
                          [mneon_ok=yes],
                          [mneon_ok=no])
        if test "x$mneon_ok" = "xyes"; then
          SIMD_FLAGS="-march=native -mfpu=neon -DHAVE_NEON"
        fi
      fi
    fi
  fi
  CXXFLAGS=$save_CXXFLAGS
  AC_MSG_RESULT($mneon_ok)
else
  AC_MSG_RESULT(yes)
  SIMD_FLAGS=
fi
fi

AC_SUBST(SIMD_FLAGS)
AC_SUBST(SIMD_AVX2_FLAGS)
AC_SUBST(SIMD_AVX512BW_FLAGS)

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
    if test x"$DOXYGEN" = x"NO_DOXYGEN"; then
        AC_MSG_RESULT([not found])
    else
        AC_MSG_RESULT($DOXYGEN)
    fi
fi
if test x"$DOXYGEN" = x"NO_DOXYGEN"; then
    AC_MSG_NOTICE([Couldn't find Doxygen -- documentation updates cannot be built without Doxygen])
else
    AC_CONFIG_FILES([doc/Doxyfile])
fi
DOXYGEN_OUTPUT_DIRECTORY="doc/html"
AC_SUBST(DOXYGEN_OUTPUT_DIRECTORY)
  
AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile tests/Makefile examples/Makefile])

AC_OUTPUT
