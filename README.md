# blas_benchmark

## 1. Project Overview

This project is a **C++23**-based **OpenBLAS** performance testing framework designed for evaluating and analyzing the performance of BLAS (Basic Linear Algebra Subprograms) functions across different levels (Level 1, 2, 3).

**Core Features:**
- Benchmark performance of BLAS Level 1, 2, and 3 functions
- Support custom problem sizes, thread counts, and test iterations
- Include warmup runs and cache flush for accurate measurements
- Output results in Markdown or CSV format
- Built with C++23 standard and Xmake build system (Recommend Ninja)
- Currently supports CPU CBLAS framework, with future support planned for Fortran and XPU heterogeneous BLAS testing

**Environment:** Pure command-line application, primarily supports **Linux** systems (e.g., Ubuntu 24.04).

## 2. Functional Requirements

### 2.1 Core Testing Features
- Test performance of BLAS functions at different levels (L1, L2, L3)
- Select partial functions per level via `config.toml`
- Support **single-threaded** and **multi-threaded** execution
- Test **multiple problem sizes**
- Calculate and output **min/avg/max execution time (ms)** and **GFLOPS**
- Cache flush option for cold-cache performance measurement

### 2.2 Test Configuration
- **Problem Size:**
  - **Level 1 (Vectors):** e.g., `10^4`, `10^7` elements. Use `--level1 <num1>`
  - **Level 2 (Matrix-Vector):** e.g., `128x128`, `1024x1024` matrices. Use `--level2 <num1,num2>`
  - **Level 3 (Matrix-Matrix):** e.g., `(128,128,128)`, `(4096,4096,4096)`. Use `--level3 <num1,num2,num3>`
- **Thread Configuration:** `-t,--threads <num>` (default: 1 thread, overrides `OPENBLAS_NUM_THREADS`)
- **Iterations:** `-c,--cycle <num>` test repetitions for averaging
- **Warmup Runs:** `-w,--warmup <num>` (default: 3)

### 2.3 Output Requirements
- **stdout:** Default Markdown-formatted results
- **File Output:** Save to file via `-o,--output <filename>`
- **Output Format:** Select format with `-f,--format <markdown|csv>`

**Performance Table Columns:**
| Column            | Description                                     |
| :---------------- | :---------------------------------------------- |
| **Function**      | BLAS function name (e.g., `ddot`, `dgemm`)      |
| **Config**        | Test parameters (e.g., `M=1024,N=1024,K=1024`) |
| **Threads**       | Thread count used                               |
| **Min Time (ms)** | Minimum execution time (milliseconds)           |
| **Avg Time (ms)** | Average execution time (milliseconds)           |
| **Max Time (ms)** | Maximum execution time (milliseconds)           |
| **GFLOPS**        | Performance metric based on FLOPs/time          |

## 3. Dependencies

### 3.1 System Dependencies
- **[OpenBLAS](https://www.openblas.net/):** High-performance BLAS implementation
  ```bash
  # Ubuntu/Debian
  sudo apt install libopenblas-dev
  ```

### 3.2 Bundled Dependencies (Git Submodules)
- **[CLI11](https://github.com/CLIUtils/CLI11):** Command-line parsing (header-only)
- **[toml++](https://github.com/marzer/tomlplusplus):** TOML configuration parsing (header-only)
- **[spdlog](https://github.com/gabime/spdlog):** Logging library (header-only mode)

## 4. Build & Run

### 4.1 Clone with Submodules
```bash
# Clone with submodules
git clone --recursive https://github.com/Degeneracy-Evil/blas_benchmark.git

# Or if already cloned, initialize submodules
git submodule update --init --recursive
```

### 4.2 Build
Uses **Xmake** build system:
```bash
# Install Xmake (see https://xmake.io/#/guide/installation)

# Configure (Release mode recommended)
xmake f -m release

# Build
xmake

# Clean build artifacts
xmake clean

# Clean all (including cache)
xmake cleanall
```

### 4.3 Run
```bash
# Show help
xmake run cblas_benchmark --help

# Basic run
xmake run cblas_benchmark --level3 1024,1024,1024

# Full example
xmake run cblas_benchmark -t 8 -c 10 --level3 2048,2048,2048 -f csv -o results.csv

# Show system info only
xmake run cblas_benchmark -s
```

## 5. Testing Recommendations
- **System State:** Ensure low system load and stable CPU frequency (consider `cpupower` performance mode)
- **Warmup:** Framework includes warmup. For strict tests, pre-run full test set
- **Size Selection:** Cover ranges from L1 cache to main memory (e.g., 4096x4096x4096 for Level 3 peak performance)
- **Threads:** Test single-thread (`--threads 1`) and multi-thread (e.g., `--threads $(nproc)`)

## 6. FLOPS Calculation

### Level 1 (Vector-Vector)
| Function | FLOPS Formula |
| :------- | :------------ |
| ddot     | $2n$          |
| daxpy    | $2n$          |
| dscal    | $n$           |

### Level 2 (Matrix-Vector)
| Function | FLOPS Formula |
| :------- | :------------ |
| dgemv    | $2mn$         |

### Level 3 (Matrix-Matrix)
| Function | FLOPS Formula |
| :------- | :------------ |
| dgemm    | $2mnk$        |

**GFLOPS Calculation:** $GFLOPS = \frac{FLOPs}{time_{sec} \times 10^9}$

## 7. Configuration Example
```toml
# config.toml
# BLAS Benchmark Configuration File

[functions]
level1 = ["cblas_ddot", "cblas_daxpy", "cblas_dscal"]
level2 = ["cblas_dgemv"]
level3 = ["cblas_dgemm"]

[weights.level1]
cblas_ddot = 1.0
cblas_daxpy = 1.0
cblas_dscal = 0.5

[weights.level2]
cblas_dgemv = 1.5

[weights.level3]
cblas_dgemm = 2.0

[defaults]
threads = 1
warmup = 3
cycles = 5
flush_cache = true
level1_size = 1000000
level2_m = 1024
level2_n = 1024
level3_m = 1024
level3_n = 1024
level3_k = 1024
```

## 8. Project Structure
```bash
blas_benchmark/
├── config.toml                # Default config
├── LICENSE
├── README.md
├── README.zh-CN.md
├── PROGRESS.md                # Development progress
├── AGENTS.md                  # AI agent guide
├── src
│   ├── main.cpp               # Entry point + CLI
│   ├── benchmark/
│   │   ├── benchmark.cpp      # Core benchmarking
│   │   ├── benchmark.h
│   │   ├── blas_functions.cpp # BLAS wrapper + benchmarks
│   │   └── blas_functions.h
│   ├── config/
│   │   ├── config_parser.cpp  # TOML parsing
│   │   └── config_parser.h
│   └── utils/
│       ├── system_info.cpp    # System info collection
│       ├── system_info.h
│       ├── timer.cpp          # High-precision timer
│       └── timer.h
├── thirdparty/                # Git submodules
│   ├── CLI11/                 # Command-line parsing
│   ├── tomlplusplus/          # TOML parsing
│   └── spdlog/                # Logging
└── xmake.lua                  # Build config
```

## 9. Compiler Requirements
- **C++23** support required
- **Clang 18+** recommended (tested)
- GCC 13+ should work but not tested

## 10. Notes
Personal project for learning OpenBLAS, C++23, Git, Xmake, toml++, CLI11, spdlog, ninja.
