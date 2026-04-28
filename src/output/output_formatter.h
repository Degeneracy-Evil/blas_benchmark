#pragma once

#include <string>

#include "benchmark/benchmark.h"

namespace blas_benchmark
{

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
