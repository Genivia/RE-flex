cmake_minimum_required(VERSION 3.15)

project(reflex VERSION 6.3.0 LANGUAGES CXX)

# Make the project-specific modules under cmake/ available to include().
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Ask CMake to compile all targets as C++11.
set(CMAKE_CXX_STANDARD 11)

# GNUInstallDirs supplies portable install paths, while CTest defines the
# BUILD_TESTING option and enables test registration when requested.
include(GNUInstallDirs)
include(CTest)

# Clang-cl uses MSVC command-line and binary interfaces despite having a
# different compiler ID.  Treat both toolchains alike when selecting targets.
set(REFLEX_MSVC_TOOLCHAIN FALSE)
if(MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
  set(REFLEX_MSVC_TOOLCHAIN TRUE)
endif()

# The following sets up the simd_* variables.
include(SIMDTestAndSetup)

# Keep the matcher implementation shared by the full and minimal libraries in
# one list so the two variants do not drift apart.
set(libreflex_common_sources
  lib/debug.cpp
  lib/error.cpp
  lib/input.cpp
  lib/matcher.cpp
  lib/pattern.cpp
  lib/simd.cpp
  lib/utf8.cpp
)

# Use AVX2 flags for AVX2 sources.
if(REFLEX_COMPILER_HAS_AVX2)
  set(reflex_avx2_sources lib/matcher_avx2.cpp lib/simd_avx2.cpp)
  list(APPEND libreflex_common_sources ${reflex_avx2_sources})
  set_source_files_properties(${reflex_avx2_sources} PROPERTIES COMPILE_OPTIONS "${simd_avx2_flags}")
endif()

# Use AVX512BW flags for AVX512BW wources.
if(REFLEX_COMPILER_HAS_AVX512BW)
  set(reflex_avx512bw_sources lib/matcher_avx512bw.cpp lib/simd_avx512bw.cpp)
  list(APPEND libreflex_common_sources ${reflex_avx512bw_sources})
  set_source_files_properties(${reflex_avx512bw_sources} PROPERTIES COMPILE_OPTIONS "${simd_avx512bw_flags}")
endif()

# The full library adds pattern conversion, POSIX support, and Unicode tables.
set(libreflex_sources
  ${libreflex_common_sources}
  lib/convert.cpp
  lib/posix.cpp
  lib/unicode.cpp
  unicode/block_scripts.cpp
  unicode/language_scripts.cpp
  unicode/letter_scripts.cpp
  unicode/letter_case.cpp
  unicode/composer.cpp
)

# The minimal library contains only the runtime needed by generated scanners.
set(libreflexmin_sources
  ${libreflex_common_sources}
)

# Only build static libs for MSVC because the public headers do not declare symbols with dllexport/dllimport.
if(REFLEX_MSVC_TOOLCHAIN)
  add_library(ReflexLib STATIC)
  add_library(ReflexMinLib STATIC)
  set(reflex_library_targets ReflexLib)
  set(reflexmin_library_targets ReflexMinLib)
else()
  add_library(ReflexLib SHARED)
  add_library(ReflexLibStatic STATIC)
  add_library(ReflexMinLib SHARED)
  add_library(ReflexMinLibStatic STATIC)
  set(reflex_library_targets ReflexLib ReflexLibStatic)
  set(reflexmin_library_targets ReflexMinLib ReflexMinLibStatic)
endif()

# Set the source files contained in each full-library target.
foreach(target IN LISTS reflex_library_targets)
  target_sources(${target} PRIVATE ${libreflex_sources})
endforeach()

# Set the smaller source set contained in each minimal-library target.
foreach(target IN LISTS reflexmin_library_targets)
  target_sources(${target} PRIVATE ${libreflexmin_sources})
endforeach()

# Gather all library targets so their common build settings remain identical.
set(reflex_all_library_targets
  ${reflex_library_targets}
  ${reflexmin_library_targets}
)

# Use source-tree headers while building and installed headers for consumers;
# SIMD definitions and options remain private implementation details.
foreach(target IN LISTS reflex_all_library_targets)
  target_include_directories(${target} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  target_compile_definitions(${target} PRIVATE ${simd_definitions})
  target_compile_options(${target} PRIVATE ${simd_flags})
endforeach()

# Link the generator to the static full library where it has a distinct target;
# on MSVC-style toolchains ReflexLib is already the static library.
add_executable(Reflex src/reflex.cpp)
if(TARGET ReflexLibStatic)
  target_link_libraries(Reflex PRIVATE ReflexLibStatic)
else()
  target_link_libraries(Reflex PRIVATE ReflexLib)
endif()

# CTest defines BUILD_TESTING; when enabled, build and register the same rtest
# regression executable used by the Autotools build.
if(BUILD_TESTING)
  add_executable(rtest tests/rtest.cpp)
  if(TARGET ReflexLibStatic)
    target_link_libraries(rtest PRIVATE ReflexLibStatic)
  else()
    # MSVC-style builds expose ReflexLib itself as the static library because
    # the public headers do not provide the exports required by a usable DLL.
    target_link_libraries(rtest PRIVATE ReflexLib)
  endif()
  add_test(NAME rtest COMMAND rtest)
endif()

# Target names are internal to CMake; give installed artifacts their traditional
# reflex and reflexmin basenames for compatibility with existing consumers.
foreach(target IN LISTS reflex_library_targets)
  set_target_properties(${target} PROPERTIES OUTPUT_NAME reflex)
endforeach()

foreach(target IN LISTS reflexmin_library_targets)
  set_target_properties(${target} PROPERTIES OUTPUT_NAME reflexmin)
endforeach()

set_target_properties(Reflex PROPERTIES OUTPUT_NAME reflex)

# Install the generator and public libraries, and record the library targets for
# the find_package export generated below.
install(TARGETS Reflex ${reflex_all_library_targets}
  EXPORT ReflexTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install only the public C++ headers from the source tree.
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/reflex
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

# Install imported targets such as Reflex::ReflexLib for downstream CMake users.
install(EXPORT ReflexTargets
  NAMESPACE Reflex::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reflex
)

# Generate and install the relocatable configuration loaded by
# find_package(Reflex CONFIG).
include(CMakePackageConfigHelpers)

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/ReflexConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reflex
)

# Do not promise compatibility between releases until the project defines an
# explicit C++ API and ABI compatibility policy.
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/ReflexConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY ExactVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/ReflexConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/ReflexConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reflex
)

# Make relative installations relocatable by deriving their prefix from the
# installed .pc file.  Absolute library destinations retain their fixed paths.
if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
  set(prefix "${CMAKE_INSTALL_PREFIX}")
  set(exec_prefix "\${prefix}")
  set(libdir "${CMAKE_INSTALL_LIBDIR}")
else()
  # Anchor both inputs in the native build directory because RELATIVE_PATH
  # requires full paths; only their relative structure enters the .pc file.
  file(RELATIVE_PATH reflex_pc_prefix_relative
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/pkgconfig"
    "${CMAKE_CURRENT_BINARY_DIR}"
  )
  set(prefix "\${pcfiledir}/${reflex_pc_prefix_relative}")
  set(exec_prefix "\${prefix}")
  set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
endif()

# GNUInstallDirs also permits the header destination to be absolute.
if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
  set(includedir "${CMAKE_INSTALL_INCLUDEDIR}")
else()
  set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()

# Reuse the Autotools templates, which expect the project version in VERSION.
set(VERSION "${PROJECT_VERSION}")
configure_file(${PROJECT_SOURCE_DIR}/reflex.pc.in
  "${CMAKE_CURRENT_BINARY_DIR}/reflex.pc"
  @ONLY
)

configure_file(${PROJECT_SOURCE_DIR}/reflexmin.pc.in
  "${CMAKE_CURRENT_BINARY_DIR}/reflexmin.pc"
  @ONLY
)

# Install both pkg-config descriptions alongside the libraries.
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/reflex.pc"
  "${CMAKE_CURRENT_BINARY_DIR}/reflexmin.pc"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
