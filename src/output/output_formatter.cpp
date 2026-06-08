#include "output/output_formatter.hpp"

#include <format>

namespace blas_benchmark
{

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
                          report.system_info.l1_cache / 1024, report.system_info.l2_cache / 1024,
                          report.system_info.l3_cache / (1024 * 1024));
    output +=
        std::format("- **Memory**: {:.1f} GB\n",
                    static_cast<double>(report.system_info.total_memory) / (1024 * 1024 * 1024));
    output += std::format("- **Threads**: {}\n\n", report.config.threads);

    // Helper lambda to format a table
    auto format_table =
        [&output](const std::string& title, const std::vector<BenchmarkResult>& results)
    {
        if(results.empty())
        {
            return;
        }

        output += std::format("### {}\n\n", title);
        output += "| Function | Config | Threads | Min(ms) | Avg(ms) | Max(ms) | GFLOPS |\n";
        output += "|:---------|:-------|:--------|:--------|:--------|:--------|:-------|\n";

        for(const auto& r : results)
        {
            output += std::format("| {} | {} | {} | {:.3f} | {:.3f} | {:.3f} | {:.2f} |\n",
                                  r.function_name, r.config_str, r.threads, r.min_time_ms,
                                  r.avg_time_ms, r.max_time_ms, r.gflops);
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
    for(const auto& r : report.level1_results)
    {
        output +=
            std::format("1,{},{},{},{:.3f},{:.3f},{:.3f},{:.2f}\n", r.function_name, r.config_str,
                        r.threads, r.min_time_ms, r.avg_time_ms, r.max_time_ms, r.gflops);
    }

    // Level 2 results
    for(const auto& r : report.level2_results)
    {
        output +=
            std::format("2,{},{},{},{:.3f},{:.3f},{:.3f},{:.2f}\n", r.function_name, r.config_str,
                        r.threads, r.min_time_ms, r.avg_time_ms, r.max_time_ms, r.gflops);
    }

    // Level 3 results
    for(const auto& r : report.level3_results)
    {
        output +=
            std::format("3,{},{},{},{:.3f},{:.3f},{:.3f},{:.2f}\n", r.function_name, r.config_str,
                        r.threads, r.min_time_ms, r.avg_time_ms, r.max_time_ms, r.gflops);
    }

    return output;
}

std::string OutputFormatter::format(const BenchmarkReport& report, const std::string& format)
{
    if(format == "csv")
    {
        return to_csv(report);
    }
    return to_markdown(report);
}

} // namespace blas_benchmark
