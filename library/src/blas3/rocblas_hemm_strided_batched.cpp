/* ************************************************************************
 * Copyright 2016-2021 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "logging.hpp"
#include "rocblas_hemm.hpp"
#include "utility.hpp"

namespace
{
    template <typename>
    constexpr char rocblas_hemm_name[] = "unknown";
    template <>
    constexpr char rocblas_hemm_name<rocblas_float_complex>[] = "rocblas_chemm_strided_batched";
    template <>
    constexpr char rocblas_hemm_name<rocblas_double_complex>[] = "rocblas_zhemm_strided_batched";

    template <typename T>
    rocblas_status rocblas_hemm_strided_batched_impl(rocblas_handle handle,
                                                     rocblas_side   side,
                                                     rocblas_fill   uplo,
                                                     rocblas_int    m,
                                                     rocblas_int    n,
                                                     const T*       alpha,
                                                     const T*       A,
                                                     rocblas_int    lda,
                                                     rocblas_stride stride_a,
                                                     const T*       B,
                                                     rocblas_int    ldb,
                                                     rocblas_stride stride_b,
                                                     const T*       beta,
                                                     T*             C,
                                                     rocblas_int    ldc,
                                                     rocblas_stride stride_c,
                                                     rocblas_int    batch_count)
    {
        if(!handle)
            return rocblas_status_invalid_handle;

        RETURN_ZERO_DEVICE_MEMORY_SIZE_IF_QUERIED(handle);

        auto layer_mode = handle->layer_mode;
        if(layer_mode
           & (rocblas_layer_mode_log_trace | rocblas_layer_mode_log_bench
              | rocblas_layer_mode_log_profile))
        {
            auto side_letter = rocblas_side_letter(side);
            auto uplo_letter = rocblas_fill_letter(uplo);

            if(layer_mode & rocblas_layer_mode_log_trace)
                log_trace(handle,
                          rocblas_hemm_name<T>,
                          side,
                          uplo,
                          m,
                          n,
                          LOG_TRACE_SCALAR_VALUE(handle, alpha),
                          A,
                          lda,
                          stride_a,
                          B,
                          ldb,
                          stride_b,
                          LOG_TRACE_SCALAR_VALUE(handle, beta),
                          C,
                          ldc,
                          stride_c,
                          batch_count);

            if(layer_mode & rocblas_layer_mode_log_bench)
                log_bench(handle,
                          "./rocblas-bench -f hemm_strided_batched -r",
                          rocblas_precision_string<T>,
                          "--side",
                          side_letter,
                          "--uplo",
                          uplo_letter,
                          "-m",
                          m,
                          "-n",
                          n,
                          LOG_BENCH_SCALAR_VALUE(handle, alpha),
                          "--lda",
                          lda,
                          "--stride_a",
                          stride_a,
                          "--ldb",
                          ldb,
                          "--stride_b",
                          stride_b,
                          LOG_BENCH_SCALAR_VALUE(handle, beta),
                          "--ldc",
                          ldc,
                          "--stride_c",
                          stride_c,
                          "--batch_count",
                          batch_count);

            if(layer_mode & rocblas_layer_mode_log_profile)
                log_profile(handle,
                            rocblas_hemm_name<T>,
                            "side",
                            side_letter,
                            "uplo",
                            uplo_letter,
                            "M",
                            m,
                            "N",
                            n,
                            "lda",
                            lda,
                            "stride_a",
                            stride_a,
                            "ldb",
                            ldb,
                            "stride_b",
                            stride_b,
                            "ldc",
                            ldc,
                            "stride_c",
                            stride_c,
                            "batch_count",
                            batch_count);
        }

        static constexpr rocblas_int offset_C = 0, offset_A = 0, offset_B = 0;

        // equivalent argument constraints for symm and hemm_batched
        rocblas_status arg_status = rocblas_symm_arg_check(handle,
                                                           side,
                                                           uplo,
                                                           m,
                                                           n,
                                                           alpha,
                                                           A,
                                                           offset_A,
                                                           lda,
                                                           stride_a,
                                                           B,
                                                           offset_B,
                                                           ldb,
                                                           stride_b,
                                                           beta,
                                                           C,
                                                           offset_C,
                                                           ldc,
                                                           stride_c,
                                                           batch_count);
        if(arg_status != rocblas_status_continue)
            return arg_status;

        static constexpr bool Hermetian = true;
        return rocblas_internal_symm_template<Hermetian>(handle,
                                                         side,
                                                         uplo,
                                                         m,
                                                         n,
                                                         alpha,
                                                         A,
                                                         offset_A,
                                                         lda,
                                                         stride_a,
                                                         B,
                                                         offset_B,
                                                         ldb,
                                                         stride_b,
                                                         beta,
                                                         C,
                                                         offset_C,
                                                         ldc,
                                                         stride_c,
                                                         batch_count);
    }

}
/*
 * ===========================================================================
 *    C wrapper
 * ===========================================================================
 */

extern "C" {

#ifdef IMPL
#error IMPL ALREADY DEFINED
#endif

#define IMPL(routine_name_, T_)                                \
    rocblas_status routine_name_(rocblas_handle handle,        \
                                 rocblas_side   side,          \
                                 rocblas_fill   uplo,          \
                                 rocblas_int    m,             \
                                 rocblas_int    n,             \
                                 const T_*      alpha,         \
                                 const T_*      A,             \
                                 rocblas_int    lda,           \
                                 rocblas_stride stride_a,      \
                                 const T_*      B,             \
                                 rocblas_int    ldb,           \
                                 rocblas_stride stride_b,      \
                                 const T_*      beta,          \
                                 T_*            C,             \
                                 rocblas_int    ldc,           \
                                 rocblas_stride stride_c,      \
                                 rocblas_int    batch_count)   \
    try                                                        \
    {                                                          \
        return rocblas_hemm_strided_batched_impl(handle,       \
                                                 side,         \
                                                 uplo,         \
                                                 m,            \
                                                 n,            \
                                                 alpha,        \
                                                 A,            \
                                                 lda,          \
                                                 stride_a,     \
                                                 B,            \
                                                 ldb,          \
                                                 stride_b,     \
                                                 beta,         \
                                                 C,            \
                                                 ldc,          \
                                                 stride_c,     \
                                                 batch_count); \
    }                                                          \
    catch(...)                                                 \
    {                                                          \
        return exception_to_rocblas_status();                  \
    }

IMPL(rocblas_chemm_strided_batched, rocblas_float_complex);
IMPL(rocblas_zhemm_strided_batched, rocblas_double_complex);

#undef IMPL

} // extern "C"
