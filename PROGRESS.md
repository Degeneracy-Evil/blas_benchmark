# PROGRESS.md - BLAS Benchmark Development Progress

> Snapshot for quick state tracking. Keep details in project skills.

## 1. Project Snapshot

- Purpose: benchmark BLAS Level 1 to Level 3 performance on OpenBLAS.
- Stack: C++23, Xmake, OpenBLAS, CLI11, tomlplusplus, spdlog.
- Platform: Linux (Ubuntu 24.04 tested).
- Main binary target: `blas_benchmark`.

---

## 2. Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        main.cpp                             │
│  ┌─────────────┐  ┌────────────────┐                        │
│  │ CLI11 (CLI) │  │ Config Parser  │                        │
│  └─────────────┘  └────────────────┘                        │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   BenchmarkRunner                           │
│  ┌─────────────┐  ┌────────────────┐  ┌─────────────────┐   │
│  │ Level1 Test │  │ Level2 Test    │  │ Level3 Test     │   │
│  └─────────────┘  └────────────────┘  └─────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   BlasWrapper<T>                            │
│  ┌─────────────┐  ┌────────────────┐  ┌─────────────────┐   │
│  │ cblas_ddot  │  │ cblas_dgemv    │  │ cblas_dgemm     │   │
│  │ cblas_daxpy │  │                │  │                 │   │
│  │ cblas_dscal │  │                │  │                 │   │
│  └─────────────┘  └────────────────┘  └─────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
              ┌─────────────┴─────────────┐
              ▼                           ▼
┌─────────────────────────┐  ┌───────────────────────────────┐
│       Utilities         │  │       OutputFormatter         │
│  ┌─────────┐ ┌────────┐│  │  ┌──────────┐  ┌──────────┐  │
│  │ Timer   │ │SysInfo ││  │  │ Markdown  │  │   CSV    │  │
│  │(hdr-only)│ │        ││  │  └──────────┘  └──────────┘  │
│  └─────────┘ └────────┘│  └───────────────────────────────┘
└─────────────────────────┘
```

---

## 3. Implemented Features

- [x] BLAS Level 1 benchmarks: ddot, daxpy, dscal (double precision)
- [x] BLAS Level 2 benchmarks: dgemv (double precision)
- [x] BLAS Level 3 benchmarks: dgemm (double precision)
- [x] TOML configuration (config.toml) for function selection and weights
- [x] CLI parsing (CLI11) with short options (-1, -2, -3, -t, -c, -w, -o, -f, -C, -v, -s)
- [x] Cache flushing for cold-cache measurement
- [x] Markdown and CSV output formats
- [x] Output to file or stdout
- [x] Multi-threaded execution via openblas_set_num_threads()
- [x] Warmup iterations before measurement
- [x] Statistical summary: min/avg/max time (ms) and GFLOPS
- [x] System information collection and display (Linux)
- [x] Modular architecture: benchmark/, config/, output/, utils/
- [x] Header-only Timer and cache flush utilities
- [x] BlasWrapper<T> template with double and float specializations
- [x] flops namespace with constexpr FLOPS formulas
- [x] Xmake build system with release/debug modes

---

## 4. TODO

### High Priority
- [ ] **Implement weights scoring**: Weights are parsed from config.toml but never used in output. Decide on scoring formula and wire into OutputFormatter or BenchmarkRunner.
- [ ] **Remove duplicated size parsers**: `parse_size_pair` and `parse_size_triple` exist identically in both `main.cpp` and `config/config_parser.cpp`. Consolidate into config_parser as public API.

### Medium Priority
- [ ] **Add more BLAS Level 1 functions**: dnrm2, dswap, dasum, idamax
- [ ] **Add more BLAS Level 2 functions**: dger, dsymv, dtrmv
- [ ] **Add more BLAS Level 3 functions**: dsymm, dsyrk, dtrsm
- [ ] **Single precision dispatch**: BlasWrapper\<float\> exists but run_level1/2/3 only call double-precision variants. Add float dispatch or make it generic.
- [ ] **Add test suite**: Unit tests for ConfigParser, flops correctness, OutputFormatter, parse_size_pair/parse_size_triple edge cases.

### Low Priority
- [ ] **JSON output format**: Add "json" to OutputFormatter
- [ ] **Automated multi-size mode**: Run same function across multiple problem sizes in one invocation
- [ ] **Complex number support**: Extend BlasWrapper to complex types
- [ ] **Fortran BLAS API**: Benchmark Fortran-style interface (dgemm_, etc.)
- [ ] **GPU / XPU support**: Abstract compute backend for CUDA, ROCm, SYCL
- [ ] **macOS / BSD support**: SystemInfoCollector is Linux-only; add platform fallbacks
- [ ] **Clean up cache_block_size**: CLI option is parsed but never used in benchmark logic

---

## 5. Recent Changelog

### 2026-04-28
- **Decoupled OutputFormatter**: Moved `OutputFormatter` from `benchmark.h/.cpp` to its own `src/output/output_formatter.h/.cpp`.
- **Header-only Timer**: Deleted `timer.cpp`; `Timer` class is fully inline in `timer.h`.
- **Unified cache parsing**: Extracted `get_cache_for_level(level, default_size)` and `parse_cache_size()` helpers in `system_info.cpp`, eliminating duplicated L1/L2/L3 parsing logic.
- **Kept weights infrastructure**: `[weights]` in `config.toml` and `BenchmarkConfig` weights fields are preserved for future weighted scoring/aggregation.

### 2026-03-11
- Simplified project guidance files (`AGENTS.md` and `PROGRESS.md`) for faster context loading.
- Split detailed instructions into dedicated skills under `.opencode/skills/`.

### 2026-02-22
- Fixed cache size detection bug by trimming sysfs reads.
- Added short CLI options for levels and config (`-1`, `-2`, `-3`, `-C`).
- Added and refined GitHub Actions CI workflow.
