#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace blas_benchmark::utils
{

// High precision timer using std::chrono
class Timer
{
public:
    Timer() = default;

    // Start the timer
    void start()
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    // Stop the timer
    void stop()
    {
        m_end = std::chrono::high_resolution_clock::now();
    }

    // Get elapsed time in milliseconds
    [[nodiscard]] double elapsed_ms() const
    {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start);
        return static_cast<double>(duration.count()) / 1000.0;
    }

    // Get elapsed time in nanoseconds
    [[nodiscard]] double elapsed_ns() const
    {
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(m_end - m_start);
        return static_cast<double>(duration.count());
    }

    // Get elapsed time in seconds
    [[nodiscard]] double elapsed_sec() const
    {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start);
        return static_cast<double>(duration.count()) / 1000000.0;
    }

private:
    std::chrono::high_resolution_clock::time_point m_start;
    std::chrono::high_resolution_clock::time_point m_end;
};

// Flush CPU cache by accessing a buffer larger than cache size
// This ensures cold cache conditions for accurate benchmarking
inline void flush_cache(std::size_t cache_size_bytes)
{
    // Allocate buffer 4x larger than cache to ensure complete eviction
    constexpr std::size_t multiplier = 4;
    std::size_t buffer_size = cache_size_bytes * multiplier / sizeof(double);
    
    auto buffer = std::vector<double>(buffer_size, 0.0);
    
    // Read and write to each element to evict cache lines
    volatile double sum = 0.0;
    for (std::size_t i = 0; i < buffer_size; ++i)
    {
        sum += buffer[i];
        buffer[i] = static_cast<double>(i);
    }
    
    // Prevent compiler optimization
    (void)sum;
}

// Get estimated cache size for flush operation
// Returns L3 cache size if available, otherwise a default value
inline std::size_t get_default_cache_size()
{
    // Default to 16MB if unable to detect (conservative estimate for modern CPUs)
    constexpr std::size_t default_cache_size = 16 * 1024 * 1024;
    return default_cache_size;
}

} // namespace blas_benchmark::utils
