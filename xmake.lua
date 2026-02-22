-- BLAS Benchmark Build Configuration
-- Xmake build system with C++23 support

add_rules("mode.debug", "mode.release")

set_languages("c++23")
set_toolchains("clang")

-- Project include directories
add_includedirs("src")
add_includedirs("thirdparty/CLI11/include")
add_includedirs("thirdparty/tomlplusplus")
add_includedirs("thirdparty/spdlog/include")

target("cblas_benchmark")
    set_kind("binary")
    add_files("src/*.cpp")
    add_files("src/**/*.cpp")
    
    -- Use libc++ for C++23 std::print support
    add_cxxflags("-stdlib=libc++")
    add_ldflags("-stdlib=libc++", "-lc++abi")
    
    -- OpenBLAS linkage
    add_includedirs("/usr/include/x86_64-linux-gnu/openblas-pthread")
    add_links("openblas")
    
    -- Compiler warnings
    add_cxxflags("-Wall", "-Wextra", "-Wpedantic")
    
    -- Release mode optimizations
    if is_mode("release") then
        add_cxxflags("-O3", "-march=native", "-ffast-math")
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
