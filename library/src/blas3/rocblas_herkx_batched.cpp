/* ************************************************************************
 * Copyright 2016-2021 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "logging.hpp"
#include "rocblas_herkx.hpp"
#include "utility.hpp"

namespace
{
    template <typename>
    constexpr char rocblas_herkx_name[] = "unknown";
    template <>
    constexpr char rocblas_herkx_name<rocblas_float_complex>[] = "rocblas_cherkx_batched";
    template <>
    constexpr char rocblas_herkx_name<rocblas_double_complex>[] = "rocblas_zherkx_batched";

    template <typename T>
    rocblas_status rocblas_herkx_batched_impl(rocblas_handle    handle,
                                              rocblas_fill      uplo,
                                              rocblas_operation trans,
                                              rocblas_int       n,
                                              rocblas_int       k,
                                              const T*          alpha,
                                              const T* const    A[],
                                              rocblas_int       lda,
                                              const T* const    B[],
                                              rocblas_int       ldb,
                                              const real_t<T>*  beta,
                                              T* const          C[],
                                              rocblas_int       ldc,
                                              rocblas_int       batch_count)
    {
        if(!handle)
            return rocblas_status_invalid_handle;

        RETURN_ZERO_DEVICE_MEMORY_SIZE_IF_QUERIED(handle);

        auto layer_mode = handle->layer_mode;
        if(layer_mode
           & (rocblas_layer_mode_log_trace | rocblas_layer_mode_log_bench
              | rocblas_layer_mode_log_profile))
        {
            auto uplo_letter   = rocblas_fill_letter(uplo);
            auto transA_letter = rocblas_transpose_letter(trans);

            if(layer_mode & rocblas_layer_mode_log_trace)
                log_trace(handle,
                          rocblas_herkx_name<T>,
                          uplo,
                          trans,
                          n,
                          k,
                          LOG_TRACE_SCALAR_VALUE(handle, alpha),
                          A,
                          lda,
                          B,
                          ldb,
                          LOG_TRACE_SCALAR_VALUE(handle, beta),
                          C,
                          ldc,
                          batch_count);

            if(layer_mode & rocblas_layer_mode_log_bench)
                log_bench(handle,
                          "./rocblas-bench -f herkx_batched -r",
                          rocblas_precision_string<T>,
                          "--uplo",
                          uplo_letter,
                          "--transposeA",
                          transA_letter,
                          "-n",
                          n,
                          "-k",
                          k,
                          LOG_BENCH_SCALAR_VALUE(handle, alpha),
                          "--lda",
                          lda,
                          "--ldb",
                          ldb,
                          LOG_BENCH_SCALAR_VALUE(handle, beta),
                          "--ldc",
                          ldc,
                          "--batch_count",
                          batch_count);

            if(layer_mode & rocblas_layer_mode_log_profile)
                log_profile(handle,
                            rocblas_herkx_name<T>,
                            "uplo",
                            uplo_letter,
                            "trans",
                            transA_letter,
                            "N",
                            n,
                            "K",
                            k,
                            "lda",
                            lda,
                            "ldb",
                            ldb,
                            "ldc",
                            ldc,
                            "batch_count",
                            batch_count);
        }

        static constexpr rocblas_int    offset_C = 0, offset_A = 0, offset_B = 0;
        static constexpr rocblas_stride stride_C = 0, stride_A = 0, stride_B = 0;

        // her2k arg check is equivalent
        rocblas_status arg_status = rocblas_her2k_arg_check(handle,
                                                            uplo,
                                                            trans,
                                                            n,
                                                            k,
                                                            alpha,
                                                            A,
                                                            offset_A,
                                                            lda,
                                                            stride_A,
                                                            B,
                                                            offset_B,
                                                            ldb,
                                                            stride_B,
                                                            beta,
                                                            C,
                                                            offset_C,
                                                            ldc,
                                                            stride_C,
                                                            batch_count);
        if(arg_status != rocblas_status_continue)
            return arg_status;

        static constexpr bool is2K = false; // herkx
        return rocblas_internal_her2k_template<is2K>(handle,
                                                     uplo,
                                                     trans,
                                                     n,
                                                     k,
                                                     alpha,
                                                     A,
                                                     offset_A,
                                                     lda,
                                                     stride_A,
                                                     B,
                                                     offset_B,
                                                     ldb,
                                                     stride_B,
                                                     beta,
                                                     C,
                                                     offset_C,
                                                     ldc,
                                                     stride_C,
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

#define IMPL(routine_name_, S_, T_)                                                       \
    rocblas_status routine_name_(rocblas_handle    handle,                                \
                                 rocblas_fill      uplo,                                  \
                                 rocblas_operation trans,                                 \
                                 rocblas_int       n,                                     \
                                 rocblas_int       k,                                     \
                                 const T_*         alpha,                                 \
                                 const T_* const   A[],                                   \
                                 rocblas_int       lda,                                   \
                                 const T_* const   B[],                                   \
                                 rocblas_int       ldb,                                   \
                                 const S_*         beta,                                  \
                                 T_* const         C[],                                   \
                                 rocblas_int       ldc,                                   \
                                 rocblas_int       batch_count)                           \
    try                                                                                   \
    {                                                                                     \
        return rocblas_herkx_batched_impl(                                                \
            handle, uplo, trans, n, k, alpha, A, lda, B, ldb, beta, C, ldc, batch_count); \
    }                                                                                     \
    catch(...)                                                                            \
    {                                                                                     \
        return exception_to_rocblas_status();                                             \
    }

IMPL(rocblas_cherkx_batched, float, rocblas_float_complex);
IMPL(rocblas_zherkx_batched, double, rocblas_double_complex);

#undef IMPL

} // extern "C"
