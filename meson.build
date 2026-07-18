project(
  'reflex',
  'cpp',
  version: '6.3.0',
  license: 'BSD-3-Clause',
  meson_version: '>= 1.5.0',
  default_options: {
    'cpp_std': 'c++11',
  },
)

includes = include_directories('include')

cpp = meson.get_compiler('cpp')
msvc = cpp.get_argument_syntax() == 'msvc'

flags = cpp.get_supported_arguments(['-Wno-non-virtual-dtor'])
add_project_arguments(
  flags,
  language: 'cpp',
)

unicode_sources = files(
  'unicode/block_scripts.cpp',
  'unicode/composer.cpp',
  'unicode/language_scripts.cpp',
  'unicode/letter_case.cpp',
  'unicode/letter_scripts.cpp',
)

libreflex_sources = files(
  'lib/convert.cpp',
  'lib/debug.cpp',
  'lib/error.cpp',
  'lib/input.cpp',
  'lib/matcher.cpp',
  'lib/pattern.cpp',
  'lib/posix.cpp',
  'lib/simd.cpp',
  'lib/unicode.cpp',
  'lib/utf8.cpp',
)

libreflexmin_sources = files(
  'lib/debug.cpp',
  'lib/error.cpp',
  'lib/input.cpp',
  'lib/matcher.cpp',
  'lib/pattern.cpp',
  'lib/simd.cpp',
  'lib/utf8.cpp',
)

have_simd_sse2 = cpp.compiles(
  '''
  #include <emmintrin.h>
  int main(int argc, char** argv) {
    __m128i n = _mm_set1_epi8(42);
  }
  ''',
  args: msvc ? '/arch:SSE2' : '-msse2',
  name: 'cxx supports SSE2 intrinsics with options [\'-msse2\']',
  required: get_option('sse2'),
)

have_simd_avx2 = cpp.compiles(
  '''
  #include <immintrin.h>
  int main(int argc, char** argv) {
    __m256i n = _mm256_set1_epi8(42);
    (void)_mm256_movemask_epi8(_mm256_and_si256(n, n));
  }
  ''',
  args: msvc ? '/arch:AVX2' : '-mavx2',
  name: 'cxx supports AVX2 intrinsics with options [\'-mavx2\']',
  required: get_option('avx2'),
)

have_simd_avx512bw = cpp.compiles(
  '''
  #include <immintrin.h>
  int main(int argc, char** argv) {
    __m512i n = _mm512_set1_epi8(42);
    (void)_mm512_cmpeq_epi8_mask(n, n);
  }
  ''',
  args: msvc ? '/arch:AVX512' : '-mavx512bw',
  name: 'cxx supports AVX512BW intrinsics with options [\'-mavx512bw\']',
  required: get_option('avx512bw'),
)

if have_simd_avx512bw
  simd_args = [msvc ? '/arch:SSE2' : '-msse2']
  simd_avx2_args = [msvc ? '/arch:AVX2' : '-mavx2']
  simd_avx512bw_args = [msvc ? '/arch:AVX512' : '-mavx512bw']
  add_project_arguments(
    '-DHAVE_AVX512BW',
    language: 'cpp',
  )
elif have_simd_avx2
  simd_args = [msvc ? '/arch:SSE2' : '-msse2']
  simd_avx2_args = [msvc ? '/arch:AVX2' : '-mavx2']
  simd_avx512bw_args = []
  add_project_arguments(
    '-DHAVE_AVX2',
    language: 'cpp',
  )
elif have_simd_sse2
  simd_args = [msvc ? '/arch:SSE2' : '-msse2']
  simd_avx2_args = []
  simd_avx512bw_args = []
  add_project_arguments(
    '-DHAVE_SSE2',
    language: 'cpp',
  )
else
  simd_args = []
  simd_avx2_args = []
  simd_avx512bw_args = []
endif

neon_option = get_option('neon')
have_neon = false
if not neon_option.disabled()
  foreach args : [[], ['-mfpu=neon'], ['-march=native', '-mfpu=neon']]
    have_neon = cpp.compiles(
      '''
      #include <arm_neon.h>
      int main(int argc, char** argv) {
        uint64x2_t n;
        uint64_t m = vgetq_lane_u64(n, 0);
      }
      ''',
      args: args,
      name: 'cxx supports NEON intrinsics with options @0@'.format(args),
      required: false,
    )
    if have_neon
      simd_args = args
      add_project_arguments(
        '-DHAVE_NEON',
        language: 'cpp',
      )
      break
    endif
  endforeach
endif

if neon_option.enabled() and not have_neon
  error('NEON support was enabled but the compiler does not support NEON intrinsics')
endif

libsimd_avx2 = static_library(
  'libsimd_avx2',
  'lib/matcher_avx2.cpp',
  'lib/simd_avx2.cpp',
  cpp_args: simd_avx2_args,
  pic: true,
  include_directories: includes,
  build_by_default: false,
)

libsimd_avx512bw = static_library(
  'libsimd_avx512bw',
  'lib/matcher_avx512bw.cpp',
  'lib/simd_avx512bw.cpp',
  cpp_args: simd_avx512bw_args,
  pic: true,
  include_directories: includes,
  build_by_default: false,
  # clang 14.0 fails to compile this code at optimization levels higher than 0
  override_options: cpp.get_id() == 'clang' and cpp.version() == '14.0.0' ? {
    'optimization': '0',
  } : {},
)

# No DLLEXPORT declarations, thus only static library supported with MSVC
library = msvc ? 'static_library' : 'library'

libreflexmin = build_target(
  'reflexmin',
  libreflexmin_sources,
  include_directories: includes,
  cpp_args: simd_args,
  # Selected based on a runtime AVX2 AVX512BW check
  link_whole: [libsimd_avx2, libsimd_avx512bw],
  target_type: library,
)

libreflex = build_target(
  'reflex',
  libreflex_sources,
  unicode_sources,
  include_directories: includes,
  cpp_args: simd_args,
  # Selected based on a runtime AVX2 AVX512BW check
  link_whole: [libsimd_avx2, libsimd_avx512bw],
  target_type: library,
)

reflex = executable(
  'reflex',
  'src/reflex.cpp',
  link_with: libreflex,
  include_directories: includes,
  install: true,
  cpp_args: ['-DPLATFORM="@0@"'.format(host_machine.system())],
)

reflex_dep = declare_dependency(
  link_with: libreflex,
  include_directories: includes,
)

meson.override_find_program('reflex', reflex)
