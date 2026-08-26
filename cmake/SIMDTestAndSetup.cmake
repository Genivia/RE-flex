# Test whether the compiler accepts each SIMD implementation with its required
# options.  The resulting variables configure baseline and optimized sources.

# These modules provide compile probes and isolate their temporary flags from
# the rest of the project configuration.
include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

# Options let packagers disable a backend; an enabled backend must still pass
# its compiler probe before it is built.
option(USE_AVX512BW "Enable AVX512BW intrinsics (if available)" ON)
option(USE_AVX2 "Enable AVX2 intrinsics (if available)" ON)
option(USE_SSE2 "Enable SSE2 intrinsics (if available)" ON)
option(USE_NEON "Enable NEON intrinsics (if available)" ON)

# Select the SIMD option spelling understood by the active compiler interface.
if(REFLEX_MSVC_TOOLCHAIN)
  set(reflex_sse2_flag "/arch:SSE2")
  set(reflex_avx2_flag "/arch:AVX2")
  set(reflex_avx512bw_flag "/arch:AVX512")
else()
  set(reflex_sse2_flag "-msse2")
  set(reflex_avx2_flag "-mavx2")
  set(reflex_avx512bw_flag "-mavx512bw")
endif()

# Compile rather than run the probe so configuration also works when the build
# machine cannot execute binaries for the target architecture.
if(USE_SSE2)
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_FLAGS "${reflex_sse2_flag}")
  check_cxx_source_compiles("
    #include <emmintrin.h>
    int main() {
      __m128i n = _mm_set1_epi8(42);
      return _mm_cvtsi128_si32(n) != 42;
    }
  " REFLEX_COMPILER_HAS_SSE2)
  cmake_pop_check_state()
else()
  set(REFLEX_COMPILER_HAS_SSE2 FALSE)
endif()

# AVX implementations use SSE2 as their portable runtime fallback, so do not
# enable them when the baseline SSE2 implementation was explicitly disabled.
if(USE_AVX2 AND REFLEX_COMPILER_HAS_SSE2)
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_FLAGS "${reflex_avx2_flag}")
  check_cxx_source_compiles("
    #include <immintrin.h>
    int main() {
      __m256i n = _mm256_set1_epi8(42);
      return _mm256_movemask_epi8(_mm256_and_si256(n, n)) == 0;
    }
  " REFLEX_COMPILER_HAS_AVX2)
  cmake_pop_check_state()
else()
  set(REFLEX_COMPILER_HAS_AVX2 FALSE)
endif()

# The AVX512BW implementation dispatches to AVX2 on machines without AVX512BW,
# so only build it when the AVX2 implementation is also available.
if(USE_AVX512BW AND REFLEX_COMPILER_HAS_AVX2)
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_FLAGS "${reflex_avx512bw_flag}")
  check_cxx_source_compiles("
    #include <immintrin.h>
    int main() {
      __m512i n = _mm512_set1_epi8(42);
      return _mm512_cmpeq_epi8_mask(n, n) == 0;
    }
  " REFLEX_COMPILER_HAS_AVX512BW)
  cmake_pop_check_state()
else()
  set(REFLEX_COMPILER_HAS_AVX512BW FALSE)
endif()

# Probe NEON only when no x86 backend is available.  AArch64 needs no extra
# option, while some 32-bit ARM compilers require -mfpu=neon.
if(USE_NEON AND NOT REFLEX_COMPILER_HAS_SSE2)
  cmake_push_check_state(RESET)
  check_cxx_source_compiles("
    #include <arm_neon.h>
    int main() {
      uint64x2_t n = vdupq_n_u64(42);
      return vgetq_lane_u64(n, 0) != 42;
    }
  " REFLEX_COMPILER_HAS_NEON)
  cmake_pop_check_state()

  if(NOT REFLEX_COMPILER_HAS_NEON AND NOT REFLEX_MSVC_TOOLCHAIN)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_FLAGS "-mfpu=neon")
    check_cxx_source_compiles("
      #include <arm_neon.h>
      int main() {
        uint64x2_t n = vdupq_n_u64(42);
        return vgetq_lane_u64(n, 0) != 42;
      }
    " REFLEX_COMPILER_HAS_NEON_WITH_FLAG)
    cmake_pop_check_state()
  else()
    set(REFLEX_COMPILER_HAS_NEON_WITH_FLAG FALSE)
  endif()
else()
  set(REFLEX_COMPILER_HAS_NEON FALSE)
  set(REFLEX_COMPILER_HAS_NEON_WITH_FLAG FALSE)
endif()

# These lists are consumed by the parent CMakeLists.txt when it defines targets.
set(simd_definitions "")
set(simd_flags "")
set(simd_avx2_flags "")
set(simd_avx512bw_flags "")

# Define only the strongest backend macro.  Baseline sources stay at SSE2 while
# optimized AVX sources receive the flags required by their specialized code.
if(REFLEX_COMPILER_HAS_AVX512BW)
  list(APPEND simd_definitions HAVE_AVX512BW)
  list(APPEND simd_flags "${reflex_sse2_flag}")
  list(APPEND simd_avx2_flags "${reflex_avx2_flag}")
  list(APPEND simd_avx512bw_flags "${reflex_avx512bw_flag}")
elseif(REFLEX_COMPILER_HAS_AVX2)
  list(APPEND simd_definitions HAVE_AVX2)
  list(APPEND simd_flags "${reflex_sse2_flag}")
  list(APPEND simd_avx2_flags "${reflex_avx2_flag}")
elseif(REFLEX_COMPILER_HAS_SSE2)
  list(APPEND simd_definitions HAVE_SSE2)
  list(APPEND simd_flags "${reflex_sse2_flag}")
elseif(REFLEX_COMPILER_HAS_NEON)
  list(APPEND simd_definitions HAVE_NEON)
elseif(REFLEX_COMPILER_HAS_NEON_WITH_FLAG)
  list(APPEND simd_definitions HAVE_NEON)
  list(APPEND simd_flags "-mfpu=neon")
endif()
