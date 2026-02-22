#include "benchmark/blas_functions.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include <spdlog/spdlog.h>

#include "utils/timer.h"

namespace blas_benchmark
{

namespace
{

// Generate random data for testing
template<typename T>
std::vector<T> generate_random_data(std::size_t size, T min_val = static_cast<T>(-1.0), T max_val = static_cast<T>(1.0))
{
    std::vector<T> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    if constexpr (std::is_floating_point_v<T>)
    {
        std::uniform_real_distribution<T> dist(min_val, max_val);
        std::generate(data.begin(), data.end(), [&]() { return dist(gen); });
    }
    else
    {
        std::uniform_int_distribution<> dist(static_cast<int>(min_val), static_cast<int>(max_val));
        std::generate(data.begin(), data.end(), [&]() { return static_cast<T>(dist(gen)); });
    }
    
    return data;
}

} // anonymous namespace

// Benchmark runner functions for each BLAS operation

template<typename T>
double benchmark_dot(std::size_t n, std::size_t warmup, std::size_t cycles, 
                     bool flush_cache, std::size_t cache_size)
{
    // Allocate and initialize data
    auto x = generate_random_data<T>(n);
    auto y = generate_random_data<T>(n);
    
    T result = static_cast<T>(0);
    
    // Warmup runs
    for (std::size_t i = 0; i < warmup; ++i)
    {
        if (flush_cache)
        {
            utils::flush_cache(cache_size);
        }
        result = BlasWrapper<T>::dot(n, x.data(), 1, y.data(), 1);
    }
    (void)result; // Suppress unused variable warning
    
    // Benchmark runs
    utils::Timer timer;
    double total_time = 0.0;
    
    for (std::size_t i = 0; i < cycles; ++i)
    {
        if (flush_cache)
        {
            utils::flush_cache(cache_size);
        }
        
        timer.start();
        result = BlasWrapper<T>::dot(n, x.data(), 1, y.data(), 1);
        timer.stop();
        
        total_time += timer.elapsed_ms();
    }
    
    return total_time / static_cast<double>(cycles);
}

template<typename T>
double benchmark_axpy(std::size_t n, std::size_t warmup, std::size_t cycles,
                      bool flush_cache, std::size_t cache_size)
{
    auto x = generate_random_data<T>(n);
    auto y = generate_random_data<T>(n);
    T alpha = static_cast<T>(0.5);
    
    // Warmup runs
    for (std::size_t i = 0; i < warmup; ++i)
    {
        if (flush_cache)
        {
            utils::flush_cache(cache_size);
        }
        BlasWrapper<T>::axpy(n, alpha, x.data(), 1, y.data(), 1);
    }
    
    // Benchmark runs
    utils::Timer timer;
    double total_time = 0.0;
    
    for (std::size_t i = 0; i < cycles; ++i)
    {
        if (flush_cache)
        {
            utils::flush_cache(cache_size);
        }
        
        timer.start();
        BlasWrapper<T>::axpy(n, alpha, x.data(), 1, y.data(), 1);
        timer.stop();
        
        total_time += timer.elapsed_ms();
    }
    
    return total_time / static_cast<double>(cycles);
}

template<typename T>
double benchmark_scal(std::size_t n, std::size_t warmup, std::size_t cycles,
                      bool flush_cache, std::size_t cache_size)
{
    auto x = generate_random_data<T>(n);
    T alpha = static_cast<T>(2.0);
    
    // Warmup runs
    for (std::size_t i = 0; i < warmup; ++i)
    {
        if (flush_cache)
        {
            utils::flush_cache(cache_size);
        }
        BlasWrapper<T>::scal(n, alpha, x.data(), 1);
    }
    
    // Benchmark runs
    utils::Timer timer;
    double total_time = 0.0;
    
    for (std::size_t i = 0; i < cycles; ++i)
    {
        if (flush_cache)
        {
            utils::flush_cache(cache_size);
        }
        
        timer.start();
        BlasWrapper<T>::scal(n, alpha, x.data(), 1);
        timer.stop();
        
        total_time += timer.elapsed_ms();
    }
    
    return total_time / static_cast<double>(cycles);
}

template<typename T>
double benchmark_gemv(std::size_t m, std::size_t n, std::size_t warmup, std::size_t cycles,
                      bool flush_cache, std::size_t cache_size)
{
    auto a = generate_random_data<T>(m * n);
    auto x = generate_random_data<T>(n);
    auto y = generate_random_data<T>(m);
    T alpha = static_cast<T>(1.0);
    T beta = static_cast<T>(0.0);
    
    // Warmup runs
    for (std::size_t i = 0; i < warmup; ++i)
    {
        if (flush_cache)
        {
            utils::flush_cache(cache_size);
        }
        BlasWrapper<T>::gemv(CblasRowMajor, CblasNoTrans, m, n, alpha, a.data(), 
                             static_cast<int>(n), x.data(), 1, beta, y.data(), 1);
    }
    
    // Benchmark runs
    utils::Timer timer;
    double total_time = 0.0;
    
    for (std::size_t i = 0; i < cycles; ++i)
    {
        if (flush_cache)
        {
            utils::flush_cache(cache_size);
        }
        
        timer.start();
        BlasWrapper<T>::gemv(CblasRowMajor, CblasNoTrans, m, n, alpha, a.data(),
                             static_cast<int>(n), x.data(), 1, beta, y.data(), 1);
        timer.stop();
        
        total_time += timer.elapsed_ms();
    }
    
    return total_time / static_cast<double>(cycles);
}

template<typename T>
double benchmark_gemm(std::size_t m, std::size_t n, std::size_t k, 
                      std::size_t warmup, std::size_t cycles,
                      bool flush_cache, std::size_t cache_size)
{
    auto a = generate_random_data<T>(m * k);
    auto b = generate_random_data<T>(k * n);
    auto c = generate_random_data<T>(m * n);
    T alpha = static_cast<T>(1.0);
    T beta = static_cast<T>(0.0);
    
    spdlog::debug("Benchmarking GEMM: M={}, N={}, K={}", m, n, k);
    
    // Warmup runs
    for (std::size_t i = 0; i < warmup; ++i)
    {
        if (flush_cache)
        {
            utils::flush_cache(cache_size);
        }
        BlasWrapper<T>::gemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                             m, n, k, alpha, a.data(), static_cast<int>(k),
                             b.data(), static_cast<int>(n), beta, c.data(), static_cast<int>(n));
    }
    
    // Benchmark runs
    utils::Timer timer;
    double total_time = 0.0;
    
    for (std::size_t i = 0; i < cycles; ++i)
    {
        if (flush_cache)
        {
            utils::flush_cache(cache_size);
        }
        
        timer.start();
        BlasWrapper<T>::gemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                             m, n, k, alpha, a.data(), static_cast<int>(k),
                             b.data(), static_cast<int>(n), beta, c.data(), static_cast<int>(n));
        timer.stop();
        
        total_time += timer.elapsed_ms();
        spdlog::debug("Iteration {}: {} ms", i, timer.elapsed_ms());
    }
    
    return total_time / static_cast<double>(cycles);
}

// Explicit template instantiation for double precision
template double benchmark_dot<double>(std::size_t n, std::size_t warmup, std::size_t cycles,
                                       bool flush_cache, std::size_t cache_size);
template double benchmark_axpy<double>(std::size_t n, std::size_t warmup, std::size_t cycles,
                                        bool flush_cache, std::size_t cache_size);
template double benchmark_scal<double>(std::size_t n, std::size_t warmup, std::size_t cycles,
                                        bool flush_cache, std::size_t cache_size);
template double benchmark_gemv<double>(std::size_t m, std::size_t n, std::size_t warmup, std::size_t cycles,
                                        bool flush_cache, std::size_t cache_size);
template double benchmark_gemm<double>(std::size_t m, std::size_t n, std::size_t k,
                                        std::size_t warmup, std::size_t cycles,
                                        bool flush_cache, std::size_t cache_size);

} // namespace blas_benchmark
