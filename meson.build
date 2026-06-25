project('reflex', 'cpp',
  version: '6.3.0',
  license: 'BSD-3-Clause',
  meson_version: '>= 1.1',
  default_options: ['cpp_std=c++11'])

pkg = import('pkgconfig')

reflex_inc = include_directories('include')

reflex_headers = files(
  'include/reflex/abslexer.h',
  'include/reflex/absmatcher.h',
  'include/reflex/bits.h',
  'include/reflex/boostmatcher.h',
  'include/reflex/convert.h',
  'include/reflex/debug.h',
  'include/reflex/error.h',
  'include/reflex/flexlexer.h',
  'include/reflex/fuzzymatcher.h',
  'include/reflex/input.h',
  'include/reflex/linematcher.h',
  'include/reflex/matcher.h',
  'include/reflex/pattern.h',
  'include/reflex/pcre2matcher.h',
  'include/reflex/posix.h',
  'include/reflex/ranges.h',
  'include/reflex/setop.h',
  'include/reflex/simd.h',
  'include/reflex/stdmatcher.h',
  'include/reflex/timer.h',
  'include/reflex/traits.h',
  'include/reflex/unicode.h',
  'include/reflex/utf8.h')

reflex_sources = files(
  'lib/convert.cpp',
  'lib/debug.cpp',
  'lib/error.cpp',
  'lib/input.cpp',
  'lib/matcher.cpp',
  'lib/matcher_avx2.cpp',
  'lib/matcher_avx512bw.cpp',
  'lib/pattern.cpp',
  'lib/posix.cpp',
  'lib/simd.cpp',
  'lib/simd_avx2.cpp',
  'lib/simd_avx512bw.cpp',
  'lib/unicode.cpp',
  'lib/utf8.cpp',
  'unicode/block_scripts.cpp',
  'unicode/language_scripts.cpp',
  'unicode/letter_scripts.cpp',
  'unicode/letter_case.cpp',
  'unicode/composer.cpp')

reflex_min_sources = files(
  'lib/debug.cpp',
  'lib/error.cpp',
  'lib/input.cpp',
  'lib/matcher.cpp',
  'lib/matcher_avx2.cpp',
  'lib/matcher_avx512bw.cpp',
  'lib/pattern.cpp',
  'lib/simd.cpp',
  'lib/simd_avx2.cpp',
  'lib/simd_avx512bw.cpp',
  'lib/utf8.cpp')

# The Meson build starts with RE/flex's portable code path. Add SIMD feature
# detection later if the portable matcher is too slow for Meson-built packages.
reflex_lib = library('reflex',
  reflex_sources,
  include_directories: reflex_inc,
  install: true)

reflexmin_lib = library('reflexmin',
  reflex_min_sources,
  include_directories: reflex_inc,
  install: true)

reflex_exe = executable('reflex',
  'src/reflex.cpp',
  include_directories: reflex_inc,
  link_with: reflex_lib,
  install: true)

install_headers(reflex_headers, subdir: 'reflex')

reflex_dep = declare_dependency(
  include_directories: reflex_inc,
  link_with: reflex_lib)

reflexmin_dep = declare_dependency(
  include_directories: reflex_inc,
  link_with: reflexmin_lib)

pkg.generate(reflex_lib,
  filebase: 'reflex',
  name: 'RE/flex',
  description: 'high-performance C++ regex library and lexical analyzer generator',
  subdirs: '.')

pkg.generate(reflexmin_lib,
  filebase: 'reflexmin',
  name: 'RE/flex',
  description: 'high-performance C++ regex library and lexical analyzer generator',
  subdirs: '.')

# Register a handler for any requested 'reflex' dependency
meson.override_dependency('reflex', reflex_dep)
# Register a handler for any requested 'reflexmin' dependency
meson.override_dependency('reflexmin', reflexmin_dep)
