/* ************************************************************************
 * Copyright 2018-2020 Advanced Micro Devices, Inc.


 *
 * ************************************************************************ */

#pragma once

#include "bytes.hpp"
#include "cblas_interface.hpp"
#include "flops.hpp"
#include "near.hpp"
#include "norm.hpp"
#include "rocblas.hpp"
#include "rocblas_datatype2string.hpp"
#include "rocblas_init.hpp"
#include "rocblas_math.hpp"
#include "rocblas_random.hpp"
#include "rocblas_test.hpp"
#include "rocblas_vector.hpp"
#include "unit.hpp"
#include "utility.hpp"

template <typename T>
void testing_trmv_batched_bad_arg(const Arguments& arg)
{
    auto rocblas_trmv_batched_fn
        = arg.fortran ? rocblas_trmv_batched<T, true> : rocblas_trmv_batched<T, false>;

    const rocblas_int       M           = 100;
    const rocblas_int       lda         = 100;
    const rocblas_int       incx        = 1;
    const rocblas_int       batch_count = 1;
    const rocblas_operation transA      = rocblas_operation_none;
    const rocblas_fill      uplo        = rocblas_fill_lower;
    const rocblas_diagonal  diag        = rocblas_diagonal_non_unit;

    rocblas_local_handle handle{arg};

    size_t size_A = lda * size_t(M);

    host_batch_vector<T> hA(size_A, 1, batch_count);
    CHECK_HIP_ERROR(hA.memcheck());
    host_batch_vector<T> hx(M, incx, batch_count);
    CHECK_HIP_ERROR(hx.memcheck());

    device_batch_vector<T> dA(batch_count, M * lda);
    CHECK_DEVICE_ALLOCATION(dA.memcheck());
    device_batch_vector<T> dx(M, incx, batch_count);
    CHECK_DEVICE_ALLOCATION(dx.memcheck());

    //
    // Checks.
    //
    EXPECT_ROCBLAS_STATUS(
        rocblas_trmv_batched_fn(
            handle, uplo, transA, diag, M, nullptr, lda, dx.ptr_on_device(), incx, batch_count),
        rocblas_status_invalid_pointer);

    EXPECT_ROCBLAS_STATUS(
        rocblas_trmv_batched_fn(
            handle, uplo, transA, diag, M, dA.ptr_on_device(), lda, nullptr, incx, batch_count),
        rocblas_status_invalid_pointer);

    EXPECT_ROCBLAS_STATUS(rocblas_trmv_batched_fn(nullptr,
                                                  uplo,
                                                  transA,
                                                  diag,
                                                  M,
                                                  dA.ptr_on_device(),
                                                  lda,
                                                  dx.ptr_on_device(),
                                                  incx,
                                                  batch_count),
                          rocblas_status_invalid_handle);
}

template <typename T>
void testing_trmv_batched(const Arguments& arg)
{
    auto rocblas_trmv_batched_fn
        = arg.fortran ? rocblas_trmv_batched<T, true> : rocblas_trmv_batched<T, false>;

    rocblas_int M = arg.M, lda = arg.lda, incx = arg.incx, batch_count = arg.batch_count;

    char char_uplo = arg.uplo, char_transA = arg.transA, char_diag = arg.diag;

    rocblas_fill      uplo   = char2rocblas_fill(char_uplo);
    rocblas_operation transA = char2rocblas_operation(char_transA);
    rocblas_diagonal  diag   = char2rocblas_diagonal(char_diag);

    rocblas_local_handle handle{arg};

    bool invalid_size = M < 0 || lda < M || lda < 1 || !incx || batch_count < 0;
    if(invalid_size || !M || !batch_count)
    {
        EXPECT_ROCBLAS_STATUS(
            rocblas_trmv_batched_fn(
                handle, uplo, transA, diag, M, nullptr, lda, nullptr, incx, batch_count),
            invalid_size ? rocblas_status_invalid_size : rocblas_status_success);

        return;
    }

    size_t size_A   = lda * size_t(M);
    size_t abs_incx = incx >= 0 ? incx : -incx;

    host_batch_vector<T> hA(size_A, 1, batch_count);
    CHECK_HIP_ERROR(hA.memcheck());

    host_batch_vector<T> hx(M, incx, batch_count);
    CHECK_HIP_ERROR(hx.memcheck());

    host_batch_vector<T> hres(M, incx, batch_count);
    CHECK_HIP_ERROR(hres.memcheck());

    device_batch_vector<T> dA(batch_count, size_A);
    CHECK_DEVICE_ALLOCATION(dA.memcheck());

    device_batch_vector<T> dx(M, incx, batch_count);
    CHECK_DEVICE_ALLOCATION(dx.memcheck());

    auto dA_on_device = dA.ptr_on_device();
    auto dx_on_device = dx.ptr_on_device();

    //
    // Initialize.
    //
    rocblas_init(hA, true);
    rocblas_init(hx);

    //
    // Transfer.
    //
    CHECK_HIP_ERROR(dA.transfer_from(hA));
    CHECK_HIP_ERROR(dx.transfer_from(hx));

    double gpu_time_used, cpu_time_used, rocblas_error;

    /* =====================================================================
     ROCBLAS
     =================================================================== */
    if(arg.unit_check || arg.norm_check)
    {
        //
        // GPU BLAS
        //
        CHECK_ROCBLAS_ERROR(rocblas_trmv_batched_fn(
            handle, uplo, transA, diag, M, dA_on_device, lda, dx_on_device, incx, batch_count));

        //
        // CPU BLAS
        //
        {
            cpu_time_used = get_time_us_no_sync();
            for(rocblas_int batch_index = 0; batch_index < batch_count; ++batch_index)
            {
                cblas_trmv<T>(uplo, transA, diag, M, hA[batch_index], lda, hx[batch_index], incx);
            }
            cpu_time_used = get_time_us_no_sync() - cpu_time_used;
        }

        // fetch GPU
        CHECK_HIP_ERROR(hres.transfer_from(dx));
        //
        // Unit check.
        //
        if(arg.unit_check)
        {
            unit_check_general<T>(1, M, abs_incx, hx, hres, batch_count);
        }

        //
        // Norm check.
        //
        if(arg.norm_check)
        {
            rocblas_error = norm_check_general<T>('F', 1, M, abs_incx, hx, hres, batch_count);
        }
    }

    if(arg.timing)
    {

        //
        // Warmup
        //
        {
            int number_cold_calls = arg.cold_iters;
            for(int iter = 0; iter < number_cold_calls; iter++)
            {
                rocblas_trmv_batched_fn(handle,
                                        uplo,
                                        transA,
                                        diag,
                                        M,
                                        dA_on_device,
                                        lda,
                                        dx_on_device,
                                        incx,
                                        batch_count);
            }
        }

        //
        // Go !
        //
        {
            hipStream_t stream;
            CHECK_ROCBLAS_ERROR(rocblas_get_stream(handle, &stream));
            gpu_time_used        = get_time_us_sync(stream); // in microseconds
            int number_hot_calls = arg.iters;
            for(int iter = 0; iter < number_hot_calls; iter++)
            {
                rocblas_trmv_batched_fn(handle,
                                        uplo,
                                        transA,
                                        diag,
                                        M,
                                        dA_on_device,
                                        lda,
                                        dx_on_device,
                                        incx,
                                        batch_count);
            }
            gpu_time_used = get_time_us_sync(stream) - gpu_time_used;
        }

        //
        // Log performance
        //
        ArgumentModel<e_uplo, e_transA, e_diag, e_M, e_lda, e_incx, e_batch_count>{}.log_args<T>(
            rocblas_cout,
            arg,
            gpu_time_used,
            trmv_gflop_count<T>(M),
            trmv_gbyte_count<T>(M),
            cpu_time_used,
            rocblas_error);
    }
}
