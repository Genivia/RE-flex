project(RE-Flex)
cmake_minimum_required(VERSION 3.5)

option(REFLEX_TEST "Build the reflex tests" OFF)
option(REFLEX_SANITIZER_BUILD "Build using AddressSanitizer" OFF)

enable_language(CXX)

if (REFLEX_SANITIZER_BUILD)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

include_directories(include)
include(cmake/gen_lexer.cmake)

add_subdirectory(lib)
add_subdirectory(unicode)
add_subdirectory(src)

if (REFLEX_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()