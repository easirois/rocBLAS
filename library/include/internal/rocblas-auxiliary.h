/* ************************************************************************
 * Copyright 2016-2021 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#ifndef _ROCBLAS_AUXILIARY_H_
#define _ROCBLAS_AUXILIARY_H_
#include "rocblas-export.h"
#include "rocblas-types.h"

/*!\file
 * \brief rocblas-auxiliary.h provides auxilary functions in rocblas
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief create handle
 */
ROCBLAS_EXPORT rocblas_status rocblas_create_handle(rocblas_handle* handle);

/*! \brief destroy handle
 */
ROCBLAS_EXPORT rocblas_status rocblas_destroy_handle(rocblas_handle handle);

/*! \brief set stream for handle
 */
ROCBLAS_EXPORT rocblas_status rocblas_set_stream(rocblas_handle handle, hipStream_t stream);

/*! \brief get stream [0] from handle
 */
ROCBLAS_EXPORT rocblas_status rocblas_get_stream(rocblas_handle handle, hipStream_t* stream);

/*! \brief set rocblas_pointer_mode
 */
ROCBLAS_EXPORT rocblas_status rocblas_set_pointer_mode(rocblas_handle       handle,
                                                       rocblas_pointer_mode pointer_mode);

/*! \brief get rocblas_pointer_mode
 */
ROCBLAS_EXPORT rocblas_status rocblas_get_pointer_mode(rocblas_handle        handle,
                                                       rocblas_pointer_mode* pointer_mode);

/*! \brief set rocblas_atomics_mode
 */
ROCBLAS_EXPORT rocblas_status rocblas_set_atomics_mode(rocblas_handle       handle,
                                                       rocblas_atomics_mode atomics_mode);

/*! \brief get rocblas_atomics_mode
 */
ROCBLAS_EXPORT rocblas_status rocblas_get_atomics_mode(rocblas_handle        handle,
                                                       rocblas_atomics_mode* atomics_mode);

/*! \brief query the preferable supported int8 input layout for gemm
     \details
    Indicates the supported int8 input layout for gemm according to the device.
    If the device supports packed-int8x4 (1) only, output flag is rocblas_gemm_flags_pack_int8x4
    and users must bitwise-or your flag with rocblas_gemm_flags_pack_int8x4.
    If output flag is rocblas_gemm_flags_none (0), then unpacked int8 is preferable and suggested.
    @param[in]
    handle      [rocblas_handle]
                the handle of device
    @param[out]
    flag        pointer to rocblas_gemm_flags
     ********************************************************************/
ROCBLAS_EXPORT rocblas_status rocblas_query_int8_layout_flag(rocblas_handle      handle,
                                                             rocblas_gemm_flags* flag);

/*! \brief  Indicates whether the pointer is on the host or device.
 */
ROCBLAS_EXPORT rocblas_pointer_mode rocblas_pointer_to_mode(void* ptr);

/*! \brief copy vector from host to device
 */
ROCBLAS_EXPORT rocblas_status rocblas_set_vector(rocblas_int n,
                                                 rocblas_int elem_size,
                                                 const void* x,
                                                 rocblas_int incx,
                                                 void*       y,
                                                 rocblas_int incy);

/*! \brief copy vector from device to host
 */
ROCBLAS_EXPORT rocblas_status rocblas_get_vector(rocblas_int n,
                                                 rocblas_int elem_size,
                                                 const void* x,
                                                 rocblas_int incx,
                                                 void*       y,
                                                 rocblas_int incy);

/*! \brief copy matrix from host to device
 */
ROCBLAS_EXPORT rocblas_status rocblas_set_matrix(rocblas_int rows,
                                                 rocblas_int cols,
                                                 rocblas_int elem_size,
                                                 const void* a,
                                                 rocblas_int lda,
                                                 void*       b,
                                                 rocblas_int ldb);

/*! \brief copy matrix from device to host
 */
ROCBLAS_EXPORT rocblas_status rocblas_get_matrix(rocblas_int rows,
                                                 rocblas_int cols,
                                                 rocblas_int elem_size,
                                                 const void* a,
                                                 rocblas_int lda,
                                                 void*       b,
                                                 rocblas_int ldb);

/*! \brief asynchronously copy vector from host to device
     \details
    rocblas_set_vector_async copies a vector from pinned host memory to device memory asynchronously.
    Memory on the host must be allocated with hipHostMalloc or the transfer will be synchronous.
    @param[in]
    n           [rocblas_int]
                number of elements in the vector
    @param[in]
    x           pointer to vector on the host
    @param[in]
    incx        [rocblas_int]
                specifies the increment for the elements of the vector
    @param[out]
    y           pointer to vector on the device
    @param[in]
    incy        [rocblas_int]
                specifies the increment for the elements of the vector
    @param[in]
    stream      specifies the stream into which this transfer request is queued
     ********************************************************************/
ROCBLAS_EXPORT rocblas_status rocblas_set_vector_async(rocblas_int n,
                                                       rocblas_int elem_size,
                                                       const void* x,
                                                       rocblas_int incx,
                                                       void*       y,
                                                       rocblas_int incy,
                                                       hipStream_t stream);

/*! \brief asynchronously copy vector from device to host
     \details
    rocblas_get_vector_async copies a vector from pinned host memory to device memory asynchronously.
    Memory on the host must be allocated with hipHostMalloc or the transfer will be synchronous.
    @param[in]
    n           [rocblas_int]
                number of elements in the vector
    @param[in]
    x           pointer to vector on the device
    @param[in]
    incx        [rocblas_int]
                specifies the increment for the elements of the vector
    @param[out]
    y           pointer to vector on the host
    @param[in]
    incy        [rocblas_int]
                specifies the increment for the elements of the vector
    @param[in]
    stream      specifies the stream into which this transfer request is queued
     ********************************************************************/
ROCBLAS_EXPORT rocblas_status rocblas_get_vector_async(rocblas_int n,
                                                       rocblas_int elem_size,
                                                       const void* x,
                                                       rocblas_int incx,
                                                       void*       y,
                                                       rocblas_int incy,
                                                       hipStream_t stream);

/*! \brief asynchronously copy matrix from host to device
     \details
    rocblas_set_matrix_async copies a matrix from pinned host memory to device memory asynchronously.
    Memory on the host must be allocated with hipHostMalloc or the transfer will be synchronous.
    @param[in]
    rows        [rocblas_int]
                number of rows in matrices
    @param[in]
    cols        [rocblas_int]
                number of columns in matrices
    @param[in]
    elem_size   [rocblas_int]
                number of bytes per element in the matrix
    @param[in]
    a           pointer to matrix on the host
    @param[in]
    lda         [rocblas_int]
                specifies the leading dimension of A
    @param[out]
    b           pointer to matrix on the GPU
    @param[in]
    ldb         [rocblas_int]
                specifies the leading dimension of B
    @param[in]
    stream      specifies the stream into which this transfer request is queued
     ********************************************************************/
ROCBLAS_EXPORT rocblas_status rocblas_set_matrix_async(rocblas_int rows,
                                                       rocblas_int cols,
                                                       rocblas_int elem_size,
                                                       const void* a,
                                                       rocblas_int lda,
                                                       void*       b,
                                                       rocblas_int ldb,
                                                       hipStream_t stream);

/*! \brief asynchronously copy matrix from device to host
     \details
    rocblas_get_matrix_async copies a matrix from device memory to pinned host memory asynchronously.
    Memory on the host must be allocated with hipHostMalloc or the transfer will be synchronous.
    @param[in]
    rows        [rocblas_int]
                number of rows in matrices
    @param[in]
    cols        [rocblas_int]
                number of columns in matrices
    @param[in]
    elem_size   [rocblas_int]
                number of bytes per element in the matrix
    @param[in]
    a           pointer to matrix on the GPU
    @param[in]
    lda         [rocblas_int]
                specifies the leading dimension of A
    @param[out]
    b           pointer to matrix on the host
    @param[in]
    ldb         [rocblas_int]
                specifies the leading dimension of B
    @param[in]
    stream      specifies the stream into which this transfer request is queued
     ********************************************************************/
ROCBLAS_EXPORT rocblas_status rocblas_get_matrix_async(rocblas_int rows,
                                                       rocblas_int cols,
                                                       rocblas_int elem_size,
                                                       const void* a,
                                                       rocblas_int lda,
                                                       void*       b,
                                                       rocblas_int ldb,
                                                       hipStream_t stream);

/*******************************************************************************
 * Function to set start/stop event handlers (for internal use only)
 ******************************************************************************/
ROCBLAS_EXPORT rocblas_status rocblas_set_start_stop_events(rocblas_handle handle,
                                                            hipEvent_t     startEvent,
                                                            hipEvent_t     stopEvent);

#define ROCBLAS_INVOKE_START_STOP_EVENTS(handle, startEvent, stopEvent, call) \
    do                                                                        \
    {                                                                         \
        rocblas_handle tmp_h = (handle);                                      \
        rocblas_set_start_stop_events(tmp_h, (startEvent), (stopEvent));      \
        call;                                                                 \
        rocblas_set_start_stop_events(tmp_h, (hipEvent_t)0, (hipEvent_t)0);   \
    } while(0)

// For testing solution selection fitness -- for internal testing only
ROCBLAS_EXPORT rocblas_status rocblas_set_solution_fitness_query(rocblas_handle handle,
                                                                 double*        fitness);

/*! \brief specifies the performance metric that solution selection uses
     \details
    Determines which performance metric will be used by Tensile when selecting the optimal solution
    for gemm problems. If a valid solution benchmarked for this performance metric does not exist
    for a problem, Tensile will default to a solution benchmarked for overall performance instead.
    @param[in]
    handle      [rocblas_handle]
                the handle of device
    @param[in]
    metric      [rocblas_performance_metric]
                the performance metric to be used
     ********************************************************************/
ROCBLAS_EXPORT rocblas_status rocblas_set_performance_metric(rocblas_handle             handle,
                                                             rocblas_performance_metric metric);
/*! \brief returns the performance metric being used for solution selection
     \details
    Returns the performance metric used by Tensile to select the optimal solution for gemm problems.
    @param[in]
    handle      [rocblas_handle]
                the handle of device
    @param[out]
    metric      [rocblas_performance_metric*]
                pointer to where the metric will be stored
     ********************************************************************/
ROCBLAS_EXPORT rocblas_status rocblas_get_performance_metric(rocblas_handle              handle,
                                                             rocblas_performance_metric* metric);

#ifdef __cplusplus
}
#endif

#endif /* _ROCBLAS_AUXILIARY_H_ */
