#!/bin/bash

# Script to build reflex without autotools
# May be used when autotools is not installed
#
# Normally we want to build with autotools:
#   $ ./configure
#   $ make
#   $ make test
#   $ sudo make install
#
# To also build the examples:
#   $ ./configure --enable-examples

echo
echo "Building reflex..."
echo

# do we have a C compiler?
if ! cc --version >& /dev/null ; then
echo "C compiler command 'cc' does not exist, try building with:"
echo "./configure && make -j"
exit 1
fi

# do we have a C++ compiler?
if ! c++ --version >& /dev/null ; then
echo "C++ compiler command 'c++' does not exist, try building with:"
echo "./configure && make -j"
exit 1
fi

# check if this piece of metal has AVX512BW
cat > conftest.c << END
#include <immintrin.h>
main() { __m512 n = _mm512_set1_epi8(42); (void)_mm512_cmpeq_epi8_mask(n, n); }
END
if cc -march=native -c conftest.c >& /dev/null ; then
  CMFLAGS='-mavx512bw -DHAVE_AVX512BW'
  echo "Compiling reflex with AVX512BW optimizations"
  echo
else

# if not AVX512BW, check if this piece of metal has AVX2
cat > conftest.c << END
#include <immintrin.h>
main() { __m256i n = _mm256_set1_epi8(42); (void)_mm256_movemask_epi8(_mm256_and_si256(n, n)); }
END
if cc -march=native -c conftest.c >& /dev/null ; then
  CMFLAGS='-mavx2 -DHAVE_AVX2'
  echo "Compiling reflex with AVX2 optimizations"
  echo
else

# if not AVX2, check if this piece of metal has SSE2
cat > conftest.c << END
#include <emmintrin.h>
main() { __m128i n = _mm_set1_epi8(42); }
END
if cc -march=native -c conftest.c >& /dev/null ; then
  CMFLAGS='-msse2 -DHAVE_SSE2'
  echo "Compiling reflex with SSE2 optimizations"
  echo
else

# if not AVX512BW/AVX2/SSE2, check if this piece of metal has ARM NEON/AArch64
cat > conftest.c << END
#include <arm_neon.h>
int main() { uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0); }
END
if cc -c conftest.c >& /dev/null ; then
  CMFLAGS='-DHAVE_NEON'
  echo "Compiling reflex with ARM NEON/AArch64 optimizations"
  echo
elif cc -march=native -E conftest.c >& /dev/null ; then
  echo "Checking for ARM NEON/AArch64 instructions (by compiling)"
  if cc -march=native -c conftest.c >& /dev/null ; then
    CMFLAGS='-march=native -DHAVE_NEON'
    echo "Compiling reflex with ARM NEON/AArch64 optimizations"
    echo
  elif cc -o conftest -march=native -mfpu=neon conftest.c >& /dev/null ; then
    echo "Checking for ARM NEON/AArch64 instructions (by execution)"
    if ./conftest >& /dev/null ; then
      CMFLAGS='-march=native -mfpu=neon -DHAVE_NEON'
      echo "Compiling reflex with ARM NEON/AArch64 optimizations"
    else
      echo "Not compiling reflex with ARM NEON/AArch64 optimizations"
    fi
    echo
  fi
  echo
fi
fi
fi
fi

# remove the conftest files
rm -f conftest.c conftest.o conftest

# compile
cd lib; make -j -f Make CMFLAGS="$CMFLAGS" || exit 1; cd -
cd src; make -j -f Make CMFLAGS="$CMFLAGS" || exit 1; cd -

echo
echo "OK"
