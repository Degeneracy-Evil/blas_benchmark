#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace blas_benchmark::config
{

// Benchmark configuration structure
struct BenchmarkConfig
{
    // Execution parameters
    int threads{1};
    int cycles{5};
    int warmup{3};
    bool flush_cache{true};

    // Test sizes for each BLAS level
    std::optional<std::size_t> level1_size;
    std::optional<std::pair<int, int>> level2_size;      // (M, N)
    std::optional<std::tuple<int, int, int>> level3_size; // (M, N, K)

    // Output configuration
    std::string output_file;
    std::string format{"markdown"}; // "markdown" or "csv"

    // Function selection
    std::vector<std::string> level1_functions;
    std::vector<std::string> level2_functions;
    std::vector<std::string> level3_functions;

    // Function weights for scoring
    std::vector<std::pair<std::string, double>> level1_weights;
    std::vector<std::pair<std::string, double>> level2_weights;
    std::vector<std::pair<std::string, double>> level3_weights;
};

// Configuration file parser using TOML
class ConfigParser
{
public:
    ConfigParser() = default;

    // Parse configuration from file
    [[nodiscard]] BenchmarkConfig parse_file(const std::string& path) const;

    // Parse configuration from string
    [[nodiscard]] BenchmarkConfig parse_string(const std::string& content) const;

    // Get default configuration
    [[nodiscard]] static BenchmarkConfig get_default()
    {
        BenchmarkConfig config;
        config.level1_size = 1000000;
        config.level2_size = {1024, 1024};
        config.level3_size = {1024, 1024, 1024};
        config.level1_functions = {"cblas_ddot", "cblas_daxpy", "cblas_dscal"};
        config.level2_functions = {"cblas_dgemv"};
        config.level3_functions = {"cblas_dgemm"};
        return config;
    }
};

} // namespace blas_benchmark::config
