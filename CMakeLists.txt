cmake_minimum_required(VERSION 3.15)

project(reflex CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

set(CMAKE_CXX_STANDARD 11)

# The following setups the simd_* variables
include(SIMDTestAndSetup)

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

  unicode/block_scripts.cpp
  unicode/language_scripts.cpp
  unicode/letter_scripts.cpp
)

set(bin_sources
  src/reflex.cpp
)

list(TRANSFORM lib_sources PREPEND ${PROJECT_SOURCE_DIR}/)
list(TRANSFORM bin_sources PREPEND ${PROJECT_SOURCE_DIR}/)

add_library(libreflex SHARED "")
target_sources(libreflex PRIVATE ${lib_sources})
target_include_directories(libreflex PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_definitions(libreflex PRIVATE ${simd_definitions})
target_compile_options(libreflex PRIVATE ${simd_flags})

add_library(libreflexStatic STATIC "")
target_sources(libreflexStatic PRIVATE ${lib_sources})
target_include_directories(libreflexStatic PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_definitions(libreflexStatic PRIVATE ${simd_definitions})
target_compile_options(libreflexStatic PRIVATE ${simd_flags})

# Renaming static lib output to match the name (without the extention) of the shared library
set_target_properties(libreflexStatic
    PROPERTIES OUTPUT_NAME reflex
)
set_target_properties(libreflex
    PROPERTIES OUTPUT_NAME reflex
)

add_executable(reflex "")
target_sources(reflex PRIVATE ${bin_sources})
target_link_libraries(reflex PRIVATE libreflexStatic)
target_compile_definitions(reflex PRIVATE ${simd_definitions})
target_compile_options(reflex PRIVATE ${simd_flags})

include(GNUInstallDirs)

install(TARGETS reflex libreflex libreflexStatic EXPORT reflex
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/reflex
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/cmake/reflex-config.cmake DESTINATION reflex/share/reflex)

install(EXPORT reflex DESTINATION reflex/share/reflex)