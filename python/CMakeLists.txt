# Description: CMakeLists.txt file for the epaswmm python package
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

cmake_minimum_required(VERSION 3.11)

project(
    epaswmm
    VERSION 5.3.0
    LANGUAGES C CXX
)

# Define build options
cmake_policy(SET CMP0148 OLD)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(PythonExtensions REQUIRED)
find_package(Cython REQUIRED)

# Sets the position independent code property for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Sets default install prefix when cmakecache is initialized for first time
if(SKBUILD)
    message(STATUS "Building with scikit-build")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_property (TARGET ${name} PROPERTY SUFFIX ".${SKBUILD_SOABI}.pyd")
else()
    set_property (TARGET ${name} PROPERTY SUFFIX ".${SKBUILD_SOABI}${CMAKE_SHARED_MODULE_SUFFIX}")
endif()

# Create install rules for vcruntime.dll, msvcp.dll, vcomp.dll etc.
set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

# Recursivily search for solver and output libraries in parent directory
list(APPEND CMAKE_PREFIX_PATH "/Users/calebbuahin/Documents/Projects/cbuahin_github/Stormwater-Management-Model/build/debug/_CPack_Packages/Darwin/ZIP/swmm-5.3.0-Darwin/lib/cmake/swmm")

find_package(swmm REQUIRED)

add_subdirectory(epaswmm)

