#include "benchmark/benchmark.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <numeric>

#include <spdlog/spdlog.h>

#include "benchmark/blas_functions.h"
#include "utils/timer.h"

namespace blas_benchmark
{

namespace
{

// Set OpenBLAS thread count
extern "C" void openblas_set_num_threads(int num_threads);
extern "C" int openblas_get_num_threads();

} // anonymous namespace

BenchmarkRunner::BenchmarkRunner(const config::BenchmarkConfig& config)
    : m_config(config)
{
    // Calculate cache size for flushing
    auto sys_info = m_info_collector.collect();
    m_cache_size = sys_info.l1_cache + sys_info.l2_cache + sys_info.l3_cache;
    
    // Ensure minimum cache size for flushing
    if (m_cache_size < 1024 * 1024)
    {
        m_cache_size = 16 * 1024 * 1024; // 16MB default
    }
    
    spdlog::info("Cache size for flushing: {} MB", m_cache_size / (1024 * 1024));
}

void BenchmarkRunner::set_threads(int num_threads)
{
    openblas_set_num_threads(num_threads);
    spdlog::info("Set OpenBLAS threads to {}", num_threads);
}

BenchmarkReport BenchmarkRunner::run_all()
{
    BenchmarkReport report;
    report.system_info = m_info_collector.collect();
    report.config = m_config;

    spdlog::info("Starting benchmark on {}", report.system_info.cpu_model);
    spdlog::info("CPU cores: {} physical, {} logical", 
                 report.system_info.physical_cores, report.system_info.cpu_cores);

    // Set thread count
    set_threads(m_config.threads);

    // Run benchmarks for each level
    if (m_config.level1_size.has_value() && !m_config.level1_functions.empty())
    {
        spdlog::info("Running Level 1 benchmarks...");
        run_level1(report);
    }

    if (m_config.level2_size.has_value() && !m_config.level2_functions.empty())
    {
        spdlog::info("Running Level 2 benchmarks...");
        run_level2(report);
    }

    if (m_config.level3_size.has_value() && !m_config.level3_functions.empty())
    {
        spdlog::info("Running Level 3 benchmarks...");
        run_level3(report);
    }

    return report;
}

template<typename Func>
BenchmarkResult BenchmarkRunner::run_single_benchmark(
    const std::string& name,
    const std::string& config_str,
    Func&& benchmark_func,
    std::size_t flops_count)
{
    BenchmarkResult result;
    result.function_name = name;
    result.config_str = config_str;
    result.threads = m_config.threads;
    result.flops = flops_count;

    spdlog::info("Running {} benchmark...", name);

    // Collect timing data
    std::vector<double> times;
    times.reserve(m_config.cycles);

    for (int i = 0; i < m_config.cycles; ++i)
    {
        double time_ms = benchmark_func();
        times.push_back(time_ms);
        spdlog::debug("  Iteration {}: {:.3f} ms", i + 1, time_ms);
    }

    // Calculate statistics
    result.min_time_ms = *std::min_element(times.begin(), times.end());
    result.max_time_ms = *std::max_element(times.begin(), times.end());
    result.avg_time_ms = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

    // Calculate GFLOPS
    // GFLOPS = FLOPs / (time_seconds * 1e9)
    double time_sec = result.avg_time_ms / 1000.0;
    result.gflops = static_cast<double>(flops_count) / (time_sec * 1e9);

    spdlog::info("  {} - Avg: {:.3f} ms, Min: {:.3f} ms, Max: {:.3f} ms, GFLOPS: {:.2f}",
                 name, result.avg_time_ms, result.min_time_ms, result.max_time_ms, result.gflops);

    return result;
}

void BenchmarkRunner::run_level1(BenchmarkReport& report)
{
    auto n = m_config.level1_size.value();
    auto config_str = std::format("N={}", n);

    for (const auto& func_name : m_config.level1_functions)
    {
        BenchmarkResult result;

        if (func_name == "cblas_ddot")
        {
            result = run_single_benchmark(
                "ddot", config_str,
                [this, n]() {
                    return benchmark_dot<double>(n, m_config.warmup, 1, 
                                                  m_config.flush_cache, m_cache_size);
                },
                flops::dot(n));
        }
        else if (func_name == "cblas_daxpy")
        {
            result = run_single_benchmark(
                "daxpy", config_str,
                [this, n]() {
                    return benchmark_axpy<double>(n, m_config.warmup, 1,
                                                   m_config.flush_cache, m_cache_size);
                },
                flops::axpy(n));
        }
        else if (func_name == "cblas_dscal")
        {
            result = run_single_benchmark(
                "dscal", config_str,
                [this, n]() {
                    return benchmark_scal<double>(n, m_config.warmup, 1,
                                                   m_config.flush_cache, m_cache_size);
                },
                flops::scal(n));
        }
        else
        {
            spdlog::warn("Unknown Level 1 function: {}", func_name);
            continue;
        }

        report.level1_results.push_back(result);
    }
}

void BenchmarkRunner::run_level2(BenchmarkReport& report)
{
    auto [m, n] = m_config.level2_size.value();
    auto config_str = std::format("M={},N={}", m, n);

    for (const auto& func_name : m_config.level2_functions)
    {
        BenchmarkResult result;

        if (func_name == "cblas_dgemv")
        {
            result = run_single_benchmark(
                "dgemv", config_str,
                [this, m, n]() {
                    return benchmark_gemv<double>(m, n, m_config.warmup, 1,
                                                   m_config.flush_cache, m_cache_size);
                },
                flops::gemv(m, n));
        }
        else
        {
            spdlog::warn("Unknown Level 2 function: {}", func_name);
            continue;
        }

        report.level2_results.push_back(result);
    }
}

void BenchmarkRunner::run_level3(BenchmarkReport& report)
{
    auto [m, n, k] = m_config.level3_size.value();
    auto config_str = std::format("M={},N={},K={}", m, n, k);

    for (const auto& func_name : m_config.level3_functions)
    {
        BenchmarkResult result;

        if (func_name == "cblas_dgemm")
        {
            result = run_single_benchmark(
                "dgemm", config_str,
                [this, m, n, k]() {
                    return benchmark_gemm<double>(m, n, k, m_config.warmup, 1,
                                                   m_config.flush_cache, m_cache_size);
                },
                flops::gemm(m, n, k));
        }
        else
        {
            spdlog::warn("Unknown Level 3 function: {}", func_name);
            continue;
        }

        report.level3_results.push_back(result);
    }
}

std::string OutputFormatter::to_markdown(const BenchmarkReport& report)
{
    std::string output;

    // System information header
    output += "# BLAS Benchmark Results\n\n";
    output += std::format("## System Information\n");
    output += std::format("- **CPU**: {}\n", report.system_info.cpu_model);
    output += std::format("- **Cores**: {} physical, {} logical\n", 
                          report.system_info.physical_cores, report.system_info.cpu_cores);
    output += std::format("- **Cache**: L1={} KB, L2={} KB, L3={} MB\n",
                          report.system_info.l1_cache / 1024,
                          report.system_info.l2_cache / 1024,
                          report.system_info.l3_cache / (1024 * 1024));
    output += std::format("- **Memory**: {:.1f} GB\n",
                          static_cast<double>(report.system_info.total_memory) / (1024 * 1024 * 1024));
    output += std::format("- **Threads**: {}\n\n", report.config.threads);

    // Helper lambda to format a table
    auto format_table = [&output](const std::string& title, const std::vector<BenchmarkResult>& results)
    {
        if (results.empty())
        {
            return;
        }

        output += std::format("### {}\n\n", title);
        output += "| Function | Config | Threads | Min(ms) | Avg(ms) | Max(ms) | GFLOPS |\n";
        output += "|:---------|:-------|:--------|:--------|:--------|:--------|:-------|\n";

        for (const auto& r : results)
        {
            output += std::format("| {} | {} | {} | {:.3f} | {:.3f} | {:.3f} | {:.2f} |\n",
                                  r.function_name, r.config_str, r.threads,
                                  r.min_time_ms, r.avg_time_ms, r.max_time_ms, r.gflops);
        }
        output += "\n";
    };

    format_table("Level 1 (Vector-Vector)", report.level1_results);
    format_table("Level 2 (Matrix-Vector)", report.level2_results);
    format_table("Level 3 (Matrix-Matrix)", report.level3_results);

    return output;
}

std::string OutputFormatter::to_csv(const BenchmarkReport& report)
{
    std::string output;

    // CSV header
    output += "Level,Function,Config,Threads,Min(ms),Avg(ms),Max(ms),GFLOPS\n";

    // Level 1 results
    for (const auto& r : report.level1_results)
    {
        output += std::format("1,{},{},{},{:.3f},{:.3f},{:.3f},{:.2f}\n",
                              r.function_name, r.config_str, r.threads,
                              r.min_time_ms, r.avg_time_ms, r.max_time_ms, r.gflops);
    }

    // Level 2 results
    for (const auto& r : report.level2_results)
    {
        output += std::format("2,{},{},{},{:.3f},{:.3f},{:.3f},{:.2f}\n",
                              r.function_name, r.config_str, r.threads,
                              r.min_time_ms, r.avg_time_ms, r.max_time_ms, r.gflops);
    }

    // Level 3 results
    for (const auto& r : report.level3_results)
    {
        output += std::format("3,{},{},{},{:.3f},{:.3f},{:.3f},{:.2f}\n",
                              r.function_name, r.config_str, r.threads,
                              r.min_time_ms, r.avg_time_ms, r.max_time_ms, r.gflops);
    }

    return output;
}

std::string OutputFormatter::format(const BenchmarkReport& report, const std::string& format)
{
    if (format == "csv")
    {
        return to_csv(report);
    }
    return to_markdown(report);
}

} // namespace blas_benchmark
