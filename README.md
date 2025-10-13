# blas_benchmark

## 1. Project Overview

This project is a **C++23**-based **OpenBLAS** performance testing framework designed for evaluating and analyzing the performance of BLAS (Basic Linear Algebra Subprograms) functions across different levels (Level 1, 2, 3).

**Core Features:**
- Benchmark performance of BLAS Level 1, 2, and 3 functions
- Support custom problem sizes, thread counts, and test iterations
- Include warmup runs and multiple measurements for stable results
- Output results in Markdown or CSV format
- Built with C++23 standard and Xmake build system

**Environment:** Pure command-line application, primarily supports **Linux** systems (e.g., Ubuntu 24.04).

## 2. Functional Requirements

### 2.1 Core Testing Features
- Test performance of BLAS functions at different levels (L1, L2, L3)
- Select partial functions per level via `config.toml`
- Support **single-threaded** and **multi-threaded** execution
- Test **multiple problem sizes**
- Calculate and output **average execution time (ms)** and **GFLOPS**
- Generate comprehensive performance scores (optional/future)

### 2.2 Test Configuration
- **Problem Size:**
  - **Level 1 (Vectors):** e.g., `10^4`, `10^7` elements. Use `-l1,--level1 <num1>`
  - **Level 2 (Matrix-Vector):** e.g., `128x128`, `1024x1024` matrices. Use `-l2,--level2 <num1,num2>`
  - **Level 3 (Matrix-Matrix):** e.g., `(128,128,128)`, `(4096,4096,4096)`. Use `-l3,--level3 <num1,num2,num3>`
- **Thread Configuration:** `-t,--threads <num>` (default: 1 thread, overrides `OPENBLAS_NUM_THREADS`)
- **Iterations:** `-c,--cycle <num>` test repetitions for averaging
- **Warmup Runs:** `-w,--warmup <num>` (default: 3)

### 2.3 Output Requirements
- **stdout:** Default Markdown-formatted results
- **File Output:** Save to file via `-o,--output <filename>`
- **Output Format:** Select format with `-f,--format <markdown|csv>`

*Markdown outputs include system configuration:*
- OS distribution
- Kernel version
- Compiler type/version
- CPU configuration
- Memory configuration
- etc.

**Performance Table Columns:**
| Column            | Description                                     |
| :---------------- | :---------------------------------------------- |
| **Function**      | BLAS function name (e.g., `cblas_dgemm`)        |
| **Config**        | Test parameters (e.g., `M=1024,N=1024,K=1024`) |
| **Threads**       | Thread count used                               |
| **Avg Time (ms)** | Average execution time (milliseconds)           |
| **GFLOPS**        | Performance metric based on FLOPs/time          |

Function score: $ score_f = GFLOPS \times weight $
(Initial weights default to 1.0)

## 3. Dependencies
- **[OpenBLAS](https://www.openblas.net/):** High-performance BLAS implementation
- **[CLI11](https://github.com/CLIUtils/CLI11):** Command-line parsing (header-only)
- **[toml++](https://github.com/marzer/tomlplusplus):** TOML configuration parsing

## 4. Build & Run
Uses **Xmake** build system:
1. **Install Xmake:** See [Xmake docs](https://xmake.io/#/guide/installation)
2. **Build:** Run `xmake` in project root
3. **Execute Benchmark:**
```bash
# Basic run
xmake run blas_benchmark
# Example
xmake run blas_benchmark -t 8 -c 10 -f csv -o gemm_results.csv
```

*Use `xmake run blas_benchmark --help` for full options*

## 5. Testing Recommendations
- **System State:** Ensure low system load and stable CPU frequency (consider `cpupower` performance mode)
- **Warmup:** Framework includes warmup. For strict tests, pre-run full test set
- **Size Selection:** Cover L1 cache to main memory ranges (e.g., 4096x4096x4096 for Level 3 peak performance)
- **Threads:** Test single-thread (`--threads 1`) and multi-thread (e.g., `--threads $(nproc)`)

## 6. FLOPS Calculation
### Level 1
TODO
### Level 2
TODO
### Level 3
| Function   | Formula       |
| :--------- | :------------ |
| dgemm      | $O(2MNK)$     |

## 7. Configuration Example
```toml
# config.toml
# BLAS Benchmark Configuration File

[functions]
level1 = [
    "cblas_ddot",
    "cblas_daxpy",
]

level2 = [
    "cblas_dgemv",
]

level3 = [
    "cblas_dgemm",
]

[weights.level1]
cblas_ddot = 1.0
cblas_daxpy = 1.0

[weights.level2]
cblas_dgemv = 1.5

[weights.level3]
cblas_dgemm = 2.0
```

## 8. 项目结构
```bash
blas_benchmark/
├── config.toml                # Default config
├── LICENSE
├── README.md
├── src
│   ├── benchmark
│   │   ├── benchmark.cpp      # Core benchmarking
│   │   ├── benchmark.h
│   │   ├── blas_functions.cpp
│   │   └── blas_functions.h
│   ├── config
│   │   ├── config_parser.cpp  # TOML parsing
│   │   └── config_parser.h
│   ├── main.cpp
│   └── utils
│       ├── system_info.cpp
│       ├── system_info.h
│       ├── timer.cpp
│       └── timer.h
├── thirdparty
│   ├── CLI11
│   │   └── CLI11.hpp          # CLI11
│   └── toml++
│       └── toml.hpp           # toml++
└── xmake.lua                  # Xmake Build config
```

## 9. Notes
Personal project for learning OpenBLAS, C++23, Git, Xmake, toml++, and CLI11.