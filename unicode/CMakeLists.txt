function(GenScript name input_txt input_ll)
    REFlexGenLexerExec(gen_${name}_scripts_cpp ${CMAKE_CURRENT_SOURCE_DIR}/${input_ll})

    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${name}_scripts.cpp
            COMMAND $<TARGET_FILE:gen_${name}_scripts_cpp>
            < ${CMAKE_CURRENT_SOURCE_DIR}/Blocks.txt
            > ${CMAKE_CURRENT_BINARY_DIR}/${name}_scripts.cpp
            DEPENDS ${input} gen_${name}_scripts_cpp
    )
endfunction()

GenScript(block Blocks.txt block_scripts.l)
GenScript(language Scripts.txt language_scripts.l)
GenScript(letter UnicodeData.txt letter_scripts.l)

add_library(reflex-unicode STATIC
        ${CMAKE_CURRENT_BINARY_DIR}/language_scripts.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/letter_scripts.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/block_scripts.cpp)
