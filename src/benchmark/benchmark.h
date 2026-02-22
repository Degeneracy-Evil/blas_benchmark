#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "config/config_parser.h"
#include "utils/system_info.h"

namespace blas_benchmark
{

// Single benchmark result
struct BenchmarkResult
{
    std::string function_name;
    std::string config_str;
    int threads{0};
    double min_time_ms{0.0};
    double avg_time_ms{0.0};
    double max_time_ms{0.0};
    double gflops{0.0};
    std::size_t flops{0};
};

// Complete benchmark report
struct BenchmarkReport
{
    utils::SystemInfo system_info;
    std::vector<BenchmarkResult> level1_results;
    std::vector<BenchmarkResult> level2_results;
    std::vector<BenchmarkResult> level3_results;
    config::BenchmarkConfig config;
};

// Main benchmark runner class
class BenchmarkRunner
{
public:
    explicit BenchmarkRunner(const config::BenchmarkConfig& config);

    // Run all benchmarks and return results
    [[nodiscard]] BenchmarkReport run_all();

    // Run Level 1 benchmarks
    void run_level1(BenchmarkReport& report);

    // Run Level 2 benchmarks
    void run_level2(BenchmarkReport& report);

    // Run Level 3 benchmarks
    void run_level3(BenchmarkReport& report);

    // Set number of OpenBLAS threads
    void set_threads(int num_threads);

    // Get cache size for flushing
    [[nodiscard]] std::size_t get_cache_size() const
    {
        return m_cache_size;
    }

private:
    config::BenchmarkConfig m_config;
    utils::SystemInfoCollector m_info_collector;
    std::size_t m_cache_size{16 * 1024 * 1024}; // Default 16MB

    // Run a single benchmark function and collect timing statistics
    template<typename Func>
    BenchmarkResult run_single_benchmark(
        const std::string& name,
        const std::string& config_str,
        Func&& benchmark_func,
        std::size_t flops_count);
};

// Output formatter for different formats
class OutputFormatter
{
public:
    // Format report as Markdown table
    [[nodiscard]] static std::string to_markdown(const BenchmarkReport& report);

    // Format report as CSV
    [[nodiscard]] static std::string to_csv(const BenchmarkReport& report);

    // Format based on config
    [[nodiscard]] static std::string format(const BenchmarkReport& report, const std::string& format);
};

} // namespace blas_benchmark
