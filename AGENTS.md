# AGENTS.md - BLAS Benchmark Project Entry Guide

This project is a C++23 OpenBLAS benchmark tool for BLAS Level 1 to Level 3 performance testing on Linux.

## Quick Build and Run

```bash
xmake f -m release
xmake
xmake run blas_benchmark --help
xmake run blas_benchmark -1 10000 -2 128,128 -3 128,128,128 -c 2 -w 1 -t 2
```

## Core Conventions

- Naming: namespace `snake_case`, class `PascalCase`, function and variable `snake_case`, member `m_` prefix.
- Formatting: 4-space indentation, Allman braces, around 100-120 characters per line.
- Headers: use `#pragma once`.
- Comments: keep key implementation comments in English.
- Output: prefer C++23 `std::print` and `std::println`.

## Key Dependencies

- OpenBLAS (system package: `libopenblas-dev`)
- CLI11 (submodule)
- tomlplusplus (submodule)
- spdlog (submodule)

## Skill Routing

- Load `blas-benchmark` for architecture and code style details.
- Load `blas-functions` for BLAS kernel extension and FLOPS rules.
- Load `build-and-test` for build commands, CLI details, and CI troubleshooting.
- Load `documentation` for README and progress sync rules.

## Documentation Update Policy

After behavior changes, keep docs synchronized:

1. Update `README.zh-CN.md` first.
2. Mirror updates in `README.md`.
3. Update `PROGRESS.md` status and changelog.
4. Update this file only when global agent workflow changes.

## Source Tree Layout

```
src/
├── main.cpp                       # Entry point, CLI, output dispatch
├── benchmark/
│   ├── benchmark.h/.cpp           # BenchmarkRunner: run_level1/2/3
│   └── blas_functions.h/.cpp      # BlasWrapper<T>, flops::*, benchmark_*()
├── config/
│   └── config_parser.h/.cpp       # ConfigParser, BenchmarkConfig
├── output/
│   └── output_formatter.h/.cpp    # OutputFormatter (markdown/csv)
└── utils/
    ├── system_info.h/.cpp         # SystemInfoCollector
    └── timer.h                    # Header-only: Timer, flush_cache()
```
