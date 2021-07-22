***********************
Building and Installing
***********************


Prerequisites
-------------


* A ROCm enabled platform, more information `here <https://rocm.github.io/>`_ .


Installing pre-built packages
-----------------------------

rocBLAS can be installed on Ubuntu or Debian using
sudo apt-get update
sudo apt-get install rocblas
rocBLAS can be installed on CentOS using
sudo yum update
sudo yum install rocblas
rocBLAS can be installed on SLES using
sudo dnf upgrade
sudo dnf install rocblas
Once installed, rocBLAS can be used just like any other library with a C API.
The rocblas.h header file will need to be included in the user code in order to make calls
into rocBLAS, and the rocBLAS shared library will become link-time and run-time
dependent for the user applciation.
Once installed, rocblas.h and rocblas_module.f90 can be found in the /opt/rocm/include
directory. Only these two installed files should be used when needed in user code.
Other rocBLAS files can be found in /opt/rocm/include/internal, however these files
should not be directly included.

Building from source using install.sh
-------------------------------------

For most users building from source is not necessary, as rocBLAS can be used after installing the pre-built
packages as described above. If desired, the following instructions can be used to build rocBLAS from source.

Requirements
````````````

As a general rule, 64GB of system memory is required for a full rocBLAS build. This value can be lower if
rocBLAS is built with a different Tensile logic target (see the --logic command for ./install.sh). This value
may also increase in the future as more functions are added to rocBLAS and dependencies such as Tensile grow.

Download rocBLAS
````````````````

The rocBLAS source code is available at the `rocBLAS github page <https://github.com/ROCmSoftwarePlatform/rocBLAS>`_ . Check the ROCm Version on your system. For Ubuntu use
apt show rocm-libs -a
For Centos use
yum info rocm-libs
The ROCm version has major, minor, and patch fields, possibly followed by a build specific identifier. For example the ROCm version could be 4.0.0.40000-23, this corresponds to major = 4, minor = 0, patch = 0, build identifier 40000-23. There are GitHub branches at the rocBLAS site with names rocm-major.minor.x where major and minor are the same as in the ROCm version. For ROCm version 4.0.0.40000-23 you need to use the following to download rocBLAS:
git clone -b rocm-4.0.x https://github.com/ROCmSoftwarePlatform/rocBLAS.git
cd rocBLAS
For ROCm versions with other major and minor fields, clone the branch rocm-major.minor.x in place of rocm-4.0.x.
Below are steps to build either

* dependencies + library
* dependencies + library + client

You only need (dependencies + library) if you call rocBLAS from your code.
The client contains the test and benchmark code.

Build library dependencies + library
````````````````````````````````````

Common uses of install.sh to build (library dependencies + library) are
in the table below.
.. tabularcolumns::
   |\X{1}{4}|\X{3}{4}|


+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
necessary to rebuild theof install.sh it is notsubsequent invocationsto be used once. ForThe -d flag only needsin your local directory. |
+-------------------------------------------+-------------------------------------------+
assumed dependencies |
+-------------------------------------------+-------------------------------------------+
directory, you do notrocBLAS in your localIf you want to keepinstall for all users.sudo access. This willwill be prompted for/opt/rocm/rocblas. YourocBLAS package in |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
necessary to rebuild theinstall.sh it is notinvocations ofonce. For subsequentonly needs to be useddirectory. The -d flagand client in your localdependencies, library, |
+-------------------------------------------+-------------------------------------------+
dependencies have beenIt is assumed the |
+-------------------------------------------+-------------------------------------------+
you do not need the -iin your local directory,you want to keep rocBLASyou use the -i flag. Ifinstall for all usersthat if you want toaccess. It is expectedprompted for sudopackage. You will beinstall the rocBLASclient, then build anddependencies, library, |
+-------------------------------------------+-------------------------------------------+
directory, you do notrocBLAS in your localIf you want to keepinstall for all users.sudo access. This willwill be prompted forbuild the client. You |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
ROCM_PATH (/opt/rocm ifrocBLAS library at |
+-------------------------------------------+-------------------------------------------+
library at the specified |
+-------------------------------------------+-------------------------------------------+


Build library dependencies + client dependencies + library + client
```````````````````````````````````````````````````````````````````

The client contains executables in the table below.

+----------------------------------------------------+----------------------------------------------------+
 |
+----------------------------------------------------+----------------------------------------------------+
 |
+----------------------------------------------------+----------------------------------------------------+
necessary to rebuild theof install.sh it is notsubsequent invocationsto be used once. ForThe -d flag only needsin your local directory. |
+----------------------------------------------------+----------------------------------------------------+
assumed dependencies |
+----------------------------------------------------+----------------------------------------------------+
directory, you do notrocBLAS in your localIf you want to keepinstall for all users.sudo access. This willwill be prompted for/opt/rocm/rocblas. YourocBLAS package in |
+----------------------------------------------------+----------------------------------------------------+
 |
+----------------------------------------------------+----------------------------------------------------+
 |
+----------------------------------------------------+----------------------------------------------------+
 |
+----------------------------------------------------+----------------------------------------------------+
 |
+----------------------------------------------------+----------------------------------------------------+
 |
+----------------------------------------------------+----------------------------------------------------+
 |
+----------------------------------------------------+----------------------------------------------------+
necessary to rebuild theinstall.sh it is notinvocations ofonce. For subsequentonly needs to be useddirectory. The -d flagand client in your localdependencies, library, |
+----------------------------------------------------+----------------------------------------------------+
dependencies have beenIt is assumed the |
+----------------------------------------------------+----------------------------------------------------+
you do not need the -iin your local directory,you want to keep rocBLASyou use the -i flag. Ifinstall for all usersthat if you want toaccess. It is expectedprompted for sudopackage. You will beinstall the rocBLASclient, then build anddependencies, library, |
+----------------------------------------------------+----------------------------------------------------+
directory, you do notrocBLAS in your localIf you want to keepinstall for all users.sudo access. This willwill be prompted forbuild the client. You |
+----------------------------------------------------+----------------------------------------------------+
 |
+----------------------------------------------------+----------------------------------------------------+
ROCM_PATH (/opt/rocm ifrocBLAS library at |
+----------------------------------------------------+----------------------------------------------------+
library at the specified |
+----------------------------------------------------+----------------------------------------------------+

Common uses of install.sh to build (dependencies + library + client) are
in the table below.
.. tabularcolumns::
   |\X{1}{4}|\X{3}{4}|


+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
necessary to rebuild theof install.sh it is notsubsequent invocationsto be used once. ForThe -d flag only needsin your local directory. |
+-------------------------------------------+-------------------------------------------+
assumed dependencies |
+-------------------------------------------+-------------------------------------------+
directory, you do notrocBLAS in your localIf you want to keepinstall for all users.sudo access. This willwill be prompted for/opt/rocm/rocblas. YourocBLAS package in |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
necessary to rebuild theinstall.sh it is notinvocations ofonce. For subsequentonly needs to be useddirectory. The -d flagand client in your localdependencies, library, |
+-------------------------------------------+-------------------------------------------+
dependencies have beenIt is assumed the |
+-------------------------------------------+-------------------------------------------+
you do not need the -iin your local directory,you want to keep rocBLASyou use the -i flag. Ifinstall for all usersthat if you want toaccess. It is expectedprompted for sudopackage. You will beinstall the rocBLASclient, then build anddependencies, library, |
+-------------------------------------------+-------------------------------------------+
directory, you do notrocBLAS in your localIf you want to keepinstall for all users.sudo access. This willwill be prompted forbuild the client. You |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
ROCM_PATH (/opt/rocm ifrocBLAS library at |
+-------------------------------------------+-------------------------------------------+
library at the specified |
+-------------------------------------------+-------------------------------------------+


Build clients without library
`````````````````````````````

The rocBLAS clients can be built on their own using install.sh with a preexisting rocBLAS library.
Note that the version of the rocBLAS clients being built should match the version of the installed rocBLAS. The version of the installed rocBLAS can be found in the installed rocBLAS directory, in the file include/internal/rocblas-version.h. The version of rocBLAS being built can be found by running ``grep"VERSION_STRING" CMakeLists.txt`` in the rocBLAS directory being built.
.. tabularcolumns::
   |\X{1}{4}|\X{3}{4}|


+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
necessary to rebuild theof install.sh it is notsubsequent invocationsto be used once. ForThe -d flag only needsin your local directory. |
+-------------------------------------------+-------------------------------------------+
assumed dependencies |
+-------------------------------------------+-------------------------------------------+
directory, you do notrocBLAS in your localIf you want to keepinstall for all users.sudo access. This willwill be prompted for/opt/rocm/rocblas. YourocBLAS package in |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
necessary to rebuild theinstall.sh it is notinvocations ofonce. For subsequentonly needs to be useddirectory. The -d flagand client in your localdependencies, library, |
+-------------------------------------------+-------------------------------------------+
dependencies have beenIt is assumed the |
+-------------------------------------------+-------------------------------------------+
you do not need the -iin your local directory,you want to keep rocBLASyou use the -i flag. Ifinstall for all usersthat if you want toaccess. It is expectedprompted for sudopackage. You will beinstall the rocBLASclient, then build anddependencies, library, |
+-------------------------------------------+-------------------------------------------+
directory, you do notrocBLAS in your localIf you want to keepinstall for all users.sudo access. This willwill be prompted forbuild the client. You |
+-------------------------------------------+-------------------------------------------+
 |
+-------------------------------------------+-------------------------------------------+
ROCM_PATH (/opt/rocm ifrocBLAS library at |
+-------------------------------------------+-------------------------------------------+
library at the specified |
+-------------------------------------------+-------------------------------------------+


Dependencies
------------

Dependencies are listed in the script install.sh. The -d flag to install.sh installs dependencies.
CMake has a minimum version requirement listed in the file install.sh. See --cmake_install flag in install.sh to upgrade automatically.

Use of Tensile
--------------

The rocBLAS library uses
`Tensile <https://github.com/ROCmSoftwarePlatform/Tensile>`_ , which
supplies the high-performance implementation of xGEMM. Tensile is
downloaded by cmake during library configuration and automatically
configured as part of the build, so no further action is required by the
user to set it up.
