function(REFlexGenLexerExec name lexer_input)
    set(generated_ll ${CMAKE_CURRENT_BINARY_DIR}/${name}.lex.yy.cpp)

    add_custom_command(
            OUTPUT ${generated_ll}
            COMMAND reflex-exec-initial ${lexer_input} -o ${generated_ll}
            DEPENDS ${lexer_input} reflex-exec-initial
    )

    add_executable(${name} ${generated_ll})
    target_link_libraries(${name} reflex)
endfunction()
