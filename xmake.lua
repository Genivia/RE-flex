
target("libreflex")
    set_kind("static")
    set_languages("cxx11")
    add_includedirs("include")
    add_files("lib/*.cpp")
    add_files("unicode/*.cpp")
target_end()

target("reflex")
    set_kind("binary")
    set_languages("cxx11")
    add_includedirs("include")
    add_files("src/*.cpp")
    add_deps("libreflex")
target_end()
    