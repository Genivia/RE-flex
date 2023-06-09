# SIMD intrinsics setup and test.
# Tests whether specifics intrinsics are available and then defines macro definitions
# Expose the simd_definitions and simd_flags when included from another cmake file.

include(CheckCXXSourceRuns)

option(USE_AVX512BW "Enable AVX512BW intrinsics (if available)" ON)
option(USE_AVX2 "Enable AVX2 intrinsics (if available)" ON)
option(USE_SSE2 "Enable SSE2 intrinsics (if available)" ON)
option(USE_NEON "Enable NEON intrinsics (if available)" ON)

if (USE_AVX512BW)
  check_cxx_source_runs("
    #include <immintrin.h>
    int main() {
      __m512 n = _mm512_set1_epi8(42);
      (void)_mm512_cmpeq_epi8_mask(n, n);
      return 0;
    }
  " HAVE_AVX512BW)
endif()

if (USE_AVX2)
  check_cxx_source_runs("
    #include <immintrin.h>
    int main() {
      __m256i n = _mm256_set1_epi8(42);
      (void)_mm256_movemask_epi8(_mm256_and_si256(n, n));
      return 0;
    }
  " HAVE_AVX2)
endif()

if (USE_SSE2)
  check_cxx_source_runs("
    #include <emmintrin.h>
    int main() {
      __m128i n = _mm_set1_epi8(42);
      return 0;
    }
  " HAVE_SSE2)
endif()

if (USE_NEON)
  check_cxx_source_runs("
    #include <arm_neon.h>
    int main() {
      uint64x2_t n;
      uint64_t m = vgetq_lane_u64(n, 0);
      return 0;
    }
  " HAVE_NEON)
endif()

set(simd_definitions "")
set(simd_flags "")

if (${HAVE_AVX512BW})
  list(APPEND simd_definitions HAVE_AVX512BW)
  if (WIN32 AND MSVC)
    list(APPEND simd_flags "/arch:AVX512")
  else()
    list(APPEND simd_flags "-mavx512bw")
  endif()
endif()

if (${HAVE_AVX2})
  list(APPEND simd_definitions HAVE_AVX2)
  if (WIN32 AND MSVC)
    list(APPEND simd_flags "/arch:AVX2")
  else()
    list(APPEND simd_flags "-mavx2")
  endif()
endif()

if (${HAVE_SSE2})
  list(APPEND simd_definitions HAVE_SSE2)
  if (WIN32 AND MSVC)
    if ("${CMAKE_GENERATOR_PLATFORM}" MATCHES "Win32")
      # SSE2 is a given if the system is running x64
      list(APPEND simd_flags "/arch:SSE2")
    endif()
  else()
    list(APPEND simd_flags "-msse2")
  endif()
endif()

if (${HAVE_NEON})
  list(APPEND simd_definitions HAVE_NEON)
  if ((${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64|aarch64") OR (WIN32 AND MSVC))
    # Arm64 compilers and MSVC runs NEON by default according to their docs
  else()
    list(APPEND simd_flags "-march=native" "-mfpu=neon")
  endif()
endif()

