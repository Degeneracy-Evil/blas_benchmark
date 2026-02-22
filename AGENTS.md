# AGENTS.md - BLAS Benchmark 项目指南

基于 C++23 的 OpenBLAS 性能测试框架，评估 BLAS 函数在 Level 1/2/3 的性能表现。目前支持 CPU CBLAS 框架，未来将支持 Fortran 和 XPU 异构 BLAS 测试。

## 1. 构建与运行

### 环境要求
- 构建系统: Xmake (推荐 Ninja)
- 编译器: Clang
- C++ 标准: C++23

### 构建命令

```bash
xmake f -m release          # Release 模式
xmake f -m debug            # Debug 模式
xmake                       # 构建
xmake -j8                   # 并行构建
xmake clean                 # 清理
xmake f -c && xmake         # 完全重新构建
```

### 运行命令

```bash
xmake run cblas_benchmark                          # 基本运行
xmake run cblas_benchmark --help                   # 查看帮助
xmake run cblas_benchmark -t 8 -c 10               # 8线程, 10次迭代
xmake run cblas_benchmark -f csv -o out.csv        # CSV 格式输出
```

### 测试单个 BLAS 函数

```bash
# Level 1 (向量操作)
xmake run cblas_benchmark -l1 1000000 -c 5

# Level 2 (矩阵-向量)
xmake run cblas_benchmark -l2 1024,1024 -c 5

# Level 3 (矩阵-矩阵)
xmake run cblas_benchmark -l3 1024,1024,1024 -c 5 -t 4
```

## 2. 代码风格

### 命名约定

| 类型 | 风格 | 示例 |
|------|------|------|
| 命名空间 | snake_case | blas_benchmark |
| 类/结构体 | PascalCase | BenchmarkRunner |
| 函数/方法 | snake_case | run_benchmark() |
| 变量 | snake_case | num_threads |
| 常量 | UPPER_SNAKE_CASE | MAX_ITERATIONS |
| 成员变量 | snake_case + m_ 前缀 | m_config |
| 模板参数 | PascalCase | typename T |

### 文件组织

```
src/
├── main.cpp
├── benchmark/benchmark.h/cpp, blas_functions.h/cpp
├── config/config_parser.h/cpp
└── utils/timer.h/cpp, system_info.h/cpp
```

### 头文件规范

```cpp
#pragma once

#include <chrono>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "benchmark/benchmark.h"
```

### 代码格式

- 缩进: 4 空格 (不使用 Tab)
- 大括号: Allman 风格 (换行)
- 行宽: 100-120 字符
- 注释: 所有的大体和关键步骤都必须添加英文注释
- 输出: 使用cpp23引入的std::println
- 宏: 尽量不使用宏定义，最好使用inline和constexpr，若有特殊的地方也可以使用宏定义
- 头文件: 使用`#pragma once`，不使用宏守卫
### 包含文件顺序

1. 对应的头文件
2. 系统头文件 (按字母顺序)
3. 第三方库头文件
4. 项目内部头文件

## 3. 类型与错误处理

```cpp
std::size_t         // 大小、索引
std::int64_t        // 64位整数
double              // BLAS 默认浮点类型
auto result = benchmark.run();
constexpr int DEFAULT_WARMUP = 3;

// 错误处理
throw std::runtime_error("Config file not found: " + path);
spdlog::info("Starting with {} threads", num_threads);
spdlog::error("Failed: {}", path);
```

- 使用 RAII 管理资源
- 优先使用智能指针
- 避免裸指针和手动 new/delete

## 4. 命令行参数

| 参数 | 简写 | 说明 | 默认值 |
|------|------|------|--------|
| --threads | -t | 线程数 | 1 |
| --cycle | -c | 迭代次数 | - |
| --warmup | -w | 预热次数 | 3 |
| --level1 | -l1 | Level1 向量大小 | - |
| --level2 | -l2 | Level2 矩阵尺寸 | - |
| --level3 | -l3 | Level3 矩阵尺寸 | - |
| --output | -o | 输出文件 | stdout |
| --format | -f | 输出格式 | markdown |

## 5. 配置文件 (config.toml)

```toml
[functions]
level1 = ["cblas_ddot", "cblas_daxpy"]
level2 = ["cblas_dgemv"]
level3 = ["cblas_dgemm"]

[weights.level1]
cblas_ddot = 1.0

[weights.level3]
cblas_dgemm = 2.0
```

## 6. 依赖库

| 依赖 | 位置 |
|------|------|
| OpenBLAS | 系统安装 |
| CLI11 | thirdparty/CLI11/CLI11.hpp |
| toml++ | thirdparty/toml++/toml.hpp |
| spdlog | 系统安装 |

## 7. 注意事项

1. 编译前安装 OpenBLAS: `sudo apt install libopenblas-dev`
2. 使用 Clang 工具链 (已在 xmake.lua 中指定)
3. 不要提交 build/ 和 .xmake/ 目录
4. 第三方库为 header-only，无需额外编译
5. 修改构建配置后需运行 `xmake f -c`
6. 生成 LSP 配置: `xmake compdb`

## 8. 开发后检查规范

**重要:** 每次项目开发完成后，必须检查并更新以下文档:

| 文档 | 说明 |
|------|------|
| README.md | 英文文档，同步功能变更、依赖更新 |
| README.zh-CN.md | 中文文档，与英文版保持一致 |
| PROGRESS.md | 进度文档，更新实现状态、变更日志 |
| AGENTS.md | 项目指南，更新构建命令、注意事项 |

**需要更新的场景:**
- 新增/修改/删除功能
- 依赖库变更
- 构建流程变化
- CLI 参数变化
- 配置文件格式变化
- CI/CD 流程变化

**原则:** 所有文档必须保持同步，确保 AI 代理和开发者能准确理解项目状态。
