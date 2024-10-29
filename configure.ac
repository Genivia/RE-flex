AC_INIT([re-flex],[5.0.1],[https://github.com/Genivia/RE-flex/issues],[reflex])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_COPYRIGHT([Copyright (C) 2016-2024 Robert van Engelen, Genivia Inc.])

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

# removed to avoid MaxOS libtool issues, alas...
# LT_INIT

AC_PROG_YACC
AC_SUBST(YACC)

AC_CANONICAL_HOST
PLATFORM=${host}
AC_SUBST(PLATFORM)

################################################################################
# Use SIMD CPU extensions and manage cross compilation
################################################################################

# --disable-sse2 fallback option if cross compilation fails to disable SSE2 CPU extensions
AC_ARG_ENABLE(sse2,
  [AS_HELP_STRING([--disable-sse2],
                  [disable SSE2 CPU extensions])],
  [with_no_sse2="yes"],
  [with_no_sse2="no"])

# --disable-avx2 fallback option if cross compilation fails to disable AVX2/AVX512BW CPU extensions
AC_ARG_ENABLE(avx2,
  [AS_HELP_STRING([--disable-avx2],
                  [disable AVX2/AVX512BW CPU extensions])],
  [with_no_avx2="yes"],
  [with_no_avx2="no"])

# --disable-neon fallback option if cross compilation fails to disable NEON/AArch64 CPU extensions
AC_ARG_ENABLE(neon,
  [AS_HELP_STRING([--disable-neon],
                 [disable NEON CPU extensions])],
  [with_no_neon="yes"],
  [with_no_neon="no"])

# SIMD_FLAGS applies to all source code
SIMD_FLAGS=
# SIMD_AVX2_FLAGS applies to lib/matcher_avx2.cpp which is selected based on a runtime AVX2 check
SIMD_AVX2_FLAGS=
# SIMD_AVX512BW_FLAGS applies to lib/matcher_avx512bw.cpp which is selected based on a runtime AVX512BW check
SIMD_AVX512BW_FLAGS=

if test "x$cross_compiling" = "xyes"; then

  # cross compiling to a host CPU: check support for SIMD CPU extension compile flags
  case $host_cpu in
    i[[3456]]86*|x86_64*|amd64*)
      # enable SSE2/AVX2/AVX512BW extensions
      if ! test "x$with_no_sse2" = "xyes"; then
        AC_MSG_CHECKING([whether ${CXX} supports SSE2 intrinsics])
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="-msse2"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <emmintrin.h>]], [[__m128i n = _mm_set1_epi8(42);]])],
                          [msse2_ok=yes],
                          [msse2_ok=no])
        CXXFLAGS=$save_CXXFLAGS
        AC_MSG_RESULT($msse2_ok)
        if test "x$msse2_ok" = "xyes"; then
          SIMD_FLAGS="-msse2 -DHAVE_SSE2"
          if ! test "x$with_no_avx2" = "xyes"; then
            AC_MSG_CHECKING([whether ${CXX} supports AVX2/AVX512BW intrinsics])
            save_CXXFLAGS=$CXXFLAGS
            CXXFLAGS="-mavx512bw"
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <immintrin.h>]], [[__m512i n = _mm512_set1_epi8(42); (void)_mm512_cmpeq_epi8_mask(n, n);]])],
                              [mavx_ok=yes],
                              [mavx_ok=no])
            if test "x$mavx_ok" = "xyes"; then
              SIMD_FLAGS="-msse2 -DHAVE_AVX512BW"
              SIMD_AVX2_FLAGS="-mavx2"
              SIMD_AVX512BW_FLAGS="-mavx512bw"
            else
              CXXFLAGS="-mavx2"
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
          fi
        fi
      fi
    ;;
    armv5*|armv6*)
    ;;
    arm*)
      # enable arm >= 7 neon extensions with option -mfpu=neon
      if ! test "x$with_no_neon" = "xyes"; then
        AC_MSG_CHECKING([whether ${CXX} supports ARM NEON/AArch64 intrinsics])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]], [[uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0);]])],
            [mneon_ok=yes],
            [mneon_ok=no])
        if test "x$mneon_ok" = "xyes"; then
          SIMD_FLAGS="-DHAVE_NEON"
        else
          save_CXXFLAGS=$CXXFLAGS
          CXXFLAGS="-mfpu=neon"
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]], [[uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0);]])],
                            [mneon_ok=yes],
                            [mneon_ok=no])
          if test "x$mneon_ok" = "xyes"; then
            SIMD_FLAGS="-mfpu=neon -DHAVE_NEON"
          else
            # this is iffy,,,
            CXXFLAGS="-march=native -mfpu=neon"
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]], [[uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0);]])],
                              [mneon_ok=yes],
                              [mneon_ok=no])
            if test "x$mneon_ok" = "xyes"; then
              SIMD_FLAGS="-march=native -mfpu=neon -DHAVE_NEON"
            fi
          fi
          CXXFLAGS=$save_CXXFLAGS
        fi
        AC_MSG_RESULT($mneon_ok)
      fi
    ;;
    aarch64*)
      # enable AArch64 neon extensions
      if ! test "x$with_no_neon" = "xyes"; then
        SIMD_FLAGS="-DHAVE_NEON"
      fi
    ;;
  esac

else

  if ! test "x$with_no_sse2" = "xyes"; then
    AC_MSG_CHECKING([whether ${CXX} supports SSE2 intrinsics])
    save_CXXFLAGS=$CXXFLAGS
    CXXFLAGS="-msse2"
    AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <emmintrin.h>]],[[__m128i n = _mm_set1_epi8(42);]])],
                  [msse2_ok=yes],
                  [msse2_ok=no])
    CXXFLAGS=$save_CXXFLAGS
    AC_MSG_RESULT($msse2_ok)
    if test "x$msse2_ok" = "xyes"; then
      SIMD_FLAGS="-msse2 -DHAVE_SSE2"
      AC_MSG_CHECKING(for --disable-avx)
      if ! test "x$with_no_avx2" = "xyes"; then
        AC_MSG_RESULT(no)
        AC_MSG_CHECKING([whether ${CXX} supports AVX2/AVX512BW intrinsics])
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="-mavx512bw"
        AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <immintrin.h>]],[[__m512i n = _mm512_set1_epi8(42); (void)_mm512_cmpeq_epi8_mask(n, n);]])],
                      [mavx_ok=yes],
                      [mavx_ok=no])
        if test "x$mavx_ok" = "xyes"; then
          SIMD_FLAGS="-msse2 -DHAVE_AVX512BW"
          SIMD_AVX2_FLAGS="-mavx2"
          SIMD_AVX512BW_FLAGS="-mavx512bw"
        else
          CXXFLAGS="-mavx2"
          AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <immintrin.h>]],[[__m256i n = _mm256_set1_epi8(42); (void)_mm256_movemask_epi8(_mm256_and_si256(n, n));]])],
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
  fi

  if test -z "$SIMD_FLAGS"; then
    if ! test "x$with_no_neon" = "xyes"; then
      AC_MSG_CHECKING([whether ${CXX} supports NEON/AArch64 intrinsics])
      AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]],[[uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0);]])],
                    [mneon_ok=yes],
                    [mneon_ok=no])
      if test "x$mneon_ok" = "xyes"; then
        SIMD_FLAGS="-DHAVE_NEON"
      else
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="-mfpu=neon"
        AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]],[[uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0);]])],
                      [mneon_ok=yes],
                      [mneon_ok=no])
        if test "x$mneon_ok" = "xyes"; then
          SIMD_FLAGS="-mfpu=neon -DHAVE_NEON"
        else
          CXXFLAGS="-march=native -mfpu=neon"
          AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]],[[uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0);]])],
                        [mneon_ok=yes],
                        [mneon_ok=no])
          if test "x$mneon_ok" = "xyes"; then
            SIMD_FLAGS="-march=native -mfpu=neon -DHAVE_NEON"
          fi
        fi
        CXXFLAGS=$save_CXXFLAGS
      fi
      AC_MSG_RESULT($mneon_ok)
    fi
  fi

fi

AC_SUBST(SIMD_FLAGS)
AC_SUBST(SIMD_AVX2_FLAGS)
AC_SUBST(SIMD_AVX512BW_FLAGS)

################################################################################
# Build the examples when requested, requires Bison
################################################################################

AC_ARG_ENABLE(examples,
  [AS_HELP_STRING([--enable-examples],
                  [build examples @<:@default=no@:>@])],
  [enable_examples=yes],
  [enable_examples=no])
AM_CONDITIONAL([ENABLE_EXAMPLES],[test "x$enable_examples" = "xyes"])
AC_SUBST(ENABLE_EXAMPLES)

if test "x$enable_examples" = "xyes"; then
  EXAMPLESDIR="examples"
else
  EXAMPLESDIR=
fi
AC_SUBST(EXAMPLESDIR)

################################################################################
# Check for Doxygen to update manual, not a required dependency, manual included
################################################################################

AC_PATH_PROG(DOXYGEN,doxygen,NO_DOXYGEN)
if test "$DOXYGEN" = NO_DOXYGEN; then
  AC_MSG_CHECKING([for Doxygen.app])
  IFS=:
  if test -d "/Applications/Doxygen.app"; then
    DOXYGEN="/Applications/Doxygen.app/Contents/Resources/doxygen"
    ac_cv_path_DOXYGEN="/Applications/Doxygen.app/Contents/Resources/doxygen"
  fi
  IFS=$as_save_IFS
  if test "x$DOXYGEN" = "xNO_DOXYGEN"; then
    AC_MSG_RESULT([not found])
  else
    AC_MSG_RESULT($DOXYGEN)
  fi
fi
if test "x$DOXYGEN" = "xNO_DOXYGEN"; then
  AC_MSG_NOTICE([Couldn't find Doxygen -- documentation updates cannot be built without Doxygen])
else
  AC_CONFIG_FILES([doc/Doxyfile])
fi
DOXYGEN_OUTPUT_DIRECTORY="doc/html"
AC_SUBST(DOXYGEN_OUTPUT_DIRECTORY)
  
AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile tests/Makefile examples/Makefile reflex.pc reflexmin.pc])

AC_OUTPUT
