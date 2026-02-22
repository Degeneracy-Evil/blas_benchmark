---
name: blas-benchmark
description: Skills for BLAS Benchmark project - C++23 OpenBLAS performance testing framework covering build, code style, BLAS functions, and testing patterns
license: GPL3
compatibility: opencode
metadata:
  language: cpp23
  build_system: xmake
  domain: hpc
---

# SKILL.md - BLAS Benchmark 项目技能

本文档定义 AI 编程代理在项目中应具备的技能和知识。

---

## 1. 中英文 README 同步

### 1.1 同步规则

- **基准文件**: `README.zh-CN.md` (中文版)
- **翻译文件**: `README.md` (英文版)
- **同步原则**: 中文版修改后，英文版必须同步更新

### 1.2 翻译规范

| 中文 | 英文 |
|------|------|
| 项目概述 | Project Overview |
| 功能需求 | Functional Requirements |
| 核心测试功能 | Core Testing Features |
| 测试配置 | Test Configuration |
| 输出要求 | Output Requirements |
| 依赖库 | Dependencies |
| 构建与运行 | Build & Run |
| 测试建议 | Testing Recommendations |
| FLOPS计算方式 | FLOPS Calculation |
| 配置文件示例 | Configuration Example |
| 项目结构 | Project Structure |
| 备注 | Notes |

### 1.3 格式一致性

- 保持相同的 Markdown 结构
- 代码块语言标识一致
- 表格格式一致
- 数学公式使用 `$...$` 行内或 `$$...$$` 块级

---

## 2. 构建技能

### 2.1 Xmake 构建系统

```bash
# 常用命令
xmake f -m release          # 配置 Release 模式
xmake f -m debug            # 配置 Debug 模式
xmake                       # 构建
xmake -j$(nproc)            # 并行构建
xmake clean                 # 清理
xmake f -c && xmake         # 完全重新构建
xmake run cblas_benchmark   # 运行
```

### 2.2 项目结构

```
blas_benchmark/
├── src/
│   ├── main.cpp                    # 程序入口
│   ├── benchmark/                  # 性能测试模块
│   │   ├── benchmark.h/cpp         # 基准测试框架
│   │   └── blas_functions.h/cpp    # BLAS 函数封装
│   ├── config/                     # 配置解析模块
│   │   └── config_parser.h/cpp     # TOML 配置解析
│   └── utils/                      # 工具模块
│       ├── timer.h/cpp             # 高精度计时器
│       └── system_info.h/cpp       # 系统信息收集
├── thirdparty/                     # 第三方库
│   ├── CLI11/CLI11.hpp
│   └── toml++/toml.hpp
├── config.toml                     # 配置文件
├── xmake.lua                       # 构建配置
├── README.md                       # 英文文档
├── README.zh-CN.md                 # 中文文档
└── AGENTS.md                       # AI 代理指南
```

---

## 3. BLAS 函数知识

### 3.1 BLAS 层级

| Level | 操作类型 | 典型函数 | 复杂度 |
|-------|----------|----------|--------|
| Level 1 | 向量-向量 | ddot, daxpy, dscal | O(n) |
| Level 2 | 矩阵-向量 | dgemv, dger, dsymv | O(n²) |
| Level 3 | 矩阵-矩阵 | dgemm, dsymm, dtrmm | O(n³) |

### 3.2 FLOPS 计算

```cpp
// Level 1: ddot - 点积
// FLOPs = 2n (n 次乘法 + n-1 次加法 ≈ 2n)

// Level 2: dgemv - 矩阵向量乘法
// FLOPs = 2mn (m*n 次乘法 + m*(n-1) 次加法)

// Level 3: dgemm - 矩阵矩阵乘法 C = αAB + βC
// FLOPs = 2mnk (m*n*k 次乘法 + m*n*(k-1) 次加法)
```

### 3.3 GFLOPS 计算公式

```
GFLOPS = FLOPs / (执行时间(秒) * 10^9)
```

---

## 4. 代码风格技能

### 4.1 C++23 规范

```cpp
// 使用 sized 类型
std::size_t idx;
std::int64_t count;

// 使用 constexpr
constexpr int DEFAULT_WARMUP = 3;

// 使用 auto 简化
auto result = benchmark.run();

// 使用结构化绑定 (C++17+)
auto [name, value] = parse_config();

// 使用 ranges (C++20+)
auto filtered = data | std::views::filter(predicate);
```

### 4.2 错误处理模式

```cpp
// 使用异常
if (!file_exists(path)) {
    throw std::runtime_error("File not found: " + path);
}

// 使用 spdlog 日志
spdlog::info("Starting with {} threads", num_threads);
spdlog::warn("Using default configuration");
spdlog::error("Failed to parse: {}", error_msg);
spdlog::debug("Iteration {} completed", i);
```

### 4.3 RAII 资源管理

```cpp
// 使用智能指针
auto data = std::make_unique<double[]>(size);
auto config = std::make_shared<Config>();

// 使用 RAII 包装
class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}
    ~Timer() { /* 自动记录 */ }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};
```

---

## 5. 命令行参数处理

### 5.1 参数定义

| 参数 | 类型 | 说明 |
|------|------|------|
| -t, --threads | int | 线程数 |
| -c, --cycle | int | 测试迭代次数 |
| -w, --warmup | int | 预热次数 |
| -1, --level1 | int | Level1 向量大小 |
| -2, --level2 | string | Level2 矩阵尺寸 (M,N) |
| -3, --level3 | string | Level3 矩阵尺寸 (M,N,K) |
| -o, --output | string | 输出文件路径 |
| -f, --format | string | 输出格式 |
| -C, --config | string | 配置文件路径 |

### 5.2 解析示例

```cpp
CLI::App app{"BLAS Benchmark"};
app.add_option("-t,--threads", threads, "Number of threads")->default_val(1);
app.add_option("-c,--cycle", cycles, "Number of iterations");
app.add_option("-3,--level3", level3_str, "Level3 size (M,N,K)");
CLI11_PARSE(app, argc, argv);
```

---

## 6. 配置文件处理

### 6.1 TOML 配置结构

```toml
[functions]
level1 = ["cblas_ddot", "cblas_daxpy"]
level2 = ["cblas_dgemv"]
level3 = ["cblas_dgemm"]

[weights.level1]
cblas_ddot = 1.0
cblas_daxpy = 1.0

[weights.level2]
cblas_dgemv = 1.5

[weights.level3]
cblas_dgemm = 2.0
```

### 6.2 解析示例

```cpp
auto config = toml::parse_file("config.toml");
auto level1_funcs = config["functions"]["level1"].as_array();
auto weight = config["weights"]["level1"]["cblas_ddot"].value<double>();
```

---

## 7. 输出格式

### 7.1 Markdown 表格

```markdown
| Function | Config | Threads | Avg Time (ms) | GFLOPS |
|:---------|:-------|:--------|:--------------|:-------|
| dgemm | M=1024,N=1024,K=1024 | 8 | 15.23 | 141.5 |
```

### 7.2 CSV 格式

```csv
Function,Config,Threads,Avg Time (ms),GFLOPS
dgemm,M=1024,N=1024,K=1024,8,15.23,141.5
```

---

## 8. 测试技能

### 8.1 性能测试建议

- 系统状态: 低负载、CPU 频率稳定
- 预热: 至少 3 次预热运行
- 规模选择: 覆盖 L1/L2/L3 缓存和主存
- 线程: 测试单线程和多线程扩展性

### 8.2 典型测试场景

```bash
# 单线程峰值测试
xmake run cblas_benchmark -3 4096,4096,4096 -c 10 -t 1

# 多线程扩展性测试
xmake run cblas_benchmark -3 2048,2048,2048 -c 5 -t $(nproc)

# 小规模快速验证
xmake run cblas_benchmark -1 10000 -2 128,128 -3 128,128,128 -c 3
```
