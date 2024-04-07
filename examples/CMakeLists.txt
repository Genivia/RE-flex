cmake_minimum_required(VERSION 3.15)
project(reflex-examples CXX)

find_package(Reflex REQUIRED)

add_executable(echo)
reflex_target_sources_headers(
    TARGET echo
    SOURCES echo.l
    INCLUDE_PRIVACY PRIVATE
)

add_executable(calc)
reflex_target_sources_headers(
    TARGET calc
    SOURCES calc.l
    INCLUDE_PRIVACY PRIVATE
)
