-- BLAS Benchmark Build Configuration
-- Xmake build system with C++23 support

add_rules("mode.debug", "mode.release")

set_languages("c++23")
set_toolchains("clang")

-- ============================================================================
-- Thirdparty: compiled static libraries from git submodules
-- ============================================================================

target("spdlog")
    set_kind("static")
    set_languages("c++17")
    add_files("thirdparty/spdlog/src/*.cpp")
    add_includedirs("thirdparty/spdlog/include", {public = true})
    add_defines("SPDLOG_COMPILED_LIB", {public = true})
    add_cxxflags("-stdlib=libc++")
    add_syslinks("pthread")

target("tomlplusplus")
    set_kind("static")
    set_languages("c++17")
    add_files("thirdparty/tomlplusplus/src/toml.cpp")
    add_includedirs("thirdparty/tomlplusplus", {public = true})
    add_defines("TOML_HEADER_ONLY=0", {public = true})
    add_cxxflags("-stdlib=libc++")

target("cli11")
    set_kind("static")
    set_languages("c++17")
    add_files("thirdparty/CLI11/src/Precompile.cpp")
    add_includedirs("thirdparty/CLI11/include", {public = true})
    add_cxxflags("-stdlib=libc++")

-- ============================================================================
-- Main target
-- ============================================================================

target("blas_benchmark")
    set_kind("binary")
    add_files("src/*.cpp")
    add_files("src/**/*.cpp")

    add_includedirs(".")
    add_includedirs("src")

    -- Use libc++ for C++23 std::print support
    add_cxxflags("-stdlib=libc++")
    add_ldflags("-stdlib=libc++", "-lc++abi")

    -- Linker and runtime
    add_ldflags("-fuse-ld=lld")
    add_cxxflags("-rtlib=compiler-rt")

    -- Link compiled thirdparty libs
    add_deps("spdlog", "tomlplusplus", "cli11")

    -- OpenBLAS linkage
    add_includedirs("/usr/include/x86_64-linux-gnu")
    add_links("openblas")

    -- Compiler warnings
    add_cxxflags("-Wall", "-Wextra", "-Wpedantic", "-Werror")

    -- Release mode optimizations
    if is_mode("release") then
        add_cxxflags("-O3", "-march=native")
    end

-- Custom clean task for thorough cleanup
task("cleanall")
    set_menu {
        usage = "xmake cleanall",
        description = "Clean all build artifacts and cache"
    }
    on_run(function()
        os.rm("$(buildir)")
        os.rm(".xmake")
        os.rm("compile_commands.json")
        print("All build artifacts cleaned!")
    end)
task_end()

-- Generate compile_commands.json for LSP (clangd, etc.)
task("compdb")
    set_menu {
        usage = "xmake compdb",
        description = "Generate compile_commands.json for LSP"
    }
    on_run(function()
        import("core.project.task")
        task.run("project", {kind = "compile_commands"})
        print("compile_commands.json generated!")
    end)
task_end()
