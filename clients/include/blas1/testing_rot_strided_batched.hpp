/* ************************************************************************
 * Copyright 2018-2021 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once

#include "cblas_interface.hpp"
#include "norm.hpp"
#include "rocblas.hpp"
#include "rocblas_init.hpp"
#include "rocblas_math.hpp"
#include "rocblas_random.hpp"
#include "rocblas_test.hpp"
#include "rocblas_vector.hpp"
#include "unit.hpp"
#include "utility.hpp"

template <typename T, typename U = T, typename V = T>
void testing_rot_strided_batched_bad_arg(const Arguments& arg)
{
    auto rocblas_rot_strided_batched_fn = arg.fortran ? rocblas_rot_strided_batched<T, U, V, true>
                                                      : rocblas_rot_strided_batched<T, U, V, false>;

    rocblas_int         N           = 100;
    rocblas_int         incx        = 1;
    rocblas_stride      stride_x    = 1;
    rocblas_int         incy        = 1;
    rocblas_stride      stride_y    = 1;
    rocblas_int         batch_count = 5;
    static const size_t safe_size   = 100;

    rocblas_local_handle handle{arg};
    device_vector<T>     dx(safe_size);
    device_vector<T>     dy(safe_size);
    device_vector<U>     dc(1);
    device_vector<V>     ds(1);
    CHECK_DEVICE_ALLOCATION(dx.memcheck());
    CHECK_DEVICE_ALLOCATION(dy.memcheck());
    CHECK_DEVICE_ALLOCATION(dc.memcheck());
    CHECK_DEVICE_ALLOCATION(ds.memcheck());

    EXPECT_ROCBLAS_STATUS(
        (rocblas_rot_strided_batched_fn(
            nullptr, N, dx, incx, stride_x, dy, incy, stride_y, dc, ds, batch_count)),
        rocblas_status_invalid_handle);
    EXPECT_ROCBLAS_STATUS(
        (rocblas_rot_strided_batched_fn(
            handle, N, nullptr, incx, stride_x, dy, incy, stride_y, dc, ds, batch_count)),
        rocblas_status_invalid_pointer);
    EXPECT_ROCBLAS_STATUS(
        (rocblas_rot_strided_batched_fn(
            handle, N, dx, incx, stride_x, nullptr, incy, stride_y, dc, ds, batch_count)),
        rocblas_status_invalid_pointer);
    EXPECT_ROCBLAS_STATUS(
        (rocblas_rot_strided_batched_fn(
            handle, N, dx, incx, stride_x, dy, incy, stride_y, nullptr, ds, batch_count)),
        rocblas_status_invalid_pointer);
    EXPECT_ROCBLAS_STATUS(
        (rocblas_rot_strided_batched_fn(
            handle, N, dx, incx, stride_x, dy, incy, stride_y, dc, nullptr, batch_count)),
        rocblas_status_invalid_pointer);
}

template <typename T, typename U = T, typename V = T>
void testing_rot_strided_batched(const Arguments& arg)
{
    auto rocblas_rot_strided_batched_fn = arg.fortran ? rocblas_rot_strided_batched<T, U, V, true>
                                                      : rocblas_rot_strided_batched<T, U, V, false>;

    rocblas_int N           = arg.N;
    rocblas_int incx        = arg.incx;
    rocblas_int stride_x    = arg.stride_x;
    rocblas_int stride_y    = arg.stride_y;
    rocblas_int incy        = arg.incy;
    rocblas_int batch_count = arg.batch_count;

    rocblas_local_handle handle{arg};
    double               gpu_time_used, cpu_time_used;
    double norm_error_host_x = 0.0, norm_error_host_y = 0.0, norm_error_device_x = 0.0,
           norm_error_device_y = 0.0;

    // check to prevent undefined memory allocation error
    if(N <= 0 || batch_count <= 0)
    {
        CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_device));
        EXPECT_ROCBLAS_STATUS((rocblas_rot_strided_batched_fn)(handle,
                                                               N,
                                                               nullptr,
                                                               incx,
                                                               stride_x,
                                                               nullptr,
                                                               incy,
                                                               stride_y,
                                                               nullptr,
                                                               nullptr,
                                                               batch_count),
                              rocblas_status_success);
        return;
    }

    rocblas_int abs_incx = incx >= 0 ? incx : -incx;
    rocblas_int abs_incy = incy >= 0 ? incy : -incy;
    size_t      size_x   = N * size_t(abs_incx) + size_t(stride_x) * size_t(batch_count - 1);
    size_t      size_y   = N * size_t(abs_incy) + size_t(stride_y) * size_t(batch_count - 1);

    device_vector<T> dx(size_x);
    device_vector<T> dy(size_y);
    device_vector<U> dc(1);
    device_vector<V> ds(1);
    CHECK_DEVICE_ALLOCATION(dx.memcheck());
    CHECK_DEVICE_ALLOCATION(dy.memcheck());
    CHECK_DEVICE_ALLOCATION(dc.memcheck());
    CHECK_DEVICE_ALLOCATION(ds.memcheck());

    // Initial Data on CPU
    host_vector<T> hx(size_x);
    host_vector<T> hy(size_y);
    host_vector<U> hc(1);
    host_vector<V> hs(1);
    rocblas_seedrand();

    if(rocblas_isnan(arg.alpha))
    {
        rocblas_init_nan<T>(hx, 1, N, abs_incx, stride_x, batch_count);
        rocblas_init_nan<T>(hy, 1, N, abs_incy, stride_y, batch_count);

        rocblas_init_nan<U>(hc, 1, 1, 1);
        rocblas_init_nan<V>(hs, 1, 1, 1);
    }
    else
    {
        rocblas_init<T>(hx, 1, N, abs_incx, stride_x, batch_count);
        rocblas_init<T>(hy, 1, N, abs_incy, stride_y, batch_count);

        rocblas_init<U>(hc, 1, 1, 1);
        rocblas_init<V>(hs, 1, 1, 1);
    }

    // CPU BLAS reference data
    host_vector<T> cx = hx;
    host_vector<T> cy = hy;
    // cblas_rotg<T, U>(cx, cy, hc, hs);
    // cx[0] = hx[0];
    // cy[0] = hy[0];
    cpu_time_used = get_time_us_no_sync();
    for(int b = 0; b < batch_count; b++)
    {
        cblas_rot<T, T, U, V>(N, cx + b * stride_x, incx, cy + b * stride_y, incy, hc, hs);
    }
    cpu_time_used = get_time_us_no_sync() - cpu_time_used;

    if(arg.unit_check || arg.norm_check)
    {
        // Test rocblas_pointer_mode_host
        {
            CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_host));
            CHECK_HIP_ERROR(hipMemcpy(dx, hx, sizeof(T) * size_x, hipMemcpyHostToDevice));
            CHECK_HIP_ERROR(hipMemcpy(dy, hy, sizeof(T) * size_y, hipMemcpyHostToDevice));
            CHECK_ROCBLAS_ERROR((rocblas_rot_strided_batched_fn(
                handle, N, dx, incx, stride_x, dy, incy, stride_y, hc, hs, batch_count)));
            host_vector<T> rx(size_x);
            host_vector<T> ry(size_y);
            CHECK_HIP_ERROR(hipMemcpy(rx, dx, sizeof(T) * size_x, hipMemcpyDeviceToHost));
            CHECK_HIP_ERROR(hipMemcpy(ry, dy, sizeof(T) * size_y, hipMemcpyDeviceToHost));
            if(arg.unit_check)
            {
                unit_check_general<T>(1, N, abs_incx, stride_x, cx, rx, batch_count);
                unit_check_general<T>(1, N, abs_incy, stride_y, cy, ry, batch_count);
            }
            if(arg.norm_check)
            {
                norm_error_host_x
                    = norm_check_general<T>('F', 1, N, abs_incx, stride_x, cx, rx, batch_count);
                norm_error_host_y
                    = norm_check_general<T>('F', 1, N, abs_incy, stride_x, cy, ry, batch_count);
            }
        }

        // Test rocblas_pointer_mode_device
        {
            CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_device));
            CHECK_HIP_ERROR(hipMemcpy(dx, hx, sizeof(T) * size_x, hipMemcpyHostToDevice));
            CHECK_HIP_ERROR(hipMemcpy(dy, hy, sizeof(T) * size_y, hipMemcpyHostToDevice));
            CHECK_HIP_ERROR(hipMemcpy(dc, hc, sizeof(U), hipMemcpyHostToDevice));
            CHECK_HIP_ERROR(hipMemcpy(ds, hs, sizeof(V), hipMemcpyHostToDevice));
            CHECK_ROCBLAS_ERROR((rocblas_rot_strided_batched_fn(
                handle, N, dx, incx, stride_x, dy, incy, stride_y, dc, ds, batch_count)));
            host_vector<T> rx(size_x);
            host_vector<T> ry(size_y);
            CHECK_HIP_ERROR(hipMemcpy(rx, dx, sizeof(T) * size_x, hipMemcpyDeviceToHost));
            CHECK_HIP_ERROR(hipMemcpy(ry, dy, sizeof(T) * size_y, hipMemcpyDeviceToHost));
            if(arg.unit_check)
            {
                unit_check_general<T>(1, N, abs_incx, stride_x, cx, rx, batch_count);
                unit_check_general<T>(1, N, abs_incy, stride_y, cy, ry, batch_count);
            }
            if(arg.norm_check)
            {
                norm_error_device_x
                    = norm_check_general<T>('F', 1, N, abs_incx, stride_x, cx, rx, batch_count);
                norm_error_device_y
                    = norm_check_general<T>('F', 1, N, abs_incy, stride_y, cy, ry, batch_count);
            }
        }
    }

    if(arg.timing)
    {
        int number_cold_calls = arg.cold_iters;
        int number_hot_calls  = arg.iters;
        CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_device));
        CHECK_HIP_ERROR(hipMemcpy(dx, hx, sizeof(T) * size_x, hipMemcpyHostToDevice));
        CHECK_HIP_ERROR(hipMemcpy(dy, hy, sizeof(T) * size_y, hipMemcpyHostToDevice));
        CHECK_HIP_ERROR(hipMemcpy(dc, hc, sizeof(U), hipMemcpyHostToDevice));
        CHECK_HIP_ERROR(hipMemcpy(ds, hs, sizeof(V), hipMemcpyHostToDevice));

        for(int iter = 0; iter < number_cold_calls; iter++)
        {
            rocblas_rot_strided_batched_fn(
                handle, N, dx, incx, stride_x, dy, incy, stride_y, dc, ds, batch_count);
        }
        hipStream_t stream;
        CHECK_ROCBLAS_ERROR(rocblas_get_stream(handle, &stream));
        gpu_time_used = get_time_us_sync(stream); // in microseconds
        for(int iter = 0; iter < number_hot_calls; iter++)
        {
            rocblas_rot_strided_batched_fn(
                handle, N, dx, incx, stride_x, dy, incy, stride_y, dc, ds, batch_count);
        }
        gpu_time_used = (get_time_us_sync(stream) - gpu_time_used);

        ArgumentModel<e_N, e_incx, e_incy, e_stride_x, e_stride_y, e_batch_count>{}.log_args<T>(
            rocblas_cout,
            arg,
            gpu_time_used,
            rot_gflop_count<T, T, U, V>(N),
            rot_gbyte_count<T>(N),
            cpu_time_used,
            norm_error_host_x,
            norm_error_device_x,
            norm_error_host_y,
            norm_error_device_y);
    }
}
