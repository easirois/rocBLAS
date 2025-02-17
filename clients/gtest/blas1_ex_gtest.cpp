/* ************************************************************************
 * Copyright 2018-2021 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "rocblas_data.hpp"
#include "rocblas_datatype2string.hpp"
#include "testing_axpy_batched_ex.hpp"
#include "testing_axpy_ex.hpp"
#include "testing_axpy_strided_batched_ex.hpp"
#include "testing_dot_batched_ex.hpp"
#include "testing_dot_ex.hpp"
#include "testing_dot_strided_batched_ex.hpp"
#include "testing_nrm2_batched_ex.hpp"
#include "testing_nrm2_ex.hpp"
#include "testing_nrm2_strided_batched_ex.hpp"
#include "testing_rot_batched_ex.hpp"
#include "testing_rot_ex.hpp"
#include "testing_rot_strided_batched_ex.hpp"
#include "testing_scal_batched_ex.hpp"
#include "testing_scal_ex.hpp"
#include "testing_scal_strided_batched_ex.hpp"
#include "type_dispatch.hpp"
#include "utility.hpp"

namespace
{
    enum class blas1_ex
    {
        axpy_ex,
        axpy_batched_ex,
        axpy_strided_batched_ex,
        dot_ex,
        dotc_ex,
        dot_batched_ex,
        dotc_batched_ex,
        dot_strided_batched_ex,
        dotc_strided_batched_ex,
        nrm2_ex,
        nrm2_batched_ex,
        nrm2_strided_batched_ex,
        rot_ex,
        rot_batched_ex,
        rot_strided_batched_ex,
        scal_ex,
        scal_batched_ex,
        scal_strided_batched_ex,
    };

    // ----------------------------------------------------------------------------
    // BLAS1_ex testing template
    // ----------------------------------------------------------------------------
    template <template <typename...> class FILTER, blas1_ex BLAS1_EX>
    struct blas1_ex_test_template
        : public RocBLAS_Test<blas1_ex_test_template<FILTER, BLAS1_EX>, FILTER>
    {
        // Filter for which types apply to this suite
        static bool type_filter(const Arguments& arg)
        {
            return rocblas_blas1_ex_dispatch<blas1_ex_test_template::template type_filter_functor>(
                arg);
        }

        // Filter for which functions apply to this suite
        static bool function_filter(const Arguments& arg);

        // Google Test name suffix based on parameters
        static std::string name_suffix(const Arguments& arg)
        {
            RocBLAS_TestName<blas1_ex_test_template> name(arg.name);

            if(strstr(arg.function, "_bad_arg") != nullptr)
            {
                name << "_bad_arg";
            }
            else
            {
                bool is_axpy
                    = (BLAS1_EX == blas1_ex::axpy_ex || BLAS1_EX == blas1_ex::axpy_batched_ex
                       || BLAS1_EX == blas1_ex::axpy_strided_batched_ex);

                bool is_dot
                    = (BLAS1_EX == blas1_ex::dot_ex || BLAS1_EX == blas1_ex::dot_batched_ex
                       || BLAS1_EX == blas1_ex::dot_strided_batched_ex
                       || BLAS1_EX == blas1_ex::dotc_ex || BLAS1_EX == blas1_ex::dotc_batched_ex
                       || BLAS1_EX == blas1_ex::dotc_strided_batched_ex);

                bool is_rot = (BLAS1_EX == blas1_ex::rot_ex || BLAS1_EX == blas1_ex::rot_batched_ex
                               || BLAS1_EX == blas1_ex::rot_strided_batched_ex);

                bool is_scal
                    = (BLAS1_EX == blas1_ex::scal_ex || BLAS1_EX == blas1_ex::scal_batched_ex
                       || BLAS1_EX == blas1_ex::scal_strided_batched_ex);

                bool is_nrm2
                    = (BLAS1_EX == blas1_ex::nrm2_ex || BLAS1_EX == blas1_ex::nrm2_batched_ex
                       || BLAS1_EX == blas1_ex::nrm2_strided_batched_ex);

                bool is_batched
                    = (BLAS1_EX == blas1_ex::axpy_batched_ex || BLAS1_EX == blas1_ex::dot_batched_ex
                       || BLAS1_EX == blas1_ex::rot_batched_ex
                       || BLAS1_EX == blas1_ex::scal_batched_ex
                       || BLAS1_EX == blas1_ex::nrm2_batched_ex);
                bool is_strided = (BLAS1_EX == blas1_ex::axpy_strided_batched_ex
                                   || BLAS1_EX == blas1_ex::dot_strided_batched_ex
                                   || BLAS1_EX == blas1_ex::rot_strided_batched_ex
                                   || BLAS1_EX == blas1_ex::scal_strided_batched_ex
                                   || BLAS1_EX == blas1_ex::nrm2_strided_batched_ex);

                name << rocblas_datatype2string(arg.a_type) << '_'
                     << rocblas_datatype2string(arg.b_type);

                if(is_axpy || is_dot || is_rot)
                    name << '_' << rocblas_datatype2string(arg.c_type);

                name << '_' << rocblas_datatype2string(arg.compute_type);

                name << '_' << arg.N;

                if(is_axpy || is_scal)
                    name << '_' << arg.alpha << '_' << arg.alphai;

                if(is_axpy || is_dot || is_scal || is_nrm2 || is_rot)
                    name << '_' << arg.incx;

                if(is_strided)
                    name << '_' << arg.stride_x;

                if(is_axpy || is_dot || is_rot)
                    name << '_' << arg.incy;

                if(is_strided && (is_axpy || is_dot || is_rot))
                    name << '_' << arg.stride_y;

                if(is_batched || is_strided)
                    name << '_' << arg.batch_count;

                if(arg.fortran)
                    name << "_F";
            }

            return std::move(name);
        }
    };

    // This tells whether the BLAS1_EX tests are enabled
    // Appears that we will need up to 4 template variables (see dot)
    template <blas1_ex BLAS1_EX, typename T1, typename T2, typename T3, typename T4>
    using blas1_ex_enabled = std::integral_constant<
        bool,
        // axpy_ex
        // T1 is alpha_type T2 is x_type, T3 is y_type, T4 is execution_type
        ((BLAS1_EX == blas1_ex::axpy_ex || BLAS1_EX == blas1_ex::axpy_batched_ex
          || BLAS1_EX == blas1_ex::axpy_strided_batched_ex)
         && ((std::is_same<T1, T2>{} && std::is_same<T2, T3>{} && std::is_same<T3, T4>{}
              && (std::is_same<T1, float>{} || std::is_same<T1, double>{}
                  || std::is_same<T1, rocblas_half>{} || std::is_same<T1, rocblas_float_complex>{}
                  || std::is_same<T1, rocblas_double_complex>{}))
             || (std::is_same<T1, T2>{} && std::is_same<T2, T3>{}
                 && std::is_same<T1, rocblas_half>{} && std::is_same<T4, float>{})
             || (std::is_same<T2, T3>{} && std::is_same<T1, T4>{}
                 && std::is_same<T2, rocblas_half>{} && std::is_same<T1, float>{})))

            // dot_ex
            // T1 is x_type, T2 is y_type, T3 is result_type, T4 is execution_type
            || ((BLAS1_EX == blas1_ex::dot_ex || BLAS1_EX == blas1_ex::dot_batched_ex
                 || BLAS1_EX == blas1_ex::dot_strided_batched_ex || BLAS1_EX == blas1_ex::dotc_ex
                 || BLAS1_EX == blas1_ex::dotc_batched_ex
                 || BLAS1_EX == blas1_ex::dotc_strided_batched_ex)
                && ((std::is_same<T1, T2>{} && std::is_same<T2, T3>{} && std::is_same<T3, T4>{}
                     && (std::is_same<T1, float>{} || std::is_same<T1, double>{}
                         || std::is_same<T1, rocblas_half>{}
                         || std::is_same<T1, rocblas_float_complex>{}
                         || std::is_same<T1, rocblas_double_complex>{}))
                    || (std::is_same<T1, T2>{} && std::is_same<T2, T3>{}
                        && std::is_same<T1, rocblas_half>{} && std::is_same<T4, float>{})
                    || (std::is_same<T1, T2>{} && std::is_same<T2, T3>{}
                        && std::is_same<T1, rocblas_bfloat16>{} && std::is_same<T4, float>{})))

            // rot_ex
            // T1 is x_type, T2 is y_type, T3 is cs_type, T4 is execution type
            || ((BLAS1_EX == blas1_ex::rot_ex || BLAS1_EX == blas1_ex::rot_batched_ex
                 || BLAS1_EX == blas1_ex::rot_strided_batched_ex)
                // regular calls where all types are the same
                && ((std::is_same<T1, T2>{} && std::is_same<T2, T3>{} && std::is_same<T3, T4>{}
                     && (std::is_same<T1, float>{} || std::is_same<T1, double>{}
                         || std::is_same<T1, rocblas_float_complex>{}
                         || std::is_same<T1, rocblas_double_complex>{}))
                    // float compute and float16/bfloat16 input/output
                    || (std::is_same<T1, T2>{} && std::is_same<T2, T3>{}
                        && std::is_same<T4, float>{}
                        && (std::is_same<T1, rocblas_bfloat16>{}
                            || std::is_same<T1, rocblas_half>{}))
                    // complex compute and x/y with real cs inputs
                    || (std::is_same<T1, T2>{} && std::is_same<T1, T4>{}
                        && std::is_same<T1, rocblas_float_complex>{} && std::is_same<T3, float>{})
                    || (std::is_same<T1, T2>{} && std::is_same<T1, T4>{}
                        && std::is_same<T1, rocblas_double_complex>{}
                        && std::is_same<T3, double>{})))

            // scal_ex
            // T1 is alpha_type T2 is x_type T3 is execution_type
            || ((BLAS1_EX == blas1_ex::scal_ex || BLAS1_EX == blas1_ex::scal_batched_ex
                 || BLAS1_EX == blas1_ex::scal_strided_batched_ex)
                && ((std::is_same<T1, T2>{} && std::is_same<T2, T3>{}
                     && (std::is_same<T1, float>{} || std::is_same<T1, double>{}
                         || std::is_same<T1, rocblas_half>{}
                         || std::is_same<T1, rocblas_float_complex>{}
                         || std::is_same<T1, rocblas_double_complex>{}))
                    || (std::is_same<T1, T2>{} && std::is_same<T1, rocblas_half>{}
                        && std::is_same<T3, float>{})
                    || (std::is_same<T1, T3>{} && std::is_same<T1, float>{}
                        && std::is_same<T2, rocblas_half>{})
                    || (std::is_same<T2, T3>{} && std::is_same<T1, float>{}
                        && std::is_same<T2, rocblas_float_complex>{})
                    || (std::is_same<T2, T3>{} && std::is_same<T1, double>{}
                        && std::is_same<T2, rocblas_double_complex>{})))

            // nrm2_ex
            // T1 is x_type T2 is result_type T3 is execution type
            || ((BLAS1_EX == blas1_ex::nrm2_ex || BLAS1_EX == blas1_ex::nrm2_batched_ex
                 || BLAS1_EX == blas1_ex::nrm2_strided_batched_ex)
                && ((std::is_same<T1, T2>{} && std::is_same<T2, T3>{}
                     && (std::is_same<T1, float>{} || std::is_same<T1, double>{}))
                    || (std::is_same<T1, rocblas_float_complex>{} && std::is_same<T2, float>{}
                        && std::is_same<T3, float>{})
                    || (std::is_same<T1, rocblas_double_complex>{} && std::is_same<T2, double>{}
                        && std::is_same<T3, double>{})
                    || (std::is_same<T1, rocblas_half>{} && std::is_same<T2, rocblas_half>{}
                        && std::is_same<T3, float>{})))>;

// Creates tests for one of the BLAS 1 functions
// ARG passes 1-3 template arguments to the testing_* function
#define BLAS1_EX_TESTING(NAME, ARG)                                                           \
    struct blas1_ex_##NAME                                                                    \
    {                                                                                         \
        template <typename Ta,                                                                \
                  typename Tb  = Ta,                                                          \
                  typename Tc  = Tb,                                                          \
                  typename Tex = Tc,                                                          \
                  typename     = void>                                                        \
        struct testing : rocblas_test_invalid                                                 \
        {                                                                                     \
        };                                                                                    \
                                                                                              \
        template <typename Ta, typename Tb, typename Tc, typename Tex>                        \
        struct testing<Ta,                                                                    \
                       Tb,                                                                    \
                       Tc,                                                                    \
                       Tex,                                                                   \
                       std::enable_if_t<blas1_ex_enabled<blas1_ex::NAME, Ta, Tb, Tc, Tex>{}>> \
            : rocblas_test_valid                                                              \
        {                                                                                     \
            void operator()(const Arguments& arg)                                             \
            {                                                                                 \
                if(!strcmp(arg.function, #NAME))                                              \
                    testing_##NAME<ARG(Ta, Tb, Tc, Tex)>(arg);                                \
                else if(!strcmp(arg.function, #NAME "_bad_arg"))                              \
                    testing_##NAME##_bad_arg<ARG(Ta, Tb, Tc, Tex)>(arg);                      \
                else                                                                          \
                    FAIL() << "Internal error: Test called with unknown function: "           \
                           << arg.function;                                                   \
            }                                                                                 \
        };                                                                                    \
    };                                                                                        \
                                                                                              \
    using NAME = blas1_ex_test_template<blas1_ex_##NAME::template testing, blas1_ex::NAME>;   \
                                                                                              \
    template <>                                                                               \
    inline bool NAME::function_filter(const Arguments& arg)                                   \
    {                                                                                         \
        return !strcmp(arg.function, #NAME) || !strcmp(arg.function, #NAME "_bad_arg");       \
    }                                                                                         \
                                                                                              \
    TEST_P(NAME, blas1_ex)                                                                    \
    {                                                                                         \
        RUN_TEST_ON_THREADS_STREAMS(                                                          \
            rocblas_blas1_ex_dispatch<blas1_ex_##NAME::template testing>(GetParam()));        \
    }                                                                                         \
                                                                                              \
    INSTANTIATE_TEST_CATEGORIES(NAME)

#define ARG1(Ta, Tb, Tc, Tex) Ta
#define ARG2(Ta, Tb, Tc, Tex) Ta, Tb
#define ARG3(Ta, Tb, Tc, Tex) Ta, Tb, Tc
#define ARG4(Ta, Tb, Tc, Tex) Ta, Tb, Tc, Tex

    BLAS1_EX_TESTING(axpy_ex, ARG4)
    BLAS1_EX_TESTING(axpy_batched_ex, ARG4)
    BLAS1_EX_TESTING(axpy_strided_batched_ex, ARG4)
    BLAS1_EX_TESTING(dot_ex, ARG4)
    BLAS1_EX_TESTING(dot_batched_ex, ARG4)
    BLAS1_EX_TESTING(dot_strided_batched_ex, ARG4)
    BLAS1_EX_TESTING(dotc_ex, ARG4)
    BLAS1_EX_TESTING(dotc_batched_ex, ARG4)
    BLAS1_EX_TESTING(dotc_strided_batched_ex, ARG4)
    BLAS1_EX_TESTING(nrm2_ex, ARG2)
    BLAS1_EX_TESTING(nrm2_batched_ex, ARG2)
    BLAS1_EX_TESTING(nrm2_strided_batched_ex, ARG2)
    BLAS1_EX_TESTING(rot_ex, ARG4)
    BLAS1_EX_TESTING(rot_batched_ex, ARG4)
    BLAS1_EX_TESTING(rot_strided_batched_ex, ARG4)
    BLAS1_EX_TESTING(scal_ex, ARG3)
    BLAS1_EX_TESTING(scal_batched_ex, ARG3)
    BLAS1_EX_TESTING(scal_strided_batched_ex, ARG3)

} // namespace
