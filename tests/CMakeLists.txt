function(add_reflex_test name source)
    add_executable(reflex-test-${name} ${source})
    target_link_libraries(reflex-test-${name} reflex reflex-unicode)

    add_test(
            NAME reflex-test-${name}
            COMMAND reflex-test-${name}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

# add_reflex_test(lorem lorem.cpp)
add_reflex_test(streams streams.cpp)
# add_reflex_test(test test.cpp)
add_reflex_test(rtest rtest.cpp)
# add_reflex_test(ptest ptest.cpp)
# add_reflex_test(btest btest.cpp)
add_reflex_test(stest stest.cpp)
add_reflex_test(bits test_bits.cpp)
add_reflex_test(ranges test_ranges.cpp)
