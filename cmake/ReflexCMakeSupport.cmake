if (NOT TARGET Reflex::Reflex)
    message(FATAL_ERROR "Reflex::Reflex is not a target, please make sure Reflex executable is a target before including this script")
endif()

function(reflex_target_sources_headers)
    set(REFLEX_OPTIONS "REFLEXLIB_STATIC")
    set(REFLEX_SINGLE_VALUE_KEYWORDS "TARGET" "INCLUDE_PRIVACY")
    set(REFLEX_MULTIVALUE_KEYWORDS "SOURCES")

    cmake_parse_arguments("" "${REFLEX_OPTIONS}" "${REFLEX_SINGLE_VALUE_KEYWORDS}" "${REFLEX_MULTIVALUE_KEYWORDS}" "${ARGV}")
    message(STATUS "_SOURCES=${_SOURCES}")
    list(LENGTH _SOURCES sources_count)
    if (sources_count LESS 1)
        message(FATAL_ERROR "No input files specified for Reflex")
    endif()

    set(processed_filepaths)
    set(output_sources)
    set(output_headers)
    set(source_output_prefix ${CMAKE_CURRENT_BINARY_DIR}/reflex_generated_${_TARGET})

    file(MAKE_DIRECTORY ${source_output_prefix})
    set(header_output_prefix ${source_output_prefix}/include/${_TARGET})
    file(MAKE_DIRECTORY ${header_output_prefix})

    foreach(filepath IN LISTS _SOURCES)
        cmake_path(IS_ABSOLUTE filepath is_filepath_absolute)
        if (NOT is_filepath_absolute)
            set(filepath ${CMAKE_CURRENT_SOURCE_DIR}/${filepath})
        endif()
        list(APPEND processed_filepaths ${filepath})

        get_filename_component(filename_we ${filepath} NAME_WE)
        list(APPEND output_sources ${source_output_prefix}/${filename_we}.cpp)
        list(APPEND output_headers ${header_output_prefix}/${filename_we}.hpp)
    endforeach()

    # Call Reflex and properly specify dependencies and outputs for CMake to track properly
    foreach(filename IN ZIP_LISTS processed_filepaths output_sources output_headers)
        add_custom_command(
            OUTPUT ${filename_1}
            COMMAND $<TARGET_FILE:Reflex::Reflex> ${filename_0} -o ${filename_1} --header-file=${filename_2}
            DEPENDS ${filename_0}
        )
    endforeach()

    target_sources(${_TARGET} PRIVATE ${output_sources})
    target_include_directories(${_TARGET} ${_INCLUDE_PRIVACY} ${source_output_prefix}/include)

    if (_REFLEXLIB_STATIC)
        target_link_libraries(${_TARGET} PRIVATE Reflex::ReflexLibStatic)
    else()
        target_link_libraries(${_TARGET} PRIVATE Reflex::ReflexLib)
    endif()
endfunction()
