/* ************************************************************************
 * Copyright 2016-2020 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "logging.hpp"
#include "rocblas_hpmv.hpp"

namespace
{
    template <typename>
    constexpr char rocblas_hpmv_name[] = "unknown";
    template <>
    constexpr char rocblas_hpmv_name<rocblas_float_complex>[] = "rocblas_chpmv_batched";
    template <>
    constexpr char rocblas_hpmv_name<rocblas_double_complex>[] = "rocblas_zhpmv_batched";

    template <typename T>
    rocblas_status rocblas_hpmv_batched_impl(rocblas_handle handle,
                                             rocblas_fill   uplo,
                                             rocblas_int    n,
                                             const T*       alpha,
                                             const T* const AP[],
                                             const T* const x[],
                                             rocblas_int    incx,
                                             const T*       beta,
                                             T* const       y[],
                                             rocblas_int    incy,
                                             rocblas_int    batch_count)
    {
        if(!handle)
            return rocblas_status_invalid_handle;
        RETURN_ZERO_DEVICE_MEMORY_SIZE_IF_QUERIED(handle);

        auto layer_mode     = handle->layer_mode;
        auto check_numerics = handle->check_numerics;
        if(layer_mode
           & (rocblas_layer_mode_log_trace | rocblas_layer_mode_log_bench
              | rocblas_layer_mode_log_profile))
        {
            auto uplo_letter = rocblas_fill_letter(uplo);

            if(layer_mode & rocblas_layer_mode_log_trace)
                log_trace(handle,
                          rocblas_hpmv_name<T>,
                          uplo,
                          n,
                          LOG_TRACE_SCALAR_VALUE(handle, alpha),
                          AP,
                          x,
                          incx,
                          LOG_TRACE_SCALAR_VALUE(handle, beta),
                          y,
                          incy,
                          batch_count);

            if(layer_mode & rocblas_layer_mode_log_bench)
                log_bench(handle,
                          "./rocblas-bench -f hpmv_batched -r",
                          rocblas_precision_string<T>,
                          "--uplo",
                          uplo_letter,
                          "-n",
                          n,
                          LOG_BENCH_SCALAR_VALUE(handle, alpha),
                          "--incx",
                          incx,
                          LOG_BENCH_SCALAR_VALUE(handle, beta),
                          "--incy",
                          incy,
                          "--batch_count",
                          batch_count);

            if(layer_mode & rocblas_layer_mode_log_profile)
                log_profile(handle,
                            rocblas_hpmv_name<T>,
                            "uplo",
                            uplo_letter,
                            "N",
                            n,
                            "incx",
                            incx,
                            "incy",
                            incy,
                            "batch_count",
                            batch_count);
        }

        if(uplo != rocblas_fill_lower && uplo != rocblas_fill_upper)
            return rocblas_status_invalid_value;

        if(n < 0 || !incx || !incy || batch_count < 0)
            return rocblas_status_invalid_size;

        if(!n || !batch_count)
            return rocblas_status_success;

        if(!alpha || !beta)
            return rocblas_status_invalid_pointer;

        if(handle->pointer_mode == rocblas_pointer_mode_host && !*alpha)
        {
            if(*beta == 1)
                return rocblas_status_success;
        }
        else if(!AP || !x)
            return rocblas_status_invalid_pointer;

        if(!y)
            return rocblas_status_invalid_pointer;

        constexpr rocblas_int    offset_A = 0, offset_x = 0, offset_y = 0;
        constexpr rocblas_stride stride_A = 0, stride_x = 0, stride_y = 0;

        if(check_numerics)
        {
            bool           is_input = true;
            rocblas_status hpmv_check_numerics_status
                = rocblas_hpmv_check_numerics(rocblas_hpmv_name<T>,
                                              handle,
                                              n,
                                              AP,
                                              offset_A,
                                              stride_A,
                                              x,
                                              offset_x,
                                              incx,
                                              stride_x,
                                              y,
                                              offset_y,
                                              incy,
                                              stride_y,
                                              batch_count,
                                              check_numerics,
                                              is_input);
            if(hpmv_check_numerics_status != rocblas_status_success)
                return hpmv_check_numerics_status;
        }

        rocblas_status status = rocblas_hpmv_template(handle,
                                                      uplo,
                                                      n,
                                                      alpha,
                                                      AP,
                                                      offset_A,
                                                      stride_A,
                                                      x,
                                                      offset_x,
                                                      incx,
                                                      stride_x,
                                                      beta,
                                                      y,
                                                      offset_y,
                                                      incy,
                                                      stride_y,
                                                      batch_count);
        if(status != rocblas_status_success)
            return status;
        if(check_numerics)
        {
            bool           is_input = false;
            rocblas_status hpmv_check_numerics_status
                = rocblas_hpmv_check_numerics(rocblas_hpmv_name<T>,
                                              handle,
                                              n,
                                              AP,
                                              offset_A,
                                              stride_A,
                                              x,
                                              offset_x,
                                              incx,
                                              stride_x,
                                              y,
                                              offset_y,
                                              incy,
                                              stride_y,
                                              batch_count,
                                              check_numerics,
                                              is_input);
            if(hpmv_check_numerics_status != rocblas_status_success)
                return hpmv_check_numerics_status;
        }
        return status;
    }

} // namespace

/*
* ===========================================================================
*    C wrapper
* ===========================================================================
*/

extern "C" {

rocblas_status rocblas_chpmv_batched(rocblas_handle                     handle,
                                     rocblas_fill                       uplo,
                                     rocblas_int                        n,
                                     const rocblas_float_complex*       alpha,
                                     const rocblas_float_complex* const AP[],
                                     const rocblas_float_complex* const x[],
                                     rocblas_int                        incx,
                                     const rocblas_float_complex*       beta,
                                     rocblas_float_complex* const       y[],
                                     rocblas_int                        incy,
                                     rocblas_int                        batch_count)
try
{
    return rocblas_hpmv_batched_impl(
        handle, uplo, n, alpha, AP, x, incx, beta, y, incy, batch_count);
}
catch(...)
{
    return exception_to_rocblas_status();
}

rocblas_status rocblas_zhpmv_batched(rocblas_handle                      handle,
                                     rocblas_fill                        uplo,
                                     rocblas_int                         n,
                                     const rocblas_double_complex*       alpha,
                                     const rocblas_double_complex* const AP[],
                                     const rocblas_double_complex* const x[],
                                     rocblas_int                         incx,
                                     const rocblas_double_complex*       beta,
                                     rocblas_double_complex* const       y[],
                                     rocblas_int                         incy,
                                     rocblas_int                         batch_count)
try
{
    return rocblas_hpmv_batched_impl(
        handle, uplo, n, alpha, AP, x, incx, beta, y, incy, batch_count);
}
catch(...)
{
    return exception_to_rocblas_status();
}

} // extern "C"
