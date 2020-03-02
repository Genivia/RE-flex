#!/bin/bash
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
main() { __m512 n = _mm512_set1_epi8(42); __mmask64 m = _mm512_cmpeq_epi8_mask(n, n); }
END
if cc -march=native -c conftest.c >& /dev/null ; then
  CMFLAGS='-march=native -mavx512bw -DHAVE_AVX512BW'
  echo "Compiling reflex with AVX512BW optimizations"
  echo
else

# check if this piece of metal has AVX
cat > conftest.c << END
#include <immintrin.h>
main() { __m256i n = _mm256_set1_epi8(42); n = _mm256_and_si256(n, n); }
END
if cc -march=native -c conftest.c >& /dev/null ; then
  CMFLAGS='-march=native -mavx -DHAVE_AVX'
  echo "Compiling reflex with AVX optimizations"
  echo
else

# if not AVX, check if this piece of metal has SSE2
cat > conftest.c << END
#include <emmintrin.h>
main() { __m128i n = _mm_set1_epi8(42); }
END
if cc -march=native -c conftest.c >& /dev/null ; then
  CMFLAGS='-march=native -msse2 -DHAVE_SSE2'
  echo "Compiling reflex with SSE2 optimizations"
  echo
else

# if not AVX/SSE2, check if this piece of metal has ARM NEON/AArch64
cat > conftest.c << END
#include <arm_neon.h>
main() { uint64x2_t n; uint64_t m = vgetq_lane_u64(n, 0); }
END
if cc -march=native -E conftest.c >& /dev/null ; then
  if cc -march=native -c conftest.c >& /dev/null ; then
    CMFLAGS='-march=native -DHAVE_NEON'
    echo "Compiling reflex with ARM AArch64 optimizations"
    echo
  elif cc -march=native -mfpu=neon -c conftest.c >& /dev/null ; then
    CMFLAGS='-march=native -mfpu=neon -DHAVE_NEON'
    echo "Compiling reflex with ARM NEON optimizations"
    echo
  fi
fi
fi
fi
fi

# remove the conftest files
rm -f conftest.c conftest.o

# compile
cd lib; make -j -f Make CMFLAGS="$CMFLAGS" || exit 1; cd -
cd src; make -j -f Make CMFLAGS="$CMFLAGS" || exit 1; cd -

echo
echo "OK"
