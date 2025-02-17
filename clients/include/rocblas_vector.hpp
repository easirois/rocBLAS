/* ************************************************************************
 * Copyright 2018-2020 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once

#include "d_vector.hpp"

#include "device_batch_vector.hpp"
#include "device_strided_batch_vector.hpp"
#include "device_vector.hpp"

#include "host_batch_vector.hpp"
#include "host_pinned_vector.hpp"
#include "host_strided_batch_vector.hpp"
#include "host_vector.hpp"

#include "rocblas_init.hpp"

//!
//! @brief Template for initializing a host (non_batched|batched|strided_batched)vector.
//! @param that That vector.
//! @param rand_gen The random number generator
//! @param seedReset Reset the seed if true, do not reset the seed otherwise.
//!
template <typename U, typename T>
void rocblas_init_template(U& that, T rand_gen(), bool seedReset)
{
    if(seedReset)
        rocblas_seedrand();

    for(rocblas_int batch_index = 0; batch_index < that.batch_count(); ++batch_index)
    {
        auto*     batched_data = that[batch_index];
        ptrdiff_t inc          = that.inc();
        auto      n            = that.n();
        if(inc < 0)
            batched_data -= (n - 1) * inc;

        for(rocblas_int i = 0; i < n; ++i)
            batched_data[i * inc] = rand_gen();
    }
}

//!
//! @brief Initialize a host_strided_batch_vector.
//! @param that The host strided batch vector.
//! @param seedReset reset the seed if true, do not reset the seed otherwise.
//!
template <typename T>
inline void rocblas_init(host_strided_batch_vector<T>& that, bool seedReset = false)
{
    rocblas_init_template(that, random_generator<T>, seedReset);
}

//!
//! @brief Initialize a host_batch_vector.
//! @param that The host batch vector.
//! @param seedReset reset the seed if true, do not reset the seed otherwise.
//!
template <typename T>
inline void rocblas_init(host_batch_vector<T>& that, bool seedReset = false)
{
    rocblas_init_template(that, random_generator<T>, seedReset);
}

//!
//! @brief Initialize a host_vector.
//! @param that The host vector.
//! @param seedReset reset the seed if true, do not reset the seed otherwise.
//!
template <typename T>
inline void rocblas_init(host_vector<T>& that, bool seedReset = false)
{
    if(seedReset)
        rocblas_seedrand();
    rocblas_init(that, 1, that.size(), 1);
}

//!
//! @brief Initialize a host_strided_batch_vector with NaNs.
//! @param that The host strided batch vector to be initialized.
//! @param seedReset reset the seed if true, do not reset the seed otherwise.
//!
template <typename T>
inline void rocblas_init_nan(host_strided_batch_vector<T>& that, bool seedReset = false)
{
    rocblas_init_template(that, random_nan_generator<T>, seedReset);
}

//!
//! @brief Initialize a host_strided_batch_vector with NaNs.
//! @param that The host strided batch vector to be initialized.
//! @param seedReset reset the seed if true, do not reset the seed otherwise.
//!
template <typename T>
inline void rocblas_init_nan(host_batch_vector<T>& that, bool seedReset = false)
{
    rocblas_init_template(that, random_nan_generator<T>, seedReset);
}

//!
//! @brief Initialize a host_strided_batch_vector with NaNs.
//! @param that The host strided batch vector to be initialized.
//! @param seedReset reset he seed if true, do not reset the seed otherwise.
//!
template <typename T>
inline void rocblas_init_nan(host_vector<T>& that, bool seedReset = false)
{
    rocblas_init_template(that, random_nan_generator<T>, seedReset);
}
