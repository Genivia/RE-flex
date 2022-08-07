cmake_minimum_required(VERSION 3.15)

project(reflex CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)

# The following setups the simd_* variables
include(SIMDTestAndSetup)

#
# Defining source variables
#

set(lib_sources
  lib/convert.cpp
  lib/debug.cpp
  lib/error.cpp
  lib/input.cpp
  lib/matcher.cpp
  lib/pattern.cpp
  lib/posix.cpp
  lib/simd_avx2.cpp
  lib/simd_avx512bw.cpp
  lib/unicode.cpp
  lib/utf8.cpp
  lib/matcher_avx2.cpp
  lib/matcher_avx512bw.cpp

  unicode/block_scripts.cpp
  unicode/language_scripts.cpp
  unicode/letter_scripts.cpp
)

set(bin_sources
  src/reflex.cpp
)

list(TRANSFORM lib_sources PREPEND ${PROJECT_SOURCE_DIR}/)
list(TRANSFORM bin_sources PREPEND ${PROJECT_SOURCE_DIR}/)

#
# Defining targets section
#

add_library(ReflexLib SHARED "")
target_sources(ReflexLib PRIVATE ${lib_sources})
target_include_directories(ReflexLib PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_definitions(ReflexLib PRIVATE ${simd_definitions})
target_compile_options(ReflexLib PRIVATE ${simd_flags})

add_library(ReflexLibStatic STATIC "")
target_sources(ReflexLibStatic PRIVATE ${lib_sources})
target_include_directories(ReflexLibStatic PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_definitions(ReflexLibStatic PRIVATE ${simd_definitions})
target_compile_options(ReflexLibStatic PRIVATE ${simd_flags})

add_executable(Reflex "")
target_sources(Reflex PRIVATE ${bin_sources})
target_link_libraries(Reflex PRIVATE ReflexLibStatic)
target_compile_definitions(Reflex PRIVATE ${simd_definitions})
target_compile_options(Reflex PRIVATE ${simd_flags})

# Don't user target name as filename instead use lowercase name for backwards compatibility
set_target_properties(ReflexLibStatic PROPERTIES OUTPUT_NAME reflex_static_lib)
set_target_properties(ReflexLib PROPERTIES OUTPUT_NAME reflex_shared_lib)
set_target_properties(Reflex PROPERTIES OUTPUT_NAME reflex)

#
# Exporting targets section
#

include(GNUInstallDirs)

install(TARGETS Reflex ReflexLib ReflexLibStatic
  EXPORT ReflexTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/reflex
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT ReflexTargets
  NAMESPACE Reflex::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reflex
)

#
# Packaging section (find_package support)
#

include(CMakePackageConfigHelpers)

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/ReflexConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reflex
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/ReflexConfig.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reflex
)