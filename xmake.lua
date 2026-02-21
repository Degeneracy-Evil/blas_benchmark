add_rules("mode.debug", "mode.release")

set_languages("c++23")
set_toolchains("clang")

target("cblas_benchmark")
    set_kind("binary")
    add_files("src/*.cpp")