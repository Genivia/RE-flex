add_executable(reflex-exec
        reflex.cpp
        reflex.h)

target_link_libraries(reflex-exec
        reflex reflex-unicode)

add_executable(reflex-exec-initial
        reflex.cpp
        reflex.h
        ../unicode/initial/language_scripts.cpp
        ../unicode/initial/letter_scripts.cpp
        ../unicode/initial/block_scripts.cpp)

target_link_libraries(reflex-exec-initial reflex)
