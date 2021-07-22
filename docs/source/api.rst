***
API
***


Rules for obtaining the rocBLAS API from Legacy BLAS
----------------------------------------------------


#. The Legacy BLAS routine name is changed to lower case, and prefixed
by rocblas_.
#. A first argument rocblas_handle handle is added to all rocBlas
functions.
#. Input arguments are declared with the const modifier.
#. Character arguments are replaced with enumerated types defined in
rocblas_types.h. They are passed by value on the host.
#. Array arguments are passed by reference on the device.
#. Scalar arguments are passed by value on the host with the following
two exceptions. See the section Pointer Mode for more information on
these two exceptions.

* Scalar values alpha and beta are passed by reference on either the
host or the device.
* Where Legacy BLAS functions have return values, the return value is
instead added as the last function argument. It is returned by
reference on either the host or the device. This applies to the
following functions: xDOT, xDOTU, xNRM2, xASUM, IxAMAX, IxAMIN.


#. The return value of all functions is rocblas_status, defined in
rocblas_types.h. It is used to check for errors.

LP64 interface
--------------

The rocBLAS library is LP64, so rocblas_int arguments are 32 bit and
rocblas_long arguments are 64 bit.

Column-major storage and 1 based indexing
-----------------------------------------

rocBLAS uses column-major storage for 2D arrays, and 1 based indexing
for the functions xMAX and xMIN. This is the same as Legacy BLAS and
cuBLAS.
If you need row-major and 0 based indexing (used in C language arrays)
download the `CBLAS <http://www.netlib.org/blas/#_cblas>`_  file
cblas.tgz. Look at the CBLAS functions that provide a thin interface to
Legacy BLAS. They convert from row-major, 0 based, to column-major, 1
based. This is done by swapping the order of function arguments. It is
not necessary to transpose matrices.

Pointer mode
------------

The auxiliary functions rocblas_set_pointer and rocblas_get_pointer are
used to set and get the value of the state variable
rocblas_pointer_mode. This variable is stored in rocblas_handle. If rocblas_pointer_mode ==
rocblas_pointer_mode_host then scalar parameters must be allocated on
the host. If rocblas_pointer_mode == rocblas_pointer_mode_device, then
scalar parameters must be allocated on the device.
There are two types of scalar parameter:

* scaling parameters like alpha and beta used in functions like axpy, gemv, gemm 2
* scalar results from functions amax, amin, asum, dot, nrm2

For scalar parameters like alpha and beta when rocblas_pointer_mode ==
rocblas_pointer_mode_host they can be allocated on the host heap or
stack. The kernel launch is asynchronous, and if they are on the heap
they can be freed after the return from the kernel launch. When
rocblas_pointer_mode == rocblas_pointer_mode_device they must not be
changed till the kernel completes.
For scalar results, when rocblas_pointer_mode ==
rocblas_pointer_mode_host then the function blocks the CPU till the GPU
has copied the result back to the host. When rocblas_pointer_mode ==
rocblas_pointer_mode_device the function will return after the
asynchronous launch. Similarly to vector and matrix results, the scalar
result is only available when the kernel has completed execution.

Asynchronous API
----------------

rocBLAS functions will be asynchronous unless:

* the function needs to allocate device memory
* the function returns a scalar result from GPU to CPU

The order of operations in the asynchronous functions is as in the figure
below. The argument checking, calculation of process grid, and kernel
launch take very little time. The asynchronous kernel running on the GPU
does not block the CPU. After the kernel launch the CPU keeps processing
the next instructions.
asynch_blocks
Order of operations in asynchronous functions
---------------------------------------------


.. image:: yqi1626210315551.png

The above order of operations will change if there is logging, or if the
function is synchronous. Logging requires system calls, and the program
will need to wait for them to complete before executing the next instruction.
See the Logging section for more information.

.. note::
	The default is no logging.


If the cpu needs to allocate device memory, it needs to wait till this is complete before
executing the next instruction. See the Device Memory Allocation section for more information.

.. note::
	Memory can be pre-allocated. This will make the function asynchronous as it removes the need for the function to allocate memory.


The following functions copy a scalar result from GPU to CPU if
rocblas_pointer_mode == rocblas_pointer_mode_host: asum, dot, max, min, nrm2.
This makes the function synchronous, as the program will need to wait
for the copy before executing the next instruction. See the section on
Pointer Mode for more information

.. note::
	Set rocblas_pointer_mode to rocblas_pointer_mode_device make the function asynchronous by keeping the result on the GPU.


The order of operations with logging, device memory allocation and return of a scalar
result is as in the figure below:
asynch_blocks
Code blocks in synchronous function call
----------------------------------------


.. image:: szb1626210316396.png


MI100 (gfx908) Considerations
-----------------------------

On nodes with the MI100 (gfx908), MFMA instructions are available to
substantially speed up matrix operations.  This hardware feature is
used in all gemm and gemm based functions in rocBLAS with 32-bit
or shorter base datatypes with an associated 32-bit compute_type
(f32_r, i32_r or f32_c as appropriate).
Specifically, rocBLAS takes advantage of MI100's MFMA instructions for
three real base types f16_r, bf16_r and f32_r with compute_type f32_r,
one integral base type i8_r with compute_type i32_r, and one complex
base type f32_c with compute_type f32_c.  In summary, all GEMM APIs and
APIs for GEMM based functions using these five base types and their
associated compute_type (explicit or implicit) take advantage of MI100's
MFMA instructions.

.. note::
	The use of MI100's MFMA instructions is automatic.  There is no user control for on/off.
Not all problem sizes may select MFMA based kernels; additional tuning may be needed to get good performance.



gfx90a Considerations
---------------------

On nodes with gfx90a, MFMA_F64 instructions are available to
substantially speed up double precision matrix operations.  This
hardware feature is used in all GEMM and GEMM based functions in
rocBLAS with 64-bit floating-point datatype, namely DGEMM, ZGEMM,
DTRSM, ZTRSM, DTRMM, ZTRMM, DSYRKX and ZSYRKX.

.. note::
	The use of gfx90a's MFMA_F64 instructions is automatic.  There is no user control for on/off.
Not all problem sizes may select MFMA_F64 based kernels; additional tuning may be needed to get good performance.


