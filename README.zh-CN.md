# blas_benchmark

## 1. 项目概述

本项目是一个基于 **C++23** 的 **OpenBLAS** 性能测试框架，用于对 BLAS (Basic Linear Algebra Subprograms) 各级别函数（Level 1, 2, 3）进行性能评估和分析。

**主要功能：**
- 测试 BLAS Level 1、Level 2 和 Level 3 函数的性能
- 支持自定义问题规模、线程数和测试循环次数
- 包含预热运行和缓存刷新，确保测量准确性
- 支持 Markdown 和 CSV 格式的结果输出
- 使用 C++23 标准和 Xmake 构建系统（推荐使用 Ninja）
- 目前支持 CPU 的 CBLAS 框架，未来将要继续支持 Fortran 和 XPU 异构 BLAS 测试

**运行环境：** 本框架为纯命令行程序，主要支持 **Linux** 系统（例如 Ubuntu 24.04）。

## 2. 功能需求

### 2.1 核心测试功能
- 测试不同级别（L1, L2, L3）BLAS 函数的性能
- 每个级别选取**部分**函数进行测试，通过 `config.toml` 配置文件进行指定
- 支持 **单线程** 和 **多线程** 执行模式
- 支持测试 **多种问题规模**
- 计算并输出每个测试用例的 **最小/平均/最大执行时间 (ms)** 和 **GFLOPS**
- 缓存刷新选项，用于测量冷缓存性能

### 2.2 测试配置
- **问题规模 (Problem Size):**
  - **Level 1 (向量):** 例如 `10^4`, `10^7` 个元素。使用 `--level1 <num1>` 进行指定
  - **Level 2 (矩阵-向量):** 例如 `128x128`, `1024x1024` 矩阵。使用 `--level2 <num1,num2>` 进行指定
  - **Level 3 (矩阵-矩阵):** 例如 `(128, 128, 128)`, `(4096, 4096, 4096)`。使用 `--level3 <num1,num2,num3>` 进行指定
- **线程配置 (Thread Configuration):** `-t,--threads <num>` 指定使用的线程数 (`1` 表示单线程，默认为单线程)。该选项将强制覆盖 `OPENBLAS_NUM_THREADS`
- **测试循环次数 (Iterations):** `-c,--cycle <num>` 指定每个测试用例运行的次数（用于计算平均时间）
- **预热次数 (Warmup):** `-w,--warmup <num>` 指定预热次数。默认为 3 次

### 2.3 输出要求
- **标准输出 (stdout):** 默认在终端打印 **Markdown 格式**的性能结果
- **文件输出 (File Output):** 支持通过 `-o,--output <filename>` 参数将结果保存到指定文件
- **输出格式 (Output Format):** 支持通过 `-f,--format <markdown|csv>` 参数选择输出格式（Markdown 或 CSV）

**性能结果表格包含以下列：**
| 列名              | 描述                                                |
| :---------------- | :-------------------------------------------------- |
| **Function**      | 测试的 BLAS 函数名 (例如 `ddot`, `dgemm`)           |
| **Config**        | 测试配置参数 (例如 `M=1024,N=1024,K=1024`)          |
| **Threads**       | 执行测试时使用的线程数                              |
| **Min Time (ms)** | 最小执行时间（毫秒）                                |
| **Avg Time (ms)** | 平均执行时间（毫秒），基于多次运行计算得出          |
| **Max Time (ms)** | 最大执行时间（毫秒）                                |
| **GFLOPS**        | 根据函数操作的理论浮点运算次数 (FLOPs) 和时间计算得出的性能指标 |

## 3. 依赖库

### 3.1 系统依赖
- **[OpenBLAS](https://www.openblas.net/):** 提供高性能 BLAS 实现
  ```bash
  # Ubuntu/Debian
  sudo apt install libopenblas-dev
  ```

### 3.2 捆绑依赖 (Git Submodules)
- **[CLI11](https://github.com/CLIUtils/CLI11):** 用于解析命令行参数 (header-only)
- **[toml++](https://github.com/marzer/tomlplusplus):** 解析 TOML 配置文件 (header-only)
- **[spdlog](https://github.com/gabime/spdlog):** 日志库 (header-only 模式)

## 4. 构建与运行

### 4.1 克隆项目（包含 Submodules）
```bash
# 克隆时同时获取 submodules
git clone --recursive https://github.com/Degeneracy-Evil/blas_benchmark.git

# 如果已经克隆，初始化 submodules
git submodule update --init --recursive
```

### 4.2 构建
本项目使用 **Xmake** 作为构建系统：
```bash
# 安装 Xmake (参考 https://xmake.io/#/guide/installation)

# 配置 (推荐 Release 模式)
xmake f -m release

# 构建
xmake

# 清理构建产物
xmake clean

# 彻底清理（包括缓存）
xmake cleanall
```

### 4.3 运行
```bash
# 查看帮助
xmake run cblas_benchmark --help

# 基本运行
xmake run cblas_benchmark --level3 1024,1024,1024

# 完整示例
xmake run cblas_benchmark -t 8 -c 10 --level3 2048,2048,2048 -f csv -o results.csv

# 仅显示系统信息
xmake run cblas_benchmark -s
```

## 5. 测试建议
- **系统状态:** 在测试前确保系统负载较低且 CPU 频率稳定（可考虑使用 cpupower 设置性能模式）
- **预热:** 框架内置预热是好的实践。对于更严格的测试，可考虑在整体测试开始前运行一次完整的测试集进行额外预热
- **规模选择:** 选择的规模应能覆盖从 L1 缓存到主存的不同范围，以揭示内存带宽和计算强度的瓶颈。4096x4096x4096 对于 Level 3 是测试峰值计算能力的典型规模
- **线程数:** 测试单线程 (`--threads 1`) 和多线程 (例如 `--threads $(nproc)`) 以评估并行扩展性

## 6. FLOPS 计算方式

### Level 1 (向量-向量)
| 函数名 | 计算公式 |
| :----- | :------- |
| ddot   | $2n$     |
| daxpy  | $2n$     |
| dscal  | $n$      |

### Level 2 (矩阵-向量)
| 函数名 | 计算公式 |
| :----- | :------- |
| dgemv  | $2mn$    |

### Level 3 (矩阵-矩阵)
| 函数名 | 计算公式 |
| :----- | :------- |
| dgemm  | $2mnk$   |

**GFLOPS 计算:** $GFLOPS = \frac{FLOPs}{time_{sec} \times 10^9}$

## 7. 配置文件示例
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

## 8. 项目结构
```bash
blas_benchmark/
├── config.toml                # 默认的 TOML 配置文件
├── LICENSE
├── README.md                  # 英文文档
├── README.zh-CN.md            # 中文文档
├── PROGRESS.md                # 开发进度记录
├── AGENTS.md                  # AI 代理指南
├── src
│   ├── main.cpp               # 程序入口 + CLI
│   ├── benchmark/
│   │   ├── benchmark.cpp      # 基准测试
│   │   ├── benchmark.h
│   │   ├── blas_functions.cpp # BLAS 函数封装
│   │   └── blas_functions.h
│   ├── config/
│   │   ├── config_parser.cpp  # TOML 配置解析
│   │   └── config_parser.h
│   └── utils/
│       ├── system_info.cpp    # 系统信息收集
│       ├── system_info.h
│       ├── timer.cpp          # 高精度计时
│       └── timer.h
├── thirdparty/                # Git submodules
│   ├── CLI11/                 # 命令行解析
│   ├── tomlplusplus/          # TOML 解析
│   └── spdlog/                # 日志库
└── xmake.lua                  # Xmake 构建配置
```

## 9. 编译器要求
- 需要 **C++23** 支持
- 推荐 **Clang 18+** (已测试)
- GCC 13+ 理论上可行但未测试

## 10. 备注
该项目主要是用于个人学习 OpenBLAS, C++23, Git, Xmake, toml++, CLI11, spdlog, ninja 的使用。
