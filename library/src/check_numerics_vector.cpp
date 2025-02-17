#include "check_numerics_vector.hpp"
#include "utility.hpp"

/**
  *
  * rocblas_check_numerics_abnormal_struct(function_name, check_numerics, is_input, h_abnormal)
  *
  * Info about rocblas_check_numerics_abnormal_struct function:
  *
  *    It is the host function which accepts the 'h_abnormal' structure and
  *    also helps in debugging based on the different types of flags in rocblas_check_numerics_mode that users set to debug potential NaN/zero/Infinity.
  *
  * Parameters   : function_name         : Name of the rocBLAS math function
  *                check_numerics        : User defined flag for debugging
  *                is_input              : To check if the vector under consideration is an Input or an Output vector
  *                h_abnormal            : Structure holding the boolean NaN/zero/Inf
  *
  * Return Value : rocblas_status
  *        rocblas_status_success        : Return status if the vector does not have a NaN/Inf
  *   rocblas_status_check_numerics_fail : Return status if the vector contains a NaN/Inf and 'check_numerics' enum is set to 'rocblas_check_numerics_mode_fail'
  *
**/

rocblas_status rocblas_check_numerics_abnormal_struct(const char*               function_name,
                                                      const int                 check_numerics,
                                                      bool                      is_input,
                                                      rocblas_check_numerics_t* h_abnormal)
{
    //is_abnormal will be set if the vector has a NaN or an Infinity
    bool is_abnormal = (h_abnormal->has_NaN != 0) || (h_abnormal->has_Inf != 0);

    //Fully informative message will be printed if 'check_numerics == ROCBLAS_CHECK_NUMERICS_INFO' or 'check_numerics == ROCBLAS_CHECK_NUMERICS_WARN' and 'is_abnormal'
    if(((check_numerics & rocblas_check_numerics_mode_info) != 0)
       || (((check_numerics & rocblas_check_numerics_mode_warn) != 0) && is_abnormal))
    {
        if(is_input)
        {
            rocblas_cerr << "Funtion name:\t" << function_name << " :- Input :\t"
                         << " has_NaN " << h_abnormal->has_NaN << " has_zero "
                         << h_abnormal->has_zero << " has_Inf " << h_abnormal->has_Inf << std::endl;
        }
        else
        {
            rocblas_cerr << "Funtion name:\t" << function_name << " :- Output :\t"
                         << " has_NaN " << h_abnormal->has_NaN << " has_zero "
                         << h_abnormal->has_zero << " has_Inf " << h_abnormal->has_Inf << std::endl;
        }
    }

    if(is_abnormal)
    { //If 'check_numerics ==rocblas_check_numerics_mode_fail' then the 'rocblas_status_check_numerics_fail' status
        //is returned which signifies that the vector has a NaN/Inf
        if((check_numerics & rocblas_check_numerics_mode_fail) != 0)
            return rocblas_status_check_numerics_fail;
    }
    return rocblas_status_success;
}
/**
  *
  * rocblas_internal_check_numerics_vector_template(function_name, handle, n, x, offset_x, inc_x, stride_x, batch_count, check_numerics, is_input)
  *
  * Info about rocblas_internal_check_numerics_vector_template function:
  *
  *    It is the host function which accepts a vector and calls the 'rocblas_check_numerics_vector_kernel' kernel function
  *    to check for numerical abnormalities such as NaN/zero/Infinity in that vector.
  *    It also helps in debugging based on the different types of flags in rocblas_check_numerics_mode that users set to debug potential NaN/zero/Infinity.
  *
  * Parameters   : function_name         : Name of the rocBLAS math function
  *                handle                : Handle to the rocblas library context queue
  *                n                     : Total number of elements in the vector 'x'
  *                x                     : Pointer to the vector which is under check for numerical abnormalities
  *                offset_x              : Offset of vector 'x'
  *                inc_x                 : Stride between consecutive values of vector 'x'
  *                stride_x              : Specifies the pointer increment between one vector 'x_i' and the next one (x_i+1) (where (x_i) is the i-th instance of the batch)
  *                check_numerics        : User defined flag for debugging
  *                is_input              : To check if the vector under consideration is an Input or an Output vector
  *
  * Return Value : rocblas_status
  *        rocblas_status_success        : Return status if the vector does not have a NaN/Inf
  *   rocblas_status_check_numerics_fail : Return status if the vector contains a NaN/Inf and 'check_numerics' enum is set to 'rocblas_check_numerics_mode_fail'
  *
**/

template <typename T>
ROCBLAS_INTERNAL_EXPORT_NOINLINE rocblas_status
    rocblas_internal_check_numerics_vector_template(const char*    function_name,
                                                    rocblas_handle handle,
                                                    rocblas_int    n,
                                                    T              x,
                                                    rocblas_int    offset_x,
                                                    rocblas_int    inc_x,
                                                    rocblas_stride stride_x,
                                                    rocblas_int    batch_count,
                                                    const int      check_numerics,
                                                    bool           is_input)
{
    //Quick return if possible. Not Argument error
    if(n <= 0 || inc_x <= 0 || batch_count <= 0 || !x)
    {
        return rocblas_status_success;
    }

    //Creating structure host object
    rocblas_check_numerics_t h_abnormal;

    //Allocating memory for device structure
    auto d_abnormal = handle->device_malloc(sizeof(rocblas_check_numerics_t));

    //Transferring the rocblas_check_numerics_t structure from host to the device
    RETURN_IF_HIP_ERROR(hipMemcpy((rocblas_check_numerics_t*)d_abnormal,
                                  &h_abnormal,
                                  sizeof(rocblas_check_numerics_t),
                                  hipMemcpyHostToDevice));

    hipStream_t           rocblas_stream = handle->get_stream();
    constexpr rocblas_int NB             = 256;
    dim3                  blocks((n - 1) / NB + 1, batch_count);
    dim3                  threads(NB);

    hipLaunchKernelGGL(rocblas_check_numerics_vector_kernel,
                       blocks,
                       threads,
                       0,
                       rocblas_stream,
                       n,
                       x,
                       offset_x,
                       inc_x,
                       stride_x,
                       (rocblas_check_numerics_t*)d_abnormal);

    //Transferring the rocblas_check_numerics_t structure from device to the host
    RETURN_IF_HIP_ERROR(hipMemcpy(&h_abnormal,
                                  (rocblas_check_numerics_t*)d_abnormal,
                                  sizeof(rocblas_check_numerics_t),
                                  hipMemcpyDeviceToHost));

    return rocblas_check_numerics_abnormal_struct(
        function_name, check_numerics, is_input, &h_abnormal);
}

//ADDED INSTANTIATION TO SUPPORT T* AND T* CONST*

#ifdef INST
#error INST IS ALREADY DEFINED
#endif
#define INST(typet_)                                                                   \
    template ROCBLAS_INTERNAL_EXPORT_NOINLINE rocblas_status                           \
        rocblas_internal_check_numerics_vector_template(const char*    function_name,  \
                                                        rocblas_handle handle,         \
                                                        rocblas_int    n,              \
                                                        typet_         x,              \
                                                        rocblas_int    offset_x,       \
                                                        rocblas_int    incx,           \
                                                        rocblas_stride stride_x,       \
                                                        rocblas_int    batch_count,    \
                                                        const int      check_numerics, \
                                                        bool           is_input)
INST(float*);
INST(double*);
INST(float* const*);
INST(double* const*);
INST(float const*);
INST(double const*);
INST(const float* const*);
INST(const double* const*);
INST(rocblas_float_complex*);
INST(rocblas_double_complex*);
INST(rocblas_float_complex* const*);
INST(rocblas_double_complex* const*);
INST(const rocblas_float_complex* const*);
INST(const rocblas_double_complex* const*);
INST(rocblas_float_complex const*);
INST(rocblas_double_complex const*);
INST(rocblas_half*);
INST(rocblas_bfloat16*);
INST(rocblas_half* const*);
INST(rocblas_bfloat16* const*);
INST(const rocblas_half* const*);
INST(const rocblas_bfloat16* const*);
INST(rocblas_half const*);
INST(rocblas_bfloat16 const*);
#undef INST
