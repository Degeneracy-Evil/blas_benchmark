# blas_benchmark

## 1. 项目概述

本项目是一个基于 **C++23** 的 **OpenBLAS** 性能测试框架，用于对 BLAS (Basic Linear Algebra Subprograms) 各级别函数（Level 1, 2, 3）进行性能评估和分析。

**主要功能：​**

-   测试 BLAS Level 1、Level 2 和 Level 3 函数的性能
-   支持自定义问题规模、线程数和测试循环次数
-   包含预热运行和多次测量取平均，提高测试结果的稳定性
-   支持 Markdown 和 CSV 格式的结果输出
-   使用 C++23 标准和 Xmake 构建系统

**运行环境：** 本框架为纯命令行程序，主要支持 **Linux** 系统（例如 Ubuntu 24.04）。

## 2. 功能需求

### 2.1 核心测试功能

-   测试不同级别（L1, L2, L3）BLAS 函数的性能。
-   每个级别选取**部分**函数进行测试，通过`config.toml`配置文件进行指定。
-   支持 **单线程** 和 **多线程** 执行模式。
-   支持测试 **多种问题规模**。
-   计算并输出每个测试用例的 **平均执行时间 (ms)** 和 **GFLOPS**。
-   最终可汇总得出 BLAS 库在特定配置下的综合性能评分（可选或未来扩展）。

### 2.2 测试配置

-   **问题规模 (Problem Size):**
    -   **Level 1 (向量):** 例如 `10^4`, `10^7` 个元素。使用 `-l1,--level1 <num1>` 进行指定。
    -   **Level 2 (矩阵-向量):** 例如 `128x128`, `256x256`, `512x512`, `1024x1024` 矩阵。使用 `-l2,--level2 <num1,num2>` 进行指定。
    -   **Level 3 (矩阵-矩阵):** 例如 `(128, 128, 128)`, `(512, 512, 512)`, `(4096, 4096, 4096)`。使用 `-l3,--level3 <num1,num2,num3>` 进行指定。
    -   *规模可通过命令行参数灵活指定。*
-   **线程配置 (Thread Configuration):** `-t,--threads <num>` 指定使用的线程数 (`1` 表示单线程，默认为单线程)。该选项将强制覆盖 `OPENBLAS_NUM_THREADS`。
-   **测试循环次数 (Iterations):** `-c,--cycle <num>` 指定每个测试用例运行的次数（用于计算平均时间）。
-   **预热次数 (Warmup):** `-w,--warmup <num>` 指定预热次数。默认为3次。

### 2.3 输出要求

-   **标准输出 (stdout):** 默认在终端打印 **Markdown 格式**的性能结果，和输出的文件的内容完全一致。
-   **文件输出 (File Output):** 支持通过 `-o,--output <filename>` 参数将结果保存到指定文件。
-   **输出格式 (Output Format):** 支持通过 `-f,--format <markdown|csv>` 参数选择输出格式（Markdown 或 CSV）。


*当输出类型是MD的时候，将会一同输出检测到的系统配置环境，包括：*
- 系统的发行版
- 内核版本
- 编译器类型和版本
- 系统的CPU配置
- 系统的内存配置
- etc.

**性能结果表格包含以下列：**

| 列名            | 描述|
| :-------------- | :------------------------------------------------------------------- |
| **Function**    | 测试的 BLAS 函数名 (例如 `cblas_dgemm` )。|
| **Config**      | 测试配置参数 (例如 `M=1024,N=1024,K=1024` )。|
| **Threads**     | 执行测试时使用的线程数。|
| **Avg Time (ms)** | 函数执行的平均时间（毫秒），基于多次运行计算得出。|
| **GFLOPS**      | 根据函数操作的理论浮点运算次数 (FLOPs) 和平均时间计算得出的性能指标。|

最后会对于每个函数的测试得分为 $ score_f = GFLOPS \times weight $。
权重按照每个函数的重要性、业界使用频次来进行给分（全部先暂定为1.0）。

## 3. 依赖库

-   **[OpenBLAS](https://www.openblas.net/):** 提供高性能 BLAS 实现。
-   **[CLI11](https://github.com/CLIUtils/CLI11):** 用于解析命令行参数。*(作为头文件库包含在项目中)*
-   **[toml++](https://github.com/marzer/tomlplusplus):** 解析toml配置文件。

## 4. 构建与运行

本项目使用 **Xmake** 作为构建系统。

1.  **安装 Xmake:** 请参考 [Xmake 官方文档](https://xmake.io/#/guide/installation)。
2.  **构建项目:** 在项目根目录执行 `xmake`。
3.  **运行基准测试:**
```bash
# 基本运行 (使用默认参数)
xmake run blas_benchmark
# 示例
xmake run blas_benchmark -t 8 -c 10 -f csv -o gemm_results.csv
```

*使用 `xmake run blas_benchmark --help` 查看完整的命令行选项说明。*

## 5. 测试建议

- ​系统状态:​​ 在测试前确保系统负载较低且 CPU 频率稳定（可考虑使用 cpupower设置性能模式）。

- ​预热:​​ 框架内置预热是好的实践。对于更严格的测试，可考虑在整体测试开始前运行一次完整的测试集进行额外预热。

- 规模选择:​​ 选择的规模应能覆盖从 L1 缓存到主存的不同范围，以揭示内存带宽和计算强度的瓶颈。4096x4096x4096对于 Level 3 是测试峰值计算能力 (接近理论 FLOPS) 的典型规模。

- 线程数:​​ 测试单线程 (--threads 1) 和多线程 (例如 --threads $(nproc)) 以评估并行扩展性。


## 6. FLOPS计算方式
### Level 1
未定
### Level 2
未定
### Level 3
|函数名|计算公式|
|:-   |:-       |
|dgemm|$O(2MNK)$|

## 7. 配置文件示例
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
├── config.toml                # 默认的TOML配置文件
├── LICENSE
├── README.md
├── src
│   ├── benchmark
│   │   ├── benchmark.cpp      # 基准测试
│   │   ├── benchmark.h
│   │   ├── blas_functions.cpp # BLAS函数包装
│   │   └── blas_functions.h
│   ├── config
│   │   ├── config_parser.cpp  # TOML配置解析
│   │   └── config_parser.h
│   ├── main.cpp
│   └── utils
│       ├── system_info.cpp    # 系统信息收集
│       ├── system_info.h
│       ├── timer.cpp          # 高精度计时
│       └── timer.h
├── thirdparty
│   ├── CLI11
│   │   └── CLI11.hpp          # CLI11单头文件
│   └── toml++
│       └── toml.hpp           # toml++单头文件
└── xmake.lua                  # Xmake构建配置
```

## 9. 备注
该项目主要是用于个人学会OpenBLAS，C++ 23，Git，Xmake，toml++，CLI11 的使用。