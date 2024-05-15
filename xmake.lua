
target("reflexlib")
    set_kind("static")
    set_basename("reflex")
    set_languages("cxx11")
    add_includedirs("include")
    add_headerfiles("include/reflex/*.h", {prefixdir = "reflex"})
    add_files("lib/*.cpp")
    add_files("unicode/*.cpp")
target_end()

target("reflex")
    set_kind("binary")
    set_languages("cxx11")
    add_includedirs("include")
    add_files("src/*.cpp")
    add_deps("reflexlib")
target_end()
    