/* ************************************************************************
 * Copyright 2018-2020 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once

#include "bytes.hpp"
#include "cblas_interface.hpp"
#include "flops.hpp"
#include "near.hpp"
#include "norm.hpp"
#include "rocblas.hpp"
#include "rocblas_init.hpp"
#include "rocblas_math.hpp"
#include "rocblas_random.hpp"
#include "rocblas_test.hpp"
#include "rocblas_vector.hpp"
#include "unit.hpp"
#include "utility.hpp"

template <typename T>
void testing_hpr_strided_batched_bad_arg(const Arguments& arg)
{
    auto rocblas_hpr_strided_batched_fn = arg.fortran ? rocblas_hpr_strided_batched<T, true>
                                                      : rocblas_hpr_strided_batched<T, false>;

    rocblas_fill   uplo        = rocblas_fill_upper;
    rocblas_int    N           = 10;
    rocblas_int    incx        = 1;
    real_t<T>      alpha       = 0.6;
    rocblas_int    batch_count = 5;
    rocblas_stride stride_x    = 100;
    rocblas_stride stride_A    = 100;

    rocblas_local_handle handle{arg};

    size_t size_A = size_t(N) * (N + 1) / 2;

    // allocate memory on device
    device_strided_batch_vector<T> dA_1(size_A, 1, stride_A, batch_count);
    device_strided_batch_vector<T> dx(N, incx, stride_x, batch_count);
    CHECK_DEVICE_ALLOCATION(dA_1.memcheck());
    CHECK_DEVICE_ALLOCATION(dx.memcheck());

    EXPECT_ROCBLAS_STATUS(
        rocblas_hpr_strided_batched_fn(
            handle, rocblas_fill_full, N, &alpha, dx, incx, stride_x, dA_1, stride_A, batch_count),
        rocblas_status_invalid_value);

    EXPECT_ROCBLAS_STATUS(
        rocblas_hpr_strided_batched_fn(
            handle, uplo, N, &alpha, nullptr, incx, stride_x, dA_1, stride_A, batch_count),
        rocblas_status_invalid_pointer);

    EXPECT_ROCBLAS_STATUS(
        rocblas_hpr_strided_batched_fn(
            handle, uplo, N, &alpha, dx, incx, stride_x, nullptr, stride_A, batch_count),
        rocblas_status_invalid_pointer);

    EXPECT_ROCBLAS_STATUS(
        rocblas_hpr_strided_batched_fn(
            nullptr, uplo, N, &alpha, dx, incx, stride_x, dA_1, stride_A, batch_count),
        rocblas_status_invalid_handle);
}

template <typename T>
void testing_hpr_strided_batched(const Arguments& arg)
{
    auto rocblas_hpr_strided_batched_fn = arg.fortran ? rocblas_hpr_strided_batched<T, true>
                                                      : rocblas_hpr_strided_batched<T, false>;

    rocblas_int    N           = arg.N;
    rocblas_int    incx        = arg.incx;
    real_t<T>      h_alpha     = arg.get_alpha<real_t<T>>();
    rocblas_fill   uplo        = char2rocblas_fill(arg.uplo);
    rocblas_stride stride_x    = arg.stride_x;
    rocblas_stride stride_A    = arg.stride_a;
    rocblas_int    batch_count = arg.batch_count;

    rocblas_local_handle handle{arg};

    // argument check before allocating invalid memory
    bool invalid_size = N < 0 || !incx || batch_count < 0;
    if(invalid_size || !N || !batch_count)
    {
        EXPECT_ROCBLAS_STATUS(
            rocblas_hpr_strided_batched_fn(
                handle, uplo, N, nullptr, nullptr, incx, stride_x, nullptr, stride_A, batch_count),
            invalid_size ? rocblas_status_invalid_size : rocblas_status_success);
        return;
    }

    size_t abs_incx = incx >= 0 ? incx : -incx;
    size_t size_A   = size_t(N) * (N + 1) / 2;

    // Naming: dK is in GPU (device) memory. hK is in CPU (host) memory
    host_strided_batch_vector<T> hA_1(size_A, 1, stride_A, batch_count);
    host_strided_batch_vector<T> hA_2(size_A, 1, stride_A, batch_count);
    host_strided_batch_vector<T> hA_gold(size_A, 1, stride_A, batch_count);
    host_strided_batch_vector<T> hx(N, incx, stride_x, batch_count);
    host_vector<real_t<T>>       halpha(1);
    CHECK_HIP_ERROR(hA_1.memcheck());
    CHECK_HIP_ERROR(hA_2.memcheck());
    CHECK_HIP_ERROR(hA_gold.memcheck());
    CHECK_HIP_ERROR(hx.memcheck());
    CHECK_HIP_ERROR(halpha.memcheck());

    halpha[0] = h_alpha;

    // allocate memory on device
    device_strided_batch_vector<T> dA_1(size_A, 1, stride_A, batch_count);
    device_strided_batch_vector<T> dA_2(size_A, 1, stride_A, batch_count);
    device_strided_batch_vector<T> dx(N, incx, stride_x, batch_count);
    device_vector<real_t<T>>       d_alpha(1);
    CHECK_DEVICE_ALLOCATION(dA_1.memcheck());
    CHECK_DEVICE_ALLOCATION(dA_2.memcheck());
    CHECK_DEVICE_ALLOCATION(dx.memcheck());
    CHECK_DEVICE_ALLOCATION(d_alpha.memcheck());

    double gpu_time_used, cpu_time_used;
    double rocblas_error_1;
    double rocblas_error_2;

    // Initial Data on CPU
    rocblas_init<T>(hA_1, true);
    rocblas_init<T>(hx, false);

    hA_2.copy_from(hA_1);
    hA_gold.copy_from(hA_1);

    // copy data from CPU to device
    CHECK_HIP_ERROR(dA_1.transfer_from(hA_1));
    CHECK_HIP_ERROR(dA_2.transfer_from(hA_1));
    CHECK_HIP_ERROR(dx.transfer_from(hx));
    CHECK_HIP_ERROR(d_alpha.transfer_from(halpha));

    if(arg.unit_check || arg.norm_check)
    {
        CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_host));
        CHECK_ROCBLAS_ERROR(rocblas_hpr_strided_batched_fn(
            handle, uplo, N, &h_alpha, dx, incx, stride_x, dA_1, stride_A, batch_count));

        CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_device));
        CHECK_ROCBLAS_ERROR(rocblas_hpr_strided_batched_fn(
            handle, uplo, N, d_alpha, dx, incx, stride_x, dA_2, stride_A, batch_count));

        // CPU BLAS
        cpu_time_used = get_time_us_no_sync();
        for(int i = 0; i < batch_count; i++)
        {
            cblas_hpr<T>(uplo, N, h_alpha, hx[i], incx, hA_gold[i]);
        }
        cpu_time_used = get_time_us_no_sync() - cpu_time_used;

        // copy output from device to CPU
        CHECK_HIP_ERROR(hA_1.transfer_from(dA_1));
        CHECK_HIP_ERROR(hA_2.transfer_from(dA_2));

        if(arg.unit_check)
        {
            const double tol = N * sum_error_tolerance<T>;
            near_check_general<T>(1, size_A, 1, stride_A, hA_gold, hA_1, batch_count, tol);
            near_check_general<T>(1, size_A, 1, stride_A, hA_gold, hA_2, batch_count, tol);
        }

        if(arg.norm_check)
        {
            rocblas_error_1
                = norm_check_general<T>('F', 1, size_A, 1, stride_A, hA_gold, hA_1, batch_count);
            rocblas_error_2
                = norm_check_general<T>('F', 1, size_A, 1, stride_A, hA_gold, hA_2, batch_count);
        }
    }

    if(arg.timing)
    {
        int number_cold_calls = arg.cold_iters;
        int number_hot_calls  = arg.iters;
        CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_host));

        for(int iter = 0; iter < number_cold_calls; iter++)
        {
            rocblas_hpr_strided_batched_fn(
                handle, uplo, N, &h_alpha, dx, incx, stride_x, dA_1, stride_A, batch_count);
        }

        hipStream_t stream;
        CHECK_ROCBLAS_ERROR(rocblas_get_stream(handle, &stream));
        gpu_time_used = get_time_us_sync(stream); // in microseconds

        for(int iter = 0; iter < number_hot_calls; iter++)
        {
            rocblas_hpr_strided_batched_fn(
                handle, uplo, N, &h_alpha, dx, incx, stride_x, dA_1, stride_A, batch_count);
        }

        gpu_time_used = get_time_us_sync(stream) - gpu_time_used;

        ArgumentModel<e_uplo, e_N, e_alpha, e_stride_a, e_incx, e_stride_x, e_batch_count>{}
            .log_args<T>(rocblas_cout,
                         arg,
                         gpu_time_used,
                         hpr_gflop_count<T>(N),
                         hpr_gbyte_count<T>(N),
                         cpu_time_used,
                         rocblas_error_1,
                         rocblas_error_2);
    }
}
