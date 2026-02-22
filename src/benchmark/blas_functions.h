#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <cblas.h>

namespace blas_benchmark
{

// Precision type traits for BLAS functions
// This design allows future extension to single precision and other types
template<typename T>
struct BlasPrecisionTraits;

template<>
struct BlasPrecisionTraits<double>
{
    using value_type = double;
    static constexpr char precision_char = 'd';
    static constexpr const char* name = "double";
};

template<>
struct BlasPrecisionTraits<float>
{
    using value_type = float;
    static constexpr char precision_char = 's';
    static constexpr const char* name = "float";
};

// FLOPS calculation functions for each BLAS operation
namespace flops
{

// Level 1 operations
// ddot: n multiplications + (n-1) additions ≈ 2n FLOPs
constexpr std::size_t dot(std::size_t n)
{
    return 2 * n;
}

// daxpy: n multiplications + n additions = 2n FLOPs
constexpr std::size_t axpy(std::size_t n)
{
    return 2 * n;
}

// dscal: n multiplications = n FLOPs
constexpr std::size_t scal(std::size_t n)
{
    return n;
}

// Level 2 operations
// dgemv: m*n multiplications + m*(n-1) additions ≈ 2mn FLOPs
constexpr std::size_t gemv(std::size_t m, std::size_t n)
{
    return 2 * m * n;
}

// Level 3 operations
// dgemm: m*n*k multiplications + m*n*(k-1) additions ≈ 2mnk FLOPs
constexpr std::size_t gemm(std::size_t m, std::size_t n, std::size_t k)
{
    return 2 * m * n * k;
}

} // namespace flops

// BLAS function wrapper with template support for precision
template<typename T = double>
class BlasWrapper
{
public:
    using value_type = typename BlasPrecisionTraits<T>::value_type;

    // Level 1: Vector-vector operations

    // Dot product: result = x^T * y
    static T dot(std::size_t n, const T* x, int incx, const T* y, int incy)
    {
        if constexpr (std::is_same_v<T, double>)
        {
            return cblas_ddot(static_cast<int>(n), x, incx, y, incy);
        }
        else
        {
            return cblas_sdot(static_cast<int>(n), x, incx, y, incy);
        }
    }

    // AXPY: y = alpha * x + y
    static void axpy(std::size_t n, T alpha, const T* x, int incx, T* y, int incy)
    {
        if constexpr (std::is_same_v<T, double>)
        {
            cblas_daxpy(static_cast<int>(n), alpha, x, incx, y, incy);
        }
        else
        {
            cblas_saxpy(static_cast<int>(n), alpha, x, incx, y, incy);
        }
    }

    // SCAL: x = alpha * x
    static void scal(std::size_t n, T alpha, T* x, int incx)
    {
        if constexpr (std::is_same_v<T, double>)
        {
            cblas_dscal(static_cast<int>(n), alpha, x, incx);
        }
        else
        {
            cblas_sscal(static_cast<int>(n), alpha, x, incx);
        }
    }

    // Level 2: Matrix-vector operations

    // GEMV: y = alpha * A * x + beta * y
    static void gemv(CBLAS_ORDER order, CBLAS_TRANSPOSE trans,
                     std::size_t m, std::size_t n,
                     T alpha, const T* a, int lda,
                     const T* x, int incx,
                     T beta, T* y, int incy)
    {
        if constexpr (std::is_same_v<T, double>)
        {
            cblas_dgemv(order, trans,
                        static_cast<int>(m), static_cast<int>(n),
                        alpha, a, lda, x, incx, beta, y, incy);
        }
        else
        {
            cblas_sgemv(order, trans,
                        static_cast<int>(m), static_cast<int>(n),
                        alpha, a, lda, x, incx, beta, y, incy);
        }
    }

    // Level 3: Matrix-matrix operations

    // GEMM: C = alpha * A * B + beta * C
    static void gemm(CBLAS_ORDER order, CBLAS_TRANSPOSE trans_a, CBLAS_TRANSPOSE trans_b,
                     std::size_t m, std::size_t n, std::size_t k,
                     T alpha, const T* a, int lda,
                     const T* b, int ldb,
                     T beta, T* c, int ldc)
    {
        if constexpr (std::is_same_v<T, double>)
        {
            cblas_dgemm(order, trans_a, trans_b,
                        static_cast<int>(m), static_cast<int>(n), static_cast<int>(k),
                        alpha, a, lda, b, ldb, beta, c, ldc);
        }
        else
        {
            cblas_sgemm(order, trans_a, trans_b,
                        static_cast<int>(m), static_cast<int>(n), static_cast<int>(k),
                        alpha, a, lda, b, ldb, beta, c, ldc);
        }
    }
};

// Type alias for double precision (most common case)
using DBlasWrapper = BlasWrapper<double>;
using SBlasWrapper = BlasWrapper<float>;

// Benchmark function declarations
// These functions run benchmarks and return average time in milliseconds

template<typename T = double>
double benchmark_dot(std::size_t n, std::size_t warmup, std::size_t cycles,
                     bool flush_cache, std::size_t cache_size);

template<typename T = double>
double benchmark_axpy(std::size_t n, std::size_t warmup, std::size_t cycles,
                      bool flush_cache, std::size_t cache_size);

template<typename T = double>
double benchmark_scal(std::size_t n, std::size_t warmup, std::size_t cycles,
                      bool flush_cache, std::size_t cache_size);

template<typename T = double>
double benchmark_gemv(std::size_t m, std::size_t n, std::size_t warmup, std::size_t cycles,
                      bool flush_cache, std::size_t cache_size);

template<typename T = double>
double benchmark_gemm(std::size_t m, std::size_t n, std::size_t k,
                      std::size_t warmup, std::size_t cycles,
                      bool flush_cache, std::size_t cache_size);

// Benchmark function signature
template<typename T = double>
using BenchmarkFunc = std::function<double(
    std::size_t warmup,
    std::size_t cycles,
    bool flush_cache,
    std::size_t cache_size
)>;

} // namespace blas_benchmark
