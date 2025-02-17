/* ************************************************************************
 * Copyright 2016-2021 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "handle.hpp"
#include "logging.hpp"
#include "rocblas.h"
#include "rocblas_gemv.hpp"
#include "utility.hpp"

namespace
{
    template <typename>
    constexpr char rocblas_gemv_name[] = "unknown";
    template <>
    constexpr char rocblas_gemv_name<float>[] = "rocblas_sgemv_strided_batched";
    template <>
    constexpr char rocblas_gemv_name<double>[] = "rocblas_dgemv_strided_batched";
    template <>
    constexpr char rocblas_gemv_name<rocblas_float_complex>[] = "rocblas_cgemv_strided_batched";
    template <>
    constexpr char rocblas_gemv_name<rocblas_double_complex>[] = "rocblas_zgemv_strided_batched";

    template <typename T>
    rocblas_status rocblas_gemv_strided_batched_impl(rocblas_handle    handle,
                                                     rocblas_operation transA,
                                                     rocblas_int       m,
                                                     rocblas_int       n,
                                                     const T*          alpha,
                                                     const T*          A,
                                                     rocblas_int       lda,
                                                     rocblas_stride    strideA,
                                                     const T*          x,
                                                     rocblas_int       incx,
                                                     rocblas_stride    stridex,
                                                     const T*          beta,
                                                     T*                y,
                                                     rocblas_int       incy,
                                                     rocblas_stride    stridey,
                                                     rocblas_int       batch_count)
    {
        if(!handle)
            return rocblas_status_invalid_handle;

        size_t dev_bytes
            = rocblas_internal_gemv_kernel_workspace_size<T>(transA, m, n, batch_count);
        if(handle->is_device_memory_size_query())
            return handle->set_optimal_device_memory_size(dev_bytes);

        auto layer_mode     = handle->layer_mode;
        auto check_numerics = handle->check_numerics;

        if(layer_mode
           & (rocblas_layer_mode_log_trace | rocblas_layer_mode_log_bench
              | rocblas_layer_mode_log_profile))
        {
            auto transA_letter = rocblas_transpose_letter(transA);

            if(layer_mode & rocblas_layer_mode_log_trace)
                log_trace(handle,
                          rocblas_gemv_name<T>,
                          transA,
                          m,
                          n,
                          LOG_TRACE_SCALAR_VALUE(handle, alpha),
                          A,
                          lda,
                          strideA,
                          x,
                          incx,
                          stridex,
                          LOG_TRACE_SCALAR_VALUE(handle, beta),
                          y,
                          incy,
                          stridey,
                          batch_count);

            if(layer_mode & rocblas_layer_mode_log_bench)
                log_bench(handle,
                          "./rocblas-bench -f gemv_strided_batched -r",
                          rocblas_precision_string<T>,
                          "--transposeA",
                          transA_letter,
                          "-m",
                          m,
                          "-n",
                          n,
                          LOG_BENCH_SCALAR_VALUE(handle, alpha),
                          "--lda",
                          lda,
                          "--stride_a",
                          strideA,
                          "--incx",
                          incx,
                          "--stride_x",
                          stridex,
                          LOG_BENCH_SCALAR_VALUE(handle, beta),
                          "--incy",
                          incy,
                          "--stride_y",
                          stridey,
                          "--batch_count",
                          batch_count);

            if(layer_mode & rocblas_layer_mode_log_profile)
                log_profile(handle,
                            rocblas_gemv_name<T>,
                            "transA",
                            transA_letter,
                            "M",
                            m,
                            "N",
                            n,
                            "lda",
                            lda,
                            "stride_a",
                            strideA,
                            "incx",
                            incx,
                            "stride_x",
                            stridex,
                            "incy",
                            incy,
                            "stride_y",
                            stridey,
                            "batch_count",
                            batch_count);
        }

        if(m < 0 || n < 0 || lda < m || lda < 1 || !incx || !incy || batch_count < 0)
            return rocblas_status_invalid_size;

        if(!batch_count || !m || !n)
            return rocblas_status_success;

        if(!alpha || !beta)
            return rocblas_status_invalid_pointer;

        if(handle->pointer_mode == rocblas_pointer_mode_host && !*alpha)
        {
            if(*beta == 1)
                return rocblas_status_success;
        }
        else if(!A || !x)
            return rocblas_status_invalid_pointer;
        if(!y)
            return rocblas_status_invalid_pointer;

        rocblas_status perf_status = rocblas_status_success;
        auto           w_mem       = handle->device_malloc(dev_bytes);
        if(!w_mem)
            perf_status = rocblas_status_perf_degraded;

        if(check_numerics)
        {
            bool           is_input = true;
            rocblas_status gemv_check_numerics_status
                = rocblas_gemv_check_numerics(rocblas_gemv_name<T>,
                                              handle,
                                              transA,
                                              m,
                                              n,
                                              A,
                                              0,
                                              lda,
                                              strideA,
                                              x,
                                              0,
                                              incx,
                                              stridex,
                                              y,
                                              0,
                                              incy,
                                              stridey,
                                              batch_count,
                                              check_numerics,
                                              is_input);
            if(gemv_check_numerics_status != rocblas_status_success)
                return gemv_check_numerics_status;
        }

        rocblas_status status = rocblas_internal_gemv_template<T>(handle,
                                                                  transA,
                                                                  m,
                                                                  n,
                                                                  alpha,
                                                                  0,
                                                                  A,
                                                                  0,
                                                                  lda,
                                                                  strideA,
                                                                  x,
                                                                  0,
                                                                  incx,
                                                                  stridex,
                                                                  beta,
                                                                  0,
                                                                  y,
                                                                  0,
                                                                  incy,
                                                                  stridey,
                                                                  batch_count,
                                                                  (T*)w_mem);

        status = (status != rocblas_status_success) ? status : perf_status;
        if(status != rocblas_status_success)
            return status;

        if(check_numerics)
        {
            bool           is_input = false;
            rocblas_status gemv_check_numerics_status
                = rocblas_gemv_check_numerics(rocblas_gemv_name<T>,
                                              handle,
                                              transA,
                                              m,
                                              n,
                                              A,
                                              0,
                                              lda,
                                              strideA,
                                              x,
                                              0,
                                              incx,
                                              stridex,
                                              y,
                                              0,
                                              incy,
                                              stridey,
                                              batch_count,
                                              check_numerics,
                                              is_input);
            if(gemv_check_numerics_status != rocblas_status_success)
                return gemv_check_numerics_status;
        }
        return status;
    }
} //namespace

/*
* ===========================================================================
*    C wrapper
* ===========================================================================
*/

extern "C" {

rocblas_status rocblas_sgemv_strided_batched(rocblas_handle    handle,
                                             rocblas_operation transA,
                                             rocblas_int       m,
                                             rocblas_int       n,
                                             const float*      alpha,
                                             const float*      A,
                                             rocblas_int       lda,
                                             rocblas_stride    strideA,
                                             const float*      x,
                                             rocblas_int       incx,
                                             rocblas_stride    stridex,
                                             const float*      beta,
                                             float*            y,
                                             rocblas_int       incy,
                                             rocblas_stride    stridey,
                                             rocblas_int       batch_count)
try
{
    return rocblas_gemv_strided_batched_impl(handle,
                                             transA,
                                             m,
                                             n,
                                             alpha,
                                             A,
                                             lda,
                                             strideA,
                                             x,
                                             incx,
                                             stridex,
                                             beta,
                                             y,
                                             incy,
                                             stridey,
                                             batch_count);
}
catch(...)
{
    return exception_to_rocblas_status();
}

rocblas_status rocblas_dgemv_strided_batched(rocblas_handle    handle,
                                             rocblas_operation transA,
                                             rocblas_int       m,
                                             rocblas_int       n,
                                             const double*     alpha,
                                             const double*     A,
                                             rocblas_int       lda,
                                             rocblas_stride    strideA,
                                             const double*     x,
                                             rocblas_int       incx,
                                             rocblas_stride    stridex,
                                             const double*     beta,
                                             double*           y,
                                             rocblas_int       incy,
                                             rocblas_stride    stridey,
                                             rocblas_int       batch_count)
try
{
    return rocblas_gemv_strided_batched_impl(handle,
                                             transA,
                                             m,
                                             n,
                                             alpha,
                                             A,
                                             lda,
                                             strideA,
                                             x,
                                             incx,
                                             stridex,
                                             beta,
                                             y,
                                             incy,
                                             stridey,
                                             batch_count);
}
catch(...)
{
    return exception_to_rocblas_status();
}

rocblas_status rocblas_cgemv_strided_batched(rocblas_handle               handle,
                                             rocblas_operation            transA,
                                             rocblas_int                  m,
                                             rocblas_int                  n,
                                             const rocblas_float_complex* alpha,
                                             const rocblas_float_complex* A,
                                             rocblas_int                  lda,
                                             rocblas_stride               strideA,
                                             const rocblas_float_complex* x,
                                             rocblas_int                  incx,
                                             rocblas_stride               stridex,
                                             const rocblas_float_complex* beta,
                                             rocblas_float_complex*       y,
                                             rocblas_int                  incy,
                                             rocblas_stride               stridey,
                                             rocblas_int                  batch_count)
try
{
    return rocblas_gemv_strided_batched_impl(handle,
                                             transA,
                                             m,
                                             n,
                                             alpha,
                                             A,
                                             lda,
                                             strideA,
                                             x,
                                             incx,
                                             stridex,
                                             beta,
                                             y,
                                             incy,
                                             stridey,
                                             batch_count);
}
catch(...)
{
    return exception_to_rocblas_status();
}

rocblas_status rocblas_zgemv_strided_batched(rocblas_handle                handle,
                                             rocblas_operation             transA,
                                             rocblas_int                   m,
                                             rocblas_int                   n,
                                             const rocblas_double_complex* alpha,
                                             const rocblas_double_complex* A,
                                             rocblas_int                   lda,
                                             rocblas_stride                strideA,
                                             const rocblas_double_complex* x,
                                             rocblas_int                   incx,
                                             rocblas_stride                stridex,
                                             const rocblas_double_complex* beta,
                                             rocblas_double_complex*       y,
                                             rocblas_int                   incy,
                                             rocblas_stride                stridey,
                                             rocblas_int                   batch_count)
try
{
    return rocblas_gemv_strided_batched_impl(handle,
                                             transA,
                                             m,
                                             n,
                                             alpha,
                                             A,
                                             lda,
                                             strideA,
                                             x,
                                             incx,
                                             stridex,
                                             beta,
                                             y,
                                             incy,
                                             stridey,
                                             batch_count);
}
catch(...)
{
    return exception_to_rocblas_status();
}

} // extern "C"
