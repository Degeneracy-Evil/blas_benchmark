#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace blas_benchmark::utils
{

// System information structure
struct SystemInfo
{
    std::string cpu_model;
    int cpu_cores{0};
    int physical_cores{0};
    int threads_per_core{0};
    double cpu_freq_mhz{0.0};
    std::size_t l1_cache{0};
    std::size_t l2_cache{0};
    std::size_t l3_cache{0};
    std::size_t total_memory{0};
    std::string os_name;
};

// Collect system information for benchmark context
class SystemInfoCollector
{
public:
    SystemInfoCollector() = default;

    // Collect all system information
    [[nodiscard]] SystemInfo collect() const;

    // Get CPU model name
    [[nodiscard]] std::string get_cpu_model() const;

    // Get number of logical CPU cores
    [[nodiscard]] int get_cpu_cores() const;

    // Get number of physical CPU cores
    [[nodiscard]] int get_physical_cores() const;

    // Get threads per core (hyperthreading)
    [[nodiscard]] int get_threads_per_core() const;

    // Get CPU frequency in MHz
    [[nodiscard]] double get_cpu_freq_mhz() const;

    // Get L1 data cache size in bytes
    [[nodiscard]] std::size_t get_l1_cache() const;

    // Get L2 cache size in bytes
    [[nodiscard]] std::size_t get_l2_cache() const;

    // Get L3 cache size in bytes
    [[nodiscard]] std::size_t get_l3_cache() const;

    // Get total system memory in bytes
    [[nodiscard]] std::size_t get_total_memory() const;

    // Get operating system name
    [[nodiscard]] std::string get_os_name() const;

    // Get total cache size (L1 + L2 + L3) for cache flush
    [[nodiscard]] std::size_t get_total_cache() const
    {
        return get_l1_cache() + get_l2_cache() + get_l3_cache();
    }
};

} // namespace blas_benchmark::utils
