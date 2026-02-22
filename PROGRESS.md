# PROGRESS.md - BLAS Benchmark Development Progress

> This file is for AI agents to quickly understand the project state without reading all source code.

## 1. Project Overview

**Purpose:** A C++23-based OpenBLAS performance testing framework for evaluating BLAS function performance across Level 1/2/3.

**Technology Stack:**
- Language: C++23
- Build System: Xmake (with Ninja recommended)
- Compiler: Clang 18+ (primary), GCC 13+ (should work)
- BLAS: OpenBLAS (system installed)
- Third-party: CLI11, tomlplusplus, spdlog (all via git submodules, header-only)

**Target Platform:** Linux (Ubuntu 24.04 tested)

---

## 2. Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        main.cpp                              │
│  ┌─────────────┐  ┌────────────────┐  ┌─────────────────┐   │
│  │ CLI11 (CLI) │  │ Config Parser  │  │ Output Formatter│   │
│  └─────────────┘  └────────────────┘  └─────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   BenchmarkRunner                            │
│  ┌─────────────┐  ┌────────────────┐  ┌─────────────────┐   │
│  │ Level1 Test │  │ Level2 Test    │  │ Level3 Test     │   │
│  └─────────────┘  └────────────────┘  └─────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   BlasWrapper<T>                             │
│  ┌─────────────┐  ┌────────────────┐  ┌─────────────────┐   │
│  │ cblas_ddot  │  │ cblas_dgemv    │  │ cblas_dgemm     │   │
│  │ cblas_daxpy │  │                │  │                 │   │
│  │ cblas_dscal │  │                │  │                 │   │
│  └─────────────┘  └────────────────┘  └─────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                      Utilities                               │
│  ┌─────────────┐  ┌────────────────┐  ┌─────────────────┐   │
│  │ Timer       │  │ SystemInfo     │  │ Cache Flush     │   │
│  └─────────────┘  └────────────────┘  └─────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. Implemented Features

### 3.1 Core Functionality
- [x] BLAS Level 1 benchmarks (ddot, daxpy, dscal)
- [x] BLAS Level 2 benchmarks (dgemv)
- [x] BLAS Level 3 benchmarks (dgemm)
- [x] Multi-threaded execution support
- [x] Warmup iterations before measurement
- [x] Cache flush for cold-cache measurement
- [x] Min/Avg/Max time statistics
- [x] GFLOPS calculation
- [x] TOML configuration file support
- [x] Command-line argument parsing
- [x] Markdown table output
- [x] CSV output format

### 3.2 System Information Detection
- [x] CPU model name
- [x] Physical/logical core count
- [x] CPU frequency
- [ ] L1/L2/L3 cache size (partially working, needs improvement)
- [x] Total memory
- [x] OS name

### 3.3 Future Features
- [ ] More BLAS functions (dsyrk, dtrsm, dger, etc.)
- [ ] Single precision (float) support
- [ ] Complex number support
- [ ] Fortran BLAS interface
- [ ] XPU (GPU) BLAS support
- [ ] JSON output format
- [ ] Score aggregation with weights
- [ ] Multi-size automated testing

---

## 4. Module Documentation

### 4.1 src/main.cpp
**Purpose:** Entry point, CLI parsing, configuration loading, orchestration

**Key Functions:**
- `parse_size_pair()`: Parse "M,N" string
- `parse_size_triple()`: Parse "M,N,K" string
- `print_system_info()`: Display system information
- `write_output()`: Write results to file or stdout

**Dependencies:** CLI11, spdlog, all internal modules

**CLI Options:**
| Option | Default | Description |
|--------|---------|-------------|
| -t, --threads | 1 | Number of OpenBLAS threads |
| -c, --cycle | 5 | Number of benchmark cycles |
| -w, --warmup | 3 | Number of warmup iterations |
| --level1 | - | Level 1 vector size |
| --level2 | - | Level 2 matrix size (M,N) |
| --level3 | - | Level 3 matrix size (M,N,K) |
| -o, --output | stdout | Output file path |
| -f, --format | markdown | Output format |
| --config | config.toml | Config file path |
| -v, --verbose | false | Enable debug logging |
| -s, --system-info | false | Show system info only |

### 4.2 src/benchmark/benchmark.h/cpp
**Purpose:** Benchmark orchestration and result formatting

**Key Classes:**
- `BenchmarkRunner`: Main benchmark execution class
- `OutputFormatter`: Format results as Markdown or CSV

**Key Structures:**
- `BenchmarkResult`: Single benchmark result (function, time, GFLOPS)
- `BenchmarkReport`: Complete report with system info and all results

**Key Methods:**
- `run_all()`: Execute all configured benchmarks
- `run_level1/2/3()`: Execute specific level benchmarks
- `set_threads()`: Configure OpenBLAS thread count

### 4.3 src/benchmark/blas_functions.h/cpp
**Purpose:** BLAS function wrappers and benchmark implementations

**Key Components:**
- `BlasPrecisionTraits<T>`: Type traits for precision (double/float)
- `BlasWrapper<T>`: Template wrapper for BLAS functions
- `flops` namespace: FLOPS calculation functions

**Benchmark Functions:**
- `benchmark_dot<T>()`: Benchmark dot product
- `benchmark_axpy<T>()`: Benchmark AXPY
- `benchmark_scal<T>()`: Benchmark SCAL
- `benchmark_gemv<T>()`: Benchmark matrix-vector multiply
- `benchmark_gemm<T>()`: Benchmark matrix-matrix multiply

**FLOPS Formulas:**
| Function | FLOPS |
|----------|-------|
| ddot | 2n |
| daxpy | 2n |
| dscal | n |
| dgemv | 2mn |
| dgemm | 2mnk |

### 4.4 src/config/config_parser.h/cpp
**Purpose:** Parse TOML configuration files

**Key Classes:**
- `ConfigParser`: TOML file parser
- `BenchmarkConfig`: Configuration structure

**Config Structure:**
```cpp
struct BenchmarkConfig {
    int threads;
    int cycles;
    int warmup;
    bool flush_cache;
    std::optional<size_t> level1_size;
    std::optional<pair<int,int>> level2_size;
    std::optional<tuple<int,int,int>> level3_size;
    std::vector<string> level1_functions;
    std::vector<string> level2_functions;
    std::vector<string> level3_functions;
    // ... weights
};
```

### 4.5 src/utils/timer.h
**Purpose:** High-precision timing and cache flush

**Key Functions:**
- `Timer::start()`, `Timer::stop()`: Measure time
- `Timer::elapsed_ms()`, `elapsed_ns()`: Get duration
- `flush_cache()`: Evict cache lines
- `get_default_cache_size()`: Get cache size for flushing

**Note:** Timer is header-only with inline functions.

### 4.6 src/utils/system_info.h/cpp
**Purpose:** Collect system hardware information

**Key Classes:**
- `SystemInfo`: Hardware information structure
- `SystemInfoCollector`: Information collector

**Detected Info:**
- CPU model, cores, frequency
- L1/L2/L3 cache sizes
- Total memory
- OS name

---

## 5. Code Conventions

### 5.1 Naming
| Type | Style | Example |
|------|-------|---------|
| Namespace | snake_case | `blas_benchmark` |
| Class | PascalCase | `BenchmarkRunner` |
| Function | snake_case | `run_benchmark()` |
| Variable | snake_case | `num_threads` |
| Constant | UPPER_SNAKE_CASE | `MAX_ITERATIONS` |
| Member | m_ prefix | `m_config` |
| Template param | PascalCase | `typename T` |

### 5.2 Formatting
- Indent: 4 spaces (no tabs)
- Braces: Allman style (newline)
- Line width: 100-120 characters
- Comments: English for key steps

### 5.3 Headers
```cpp
#pragma once  // Always use this

#include <system_headers>  // Alphabetical
#include <third_party>
#include "project/headers"
```

### 5.4 Output
- Use `std::print` / `std::println` (C++23)
- Use `std::format` for string building
- Use `spdlog` for logging

---

## 6. Third-Party Libraries

### 6.1 CLI11 (thirdparty/CLI11)
- **Purpose:** Command-line argument parsing
- **Header:** `#include <CLI/CLI.hpp>`
- **Version:** Latest from git submodule
- **Usage:** Single header, header-only

### 6.2 tomlplusplus (thirdparty/tomlplusplus)
- **Purpose:** TOML configuration file parsing
- **Header:** `#include <toml.hpp>`
- **Version:** v3.4.0+
- **Usage:** Single header, header-only

### 6.3 spdlog (thirdparty/spdlog)
- **Purpose:** Logging
- **Header:** `#include <spdlog/spdlog.h>`
- **Usage:** Header-only mode (no SPDLOG_COMPILED_LIB)
- **Log Levels:** debug, info, warn, error

### 6.4 OpenBLAS (system)
- **Purpose:** BLAS implementation
- **Header:** `#include <cblas.h>`
- **Install:** `sudo apt install libopenblas-dev`
- **Path:** `/usr/include/x86_64-linux-gnu/openblas-pthread/cblas.h`

---

## 7. Build System

### 7.1 xmake.lua
```lua
set_languages("c++23")
set_toolchains("clang")

add_includedirs("thirdparty/CLI11/include")
add_includedirs("thirdparty/tomlplusplus")
add_includedirs("thirdparty/spdlog/include")

add_links("openblas")
```

### 7.2 Commands
```bash
xmake f -m release    # Configure
xmake                 # Build
xmake clean           # Clean
xmake cleanall        # Deep clean
xmake compdb          # Generate compile_commands.json for LSP
xmake run cblas_benchmark --help
```

### 7.3 LSP Support (.clangd)
- Uses `compile_commands.json` generated by `xmake compdb`
- Configured to use libc++ for C++23 std::print support
- Suppresses noisy warnings like `-Wunused-command-line-argument`
- Disables some clang-tidy checks (modernize, cppcoreguidelines, bugprone)

### 7.4 CI/CD (GitHub Actions)
- Workflow: `.github/workflows/main.yml`
- Runner: `ubuntu-24.04`
- Triggers: push to main/master, pull requests
- Steps: Install Clang 18 → Install Xmake → Build → Run tests
- Caches xmake build artifacts for faster subsequent runs

---

## 8. Known Issues

### 8.1 Cache Size Detection
- **Problem:** L1/L2/L3 cache sizes not detected correctly on some systems
- **Location:** `src/utils/system_info.cpp`
- **Impact:** Cache flush may use default 16MB instead of actual cache size
- **Fix Needed:** Improve sysfs parsing or use fallback methods

### 8.2 C++23 std::print Support
- **Problem:** Requires Clang 18+ with proper C++23 support
- **Workaround:** Code uses std::println from <print> header
- **Note:** GCC may need additional flags

---

## 9. TODO List

### High Priority
- [ ] Fix cache size detection
- [ ] Add more BLAS functions (dsyrk, dtrsm)
- [ ] Improve system info detection

### Medium Priority
- [ ] Add single precision support
- [ ] Add JSON output format
- [ ] Add automated multi-size testing

### Low Priority
- [ ] Complex number support
- [ ] Fortran BLAS interface
- [ ] GPU BLAS support

---

## 10. Changelog

### 2026-02-22 (2)
- Added GitHub Actions CI/CD workflow (.github/workflows/main.yml)
- Added compdb task to xmake for compile_commands.json generation
- Updated .clangd for better LSP support with libc++
- Added post-development documentation sync rule to AGENTS.md

### 2026-02-22
- Migrated to git submodules for all third-party libraries
- Replaced std::cout with std::println (C++23)
- Added cleanall task to xmake
- Updated README documentation
- Created PROGRESS.md

### 2026-02-22 (Initial)
- Basic project structure
- BLAS Level 1/2/3 benchmarks
- TOML configuration support
- Markdown/CSV output
- Cache flush support
- System info collection

---

## 11. Quick Reference for AI Agents

### Key Files to Read
1. `src/main.cpp` - Entry point and CLI
2. `src/benchmark/benchmark.h` - Main benchmark interface
3. `src/benchmark/blas_functions.h` - BLAS wrappers
4. `config.toml` - Default configuration

### Common Tasks

**Add new BLAS function:**
1. Add wrapper in `blas_functions.h/cpp`
2. Add FLOPS calculation in `flops` namespace
3. Add benchmark function
4. Update `benchmark.cpp` to call new function
5. Update `config.toml` defaults

**Add new CLI option:**
1. Add variable in `main.cpp`
2. Add `app.add_option()` call
3. Update help text in README

**Modify output format:**
1. Edit `OutputFormatter::to_markdown()` or `to_csv()` in `benchmark.cpp`
