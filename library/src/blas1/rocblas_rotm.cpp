/* ************************************************************************
 * Copyright 2016-2020 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "rocblas_rotm.hpp"
#include "handle.hpp"
#include "logging.hpp"
#include "rocblas.h"
#include "utility.hpp"

namespace
{
    constexpr int NB = 512;

    template <typename>
    constexpr char rocblas_rotm_name[] = "unknown";
    template <>
    constexpr char rocblas_rotm_name<float>[] = "rocblas_srotm";
    template <>
    constexpr char rocblas_rotm_name<double>[] = "rocblas_drotm";

    template <class T>
    rocblas_status rocblas_rotm_impl(rocblas_handle handle,
                                     rocblas_int    n,
                                     T*             x,
                                     rocblas_int    incx,
                                     T*             y,
                                     rocblas_int    incy,
                                     const T*       param)
    {
        if(!handle)
            return rocblas_status_invalid_handle;

        RETURN_ZERO_DEVICE_MEMORY_SIZE_IF_QUERIED(handle);

        auto layer_mode     = handle->layer_mode;
        auto check_numerics = handle->check_numerics;
        if(layer_mode & rocblas_layer_mode_log_trace)
            log_trace(handle, rocblas_rotm_name<T>, n, x, incx, y, incy, param);
        if(layer_mode & rocblas_layer_mode_log_bench)
            log_bench(handle,
                      "./rocblas-bench -f rotm -r",
                      rocblas_precision_string<T>,
                      "-n",
                      n,
                      "--incx",
                      incx,
                      "--incy",
                      incy);
        if(layer_mode & rocblas_layer_mode_log_profile)
            log_profile(handle, rocblas_rotm_name<T>, "N", n, "incx", incx, "incy", incy);

        if(n <= 0)
            return rocblas_status_success;

        if(!param)
            return rocblas_status_invalid_pointer;

        if(quick_return_param(handle, param, 0))
            return rocblas_status_success;

        if(!x || !y)
            return rocblas_status_invalid_pointer;

        if(check_numerics)
        {
            bool           is_input = true;
            rocblas_status rotm_check_numerics_status
                = rocblas_rotm_check_numerics(rocblas_rotm_name<T>,
                                              handle,
                                              n,
                                              x,
                                              0,
                                              incx,
                                              0,
                                              y,
                                              0,
                                              incy,
                                              0,
                                              1,
                                              check_numerics,
                                              is_input);
            if(rotm_check_numerics_status != rocblas_status_success)
                return rotm_check_numerics_status;
        }

        rocblas_status status = rocblas_rotm_template<NB, false>(
            handle, n, x, 0, incx, 0, y, 0, incy, 0, param, 0, 0, 1);
        if(status != rocblas_status_success)
            return status;

        if(check_numerics)
        {
            bool           is_input = false;
            rocblas_status rotm_check_numerics_status
                = rocblas_rotm_check_numerics(rocblas_rotm_name<T>,
                                              handle,
                                              n,
                                              x,
                                              0,
                                              incx,
                                              0,
                                              y,
                                              0,
                                              incy,
                                              0,
                                              1,
                                              check_numerics,
                                              is_input);
            if(rotm_check_numerics_status != rocblas_status_success)
                return rotm_check_numerics_status;
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

ROCBLAS_EXPORT rocblas_status rocblas_srotm(rocblas_handle handle,
                                            rocblas_int    n,
                                            float*         x,
                                            rocblas_int    incx,
                                            float*         y,
                                            rocblas_int    incy,
                                            const float*   param)
try
{
    return rocblas_rotm_impl(handle, n, x, incx, y, incy, param);
}
catch(...)
{
    return exception_to_rocblas_status();
}

ROCBLAS_EXPORT rocblas_status rocblas_drotm(rocblas_handle handle,
                                            rocblas_int    n,
                                            double*        x,
                                            rocblas_int    incx,
                                            double*        y,
                                            rocblas_int    incy,
                                            const double*  param)
try
{
    return rocblas_rotm_impl(handle, n, x, incx, y, incy, param);
}
catch(...)
{
    return exception_to_rocblas_status();
}

} // extern "C"
