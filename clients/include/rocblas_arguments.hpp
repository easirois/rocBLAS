/* ************************************************************************
 * Copyright 2018-2021 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once

#include "../../library/src/include/rocblas_ostream.hpp"
#include "rocblas.h"
#include "rocblas_datatype2string.hpp"
#include "rocblas_math.hpp"
#include <cstddef>
#include <istream>
#include <map>
#include <ostream>
#include <tuple>

// Predeclare enumerator
enum rocblas_argument : int;

/***************************************************************************
 *! \brief Class used to parse command arguments in both client & gtest    *
 * WARNING: If this data is changed, then rocblas_common.yaml must also be *
 * changed.                                                                *
 ***************************************************************************/
struct Arguments
{
    /*************************************************************************
     *                    Beginning Of Arguments                             *
     *************************************************************************/

    char function[64];
    char name[64];
    char category[64];
    char known_bug_platforms[64];

    // 64bit

    double alpha;
    double alphai;
    double beta;
    double betai;

    rocblas_stride stride_a; //  stride_a > transA == 'N' ? lda * K : lda * M
    rocblas_stride stride_b; //  stride_b > transB == 'N' ? ldb * N : ldb * K
    rocblas_stride stride_c; //  stride_c > ldc * N
    rocblas_stride stride_d; //  stride_d > ldd * N
    rocblas_stride stride_x;
    rocblas_stride stride_y;

    size_t user_allocated_workspace;

    // 32bit

    rocblas_int M;
    rocblas_int N;
    rocblas_int K;

    rocblas_int KL;
    rocblas_int KU;

    rocblas_int lda;
    rocblas_int ldb;
    rocblas_int ldc;
    rocblas_int ldd;

    rocblas_int incx;
    rocblas_int incy;
    rocblas_int incd;
    rocblas_int incb;

    rocblas_int batch_count;

    rocblas_int iters;
    rocblas_int cold_iters;

    uint32_t algo;
    int32_t  solution_index;

    rocblas_gemm_flags flags;

    rocblas_datatype a_type;
    rocblas_datatype b_type;
    rocblas_datatype c_type;
    rocblas_datatype d_type;
    rocblas_datatype compute_type;

    rocblas_initialization initialization;

    rocblas_atomics_mode atomics_mode;

    // 16 bit

    uint16_t threads;
    uint16_t streams;

    // bytes
    uint8_t devices;

    int8_t norm_check;
    int8_t unit_check;
    int8_t timing;

    char transA;
    char transB;
    char side;
    char uplo;
    char diag;

    bool c_noalias_d;
    bool HMM;
    bool fortran;

    /*************************************************************************
     *                     End Of Arguments                                  *
     *************************************************************************/

    // clang-format off

// Generic macro which operates over the list of arguments in order of declaration
#define FOR_EACH_ARGUMENT(OPER, SEP) \
    OPER(function) SEP               \
    OPER(name) SEP                   \
    OPER(category) SEP               \
    OPER(known_bug_platforms) SEP    \
    OPER(alpha) SEP                  \
    OPER(alphai) SEP                 \
    OPER(beta) SEP                   \
    OPER(betai) SEP                  \
    OPER(stride_a) SEP               \
    OPER(stride_b) SEP               \
    OPER(stride_c) SEP               \
    OPER(stride_d) SEP               \
    OPER(stride_x) SEP               \
    OPER(stride_y) SEP               \
    OPER(user_allocated_workspace) SEP \
    OPER(M) SEP                      \
    OPER(N) SEP                      \
    OPER(K) SEP                      \
    OPER(KL) SEP                     \
    OPER(KU) SEP                     \
    OPER(lda) SEP                    \
    OPER(ldb) SEP                    \
    OPER(ldc) SEP                    \
    OPER(ldd) SEP                    \
    OPER(incx) SEP                   \
    OPER(incy) SEP                   \
    OPER(incd) SEP                   \
    OPER(incb) SEP                   \
    OPER(batch_count) SEP            \
    OPER(iters) SEP                  \
    OPER(cold_iters) SEP             \
    OPER(algo) SEP                   \
    OPER(solution_index) SEP         \
    OPER(flags) SEP                  \
    OPER(a_type) SEP                 \
    OPER(b_type) SEP                 \
    OPER(c_type) SEP                 \
    OPER(d_type) SEP                 \
    OPER(compute_type) SEP           \
    OPER(initialization) SEP         \
    OPER(atomics_mode) SEP           \
    OPER(threads) SEP                \
    OPER(streams) SEP                \
    OPER(devices) SEP                \
    OPER(norm_check) SEP             \
    OPER(unit_check) SEP             \
    OPER(timing) SEP                 \
    OPER(transA) SEP                 \
    OPER(transB) SEP                 \
    OPER(side) SEP                   \
    OPER(uplo) SEP                   \
    OPER(diag) SEP                   \
    OPER(c_noalias_d) SEP            \
    OPER(HMM) SEP                    \
    OPER(fortran)

    // clang-format on

    // Validate input format.
    static void validate(std::istream& ifs);

    // Function to print Arguments out to stream in YAML format
    friend rocblas_internal_ostream& operator<<(rocblas_internal_ostream& str,
                                                const Arguments&          arg);

    // Google Tests uses this with std:ostream automatically to dump parameters
    friend std::ostream& operator<<(std::ostream& str, const Arguments& arg);

    // Function to read Arguments data from stream
    friend std::istream& operator>>(std::istream& str, Arguments& arg);

#ifdef WIN32
    // Clang specific code
    template <typename T>
    friend rocblas_internal_ostream& operator<<(rocblas_internal_ostream& os,
                                                std::pair<char const*, T> p);

    friend rocblas_internal_ostream& operator<<(rocblas_internal_ostream&                os,
                                                std::pair<char const*, rocblas_datatype> p);

    friend rocblas_internal_ostream& operator<<(rocblas_internal_ostream&                      os,
                                                std::pair<char const*, rocblas_initialization> p);

    friend rocblas_internal_ostream& operator<<(rocblas_internal_ostream&    os,
                                                std::pair<char const*, bool> p);
// End of Clang specific code
#endif

    // Convert (alpha, alphai) and (beta, betai) to a particular type
    // Return alpha, beta adjusted to 0 for when they are NaN
    template <typename T>
    T get_alpha() const
    {
        return alpha_isnan<T>() ? T(0) : convert_alpha_beta<T>(alpha, alphai);
    }

    template <typename T>
    T get_beta() const
    {
        return beta_isnan<T>() ? T(0) : convert_alpha_beta<T>(beta, betai);
    }

    template <typename T>
    bool alpha_isnan() const
    {
        return rocblas_isnan(alpha) || (is_complex<T> && rocblas_isnan(alphai));
    }

    template <typename T>
    bool beta_isnan() const
    {
        return rocblas_isnan(beta) || (is_complex<T> && rocblas_isnan(betai));
    }

private:
    template <typename T, typename U, std::enable_if_t<!is_complex<T>, int> = 0>
    static T convert_alpha_beta(U r, U i)
    {
        return T(r);
    }

    template <typename T, typename U, std::enable_if_t<+is_complex<T>, int> = 0>
    static T convert_alpha_beta(U r, U i)
    {
        return T(r, i);
    }
};

// We make sure that the Arguments struct is C-compatible
static_assert(std::is_standard_layout<Arguments>{},
              "Arguments is not a standard layout type, and thus is "
              "incompatible with C.");

static_assert(std::is_trivial<Arguments>{},
              "Arguments is not a trivial type, and thus is "
              "incompatible with C.");

// Arguments enumerators
// Create
//     enum rocblas_argument : int {e_M, e_N, e_K, e_KL, ... };
// There is an enum value for each case in FOR_EACH_ARGUMENT.
//
#define CREATE_ENUM(NAME) e_##NAME,
enum rocblas_argument : int
{
    FOR_EACH_ARGUMENT(CREATE_ENUM, )
};
#undef CREATE_ENUM

// ArgumentsHelper contains a templated lambda apply<> where there is a template
// specialization for each line in the CPP macro FOR_EACH_ARGUMENT. For example,
// the first lambda is:  apply<e_M> = [](auto&& func, const Arguments& arg, auto){func("M", arg.m);};
// This lambda can be used to print "M" and arg.m.
//
// alpha and beta are specialized separately, because they need to use get_alpha() or get_beta().
// To prevent multiple definitions of specializations for alpha and beta, the rocblas_argument
// enum for alpha and beta are changed to rocblas_argument(-1) and rocblas_argument(-2) during
// the FOR_EACH_ARGUMENT loop. Those out-of-range enum values are not used except here, and are
// only used so that the FOR_EACH_ARGUMENT loop can be used to loop over all of the arguments.

#if __cplusplus >= 201703L
// C++17
// ArgumentsHelper contains a templated lambda apply<> where there is a template
// specialization for each line in the CPP macro FOR_EACH_ARGUMENT. For example,
// the first lambda is:  apply<e_M> = [](auto&& func, const Arguments& arg, auto){func("M", arg.m)}
// This lambda can be used to print "M" and arg.m
namespace ArgumentsHelper
{
    template <rocblas_argument>
    static constexpr auto apply = nullptr;

    // Macro defining specializations for specific arguments
    // e_alpha and e_beta get turned into negative sentinel value specializations
    // clang-format off
#define APPLY(NAME)                                                                         \
    template <>                                                                             \
    ROCBLAS_CLANG_STATIC constexpr auto                                                     \
        apply<e_##NAME == e_alpha ? rocblas_argument(-1)                                    \
                                  : e_##NAME == e_beta ? rocblas_argument(-2) : e_##NAME> = \
            [](auto&& func, const Arguments& arg, auto) { func(#NAME, arg.NAME); }

    // Specialize apply for each Argument
    FOR_EACH_ARGUMENT(APPLY, ;);

    // Specialization for e_alpha
    template <>
    ROCBLAS_CLANG_STATIC constexpr auto apply<e_alpha> =
        [](auto&& func, const Arguments& arg, auto T) {
            func("alpha", arg.get_alpha<decltype(T)>());
        };

    // Specialization for e_beta
    template <>
    ROCBLAS_CLANG_STATIC constexpr auto apply<e_beta> =
        [](auto&& func, const Arguments& arg, auto T) {
            func("beta", arg.get_beta<decltype(T)>());
        };
};
    // clang-format on

#else

// C++14. TODO: Remove when C++17 is used
// clang-format off
namespace ArgumentsHelper
{
#define APPLY(NAME)                                             \
    template <>                                                 \
    struct apply<e_##NAME == e_alpha ? rocblas_argument(-1) :   \
                 e_##NAME == e_beta  ? rocblas_argument(-2) :   \
                 e_##NAME>                                      \
    {                                                           \
        auto operator()()                                       \
        {                                                       \
            return                                              \
                [](auto&& func, const Arguments& arg, auto)     \
                {                                               \
                    func(#NAME, arg.NAME);                      \
                };                                              \
        }                                                       \
    };

    template <rocblas_argument>
    struct apply
    {
    };

    // Go through every argument and define specializations
    FOR_EACH_ARGUMENT(APPLY, ;);

    // Specialization for e_alpha
    template <>
    struct apply<e_alpha>
    {
        auto operator()()
        {
            return
                [](auto&& func, const Arguments& arg, auto T)
                {
                    func("alpha", arg.get_alpha<decltype(T)>());
                };
        }
    };

    // Specialization for e_beta
    template <>
    struct apply<e_beta>
    {
        auto operator()()
        {
            return
                [](auto&& func, const Arguments& arg, auto T)
                {
                    func("beta", arg.get_beta<decltype(T)>());
                };
        }
    };
};
// clang-format on
#endif

#undef APPLY
