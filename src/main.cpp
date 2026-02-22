#include <CLI/CLI.hpp>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <print>
#include <string>
#include <tuple>
#include <utility>

#include <spdlog/spdlog.h>

#include "benchmark/benchmark.h"
#include "config/config_parser.h"
#include "utils/system_info.h"

namespace
{

// Parse size string like "1024,1024" into pair
std::optional<std::pair<int, int>> parse_size_pair(const std::string &str)
{
    auto pos = str.find(',');
    if (pos == std::string::npos)
    {
        return std::nullopt;
    }
    try
    {
        int m = std::stoi(str.substr(0, pos));
        int n = std::stoi(str.substr(pos + 1));
        return std::make_pair(m, n);
    }
    catch (...)
    {
        return std::nullopt;
    }
}

// Parse size string like "1024,1024,1024" into triple
std::optional<std::tuple<int, int, int>>
parse_size_triple(const std::string &str)
{
    auto pos1 = str.find(',');
    if (pos1 == std::string::npos)
    {
        return std::nullopt;
    }
    auto pos2 = str.find(',', pos1 + 1);
    if (pos2 == std::string::npos)
    {
        return std::nullopt;
    }
    try
    {
        int m = std::stoi(str.substr(0, pos1));
        int n = std::stoi(str.substr(pos1 + 1, pos2 - pos1 - 1));
        int k = std::stoi(str.substr(pos2 + 1));
        return std::make_tuple(m, n, k);
    }
    catch (...)
    {
        return std::nullopt;
    }
}

// Print system information
void print_system_info(const blas_benchmark::utils::SystemInfo &info)
{
    std::println("");
    std::println("=== System Information ===");
    std::println("CPU:          {}", info.cpu_model);
    std::println("Cores:        {} physical, {} logical", info.physical_cores,
                 info.cpu_cores);
    std::println("Frequency:    {:.0f} MHz", info.cpu_freq_mhz);
    std::println("L1 Cache:     {} KB", info.l1_cache / 1024);
    std::println("L2 Cache:     {} KB", info.l2_cache / 1024);
    std::println("L3 Cache:     {} MB", info.l3_cache / (1024 * 1024));
    std::println("Memory:       {:.1f} GB",
                 static_cast<double>(info.total_memory) /
                     (1024.0 * 1024.0 * 1024.0));
    std::println("OS:           {}", info.os_name);
    std::println("");
}

// Write output to file or stdout
void write_output(const std::string &content, const std::string &output_file)
{
    if (output_file.empty())
    {
        std::print("{}", content);
    }
    else
    {
        std::ofstream file(output_file);
        if (!file.is_open())
        {
            throw std::runtime_error("Cannot open output file: " + output_file);
        }
        file << content;
        spdlog::info("Output written to {}", output_file);
    }
}

} // anonymous namespace

int main(int argc, char *argv[])
{
    CLI::App app{"BLAS Benchmark - Performance testing for BLAS operations"};

    // Command line options
    int threads = 1;
    int cycles = 5;
    int warmup = 3;
    std::string level1_str;
    std::string level2_str;
    std::string level3_str;
    std::string output_file;
    std::string format = "markdown";
    std::string config_file = "config.toml";
    bool verbose = false;
    bool show_system_info = false;

    // Add options
    app.add_option("-t,--threads", threads, "Number of threads")
        ->default_val(1);
    app.add_option("-c,--cycle", cycles, "Number of benchmark cycles")
        ->default_val(5);
    app.add_option("-w,--warmup", warmup, "Number of warmup iterations")
        ->default_val(3);
    app.add_option("-1,--level1", level1_str, "Level 1 vector size (N)");
    app.add_option("-2,--level2", level2_str, "Level 2 matrix size (M,N)");
    app.add_option("-3,--level3", level3_str, "Level 3 matrix size (M,N,K)");
    app.add_option("-o,--output", output_file, "Output file path")
        ->default_val("");
    app.add_option("-f,--format", format, "Output format (markdown|csv)")
        ->default_val("markdown");
    app.add_option("-C,--config", config_file, "Configuration file path")
        ->default_val("config.toml");
    app.add_flag("-v,--verbose", verbose, "Enable verbose output");
    app.add_flag("-s,--system-info", show_system_info,
                 "Show system information only");

    // Parse arguments
    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e)
    {
        return app.exit(e);
    }

    // Setup logging
    if (verbose)
    {
        spdlog::set_level(spdlog::level::debug);
    }
    else
    {
        spdlog::set_level(spdlog::level::info);
    }

    // Show system info only
    if (show_system_info)
    {
        blas_benchmark::utils::SystemInfoCollector collector;
        auto info = collector.collect();
        print_system_info(info);
        return 0;
    }

    // Load configuration
    blas_benchmark::config::BenchmarkConfig config;

    if (std::filesystem::exists(config_file))
    {
        try
        {
            blas_benchmark::config::ConfigParser parser;
            config = parser.parse_file(config_file);
            spdlog::info("Loaded configuration from {}", config_file);
        }
        catch (const std::exception &e)
        {
            spdlog::warn("Failed to load config file: {}. Using defaults.",
                         e.what());
            config = blas_benchmark::config::ConfigParser::get_default();
        }
    }
    else
    {
        spdlog::info("Config file not found. Using defaults.");
        config = blas_benchmark::config::ConfigParser::get_default();
    }

    // Override config with command line arguments
    config.threads = threads;
    config.cycles = cycles;
    config.warmup = warmup;
    config.output_file = output_file;
    config.format = format;

    // Parse size arguments
    if (!level1_str.empty())
    {
        try
        {
            config.level1_size = std::stoull(level1_str);
        }
        catch (...)
        {
            spdlog::error("Invalid level1 size: {}", level1_str);
            return 1;
        }
    }

    if (!level2_str.empty())
    {
        auto size = parse_size_pair(level2_str);
        if (size.has_value())
        {
            config.level2_size = size.value();
        }
        else
        {
            spdlog::error("Invalid level2 size format: {}. Expected M,N",
                          level2_str);
            return 1;
        }
    }

    if (!level3_str.empty())
    {
        auto size = parse_size_triple(level3_str);
        if (size.has_value())
        {
            config.level3_size = size.value();
        }
        else
        {
            spdlog::error("Invalid level3 size format: {}. Expected M,N,K",
                          level3_str);
            return 1;
        }
    }

    // Validate at least one benchmark is configured
    if (!config.level1_size.has_value() && !config.level2_size.has_value() &&
        !config.level3_size.has_value())
    {
        spdlog::error("No benchmark sizes specified. Use --level1, --level2, "
                      "or --level3 options.");
        std::println("{}", app.help());
        return 1;
    }

    // Print benchmark configuration
    std::println("=== BLAS Benchmark ===");
    std::println("Threads:      {}", config.threads);
    std::println("Warmup:       {} iterations", config.warmup);
    std::println("Cycles:       {} iterations", config.cycles);
    std::println("Flush Cache:  {}", config.flush_cache ? "Yes" : "No");

    if (config.level1_size.has_value())
    {
        std::println("Level 1:      N={}", config.level1_size.value());
    }
    if (config.level2_size.has_value())
    {
        auto [m, n] = config.level2_size.value();
        std::println("Level 2:      M={}, N={}", m, n);
    }
    if (config.level3_size.has_value())
    {
        auto [m, n, k] = config.level3_size.value();
        std::println("Level 3:      M={}, N={}, K={}", m, n, k);
    }

    // Run benchmarks
    try
    {
        blas_benchmark::BenchmarkRunner runner(config);
        auto report = runner.run_all();

        // Print system info
        print_system_info(report.system_info);

        // Format and output results
        std::string output =
            blas_benchmark::OutputFormatter::format(report, config.format);
        write_output(output, config.output_file);
    }
    catch (const std::exception &e)
    {
        spdlog::error("Benchmark failed: {}", e.what());
        return 1;
    }

    return 0;
}
