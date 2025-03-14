# Description: CMakeLists.txt file for the epaswmm python package
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

cmake_minimum_required(VERSION 3.15...3.31)

project(
    epaswmm
    VERSION 5.3.0
    LANGUAGES C CXX
)

# Define build options
cmake_policy(SET CMP0148 OLD)
find_package(Python REQUIRED COMPONENTS Interpreter Development)
find_package(Cython REQUIRED)

# Sets the position independent code property for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Sets default install prefix when cmakecache is initialized for first time
if(SKBUILD)
    message(STATUS "Building with scikit-build")
endif()

# Sets default install prefix when cmakecache is initialized for first time
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "..." FORCE)
endif()

# Define install locations (will be prepended by install prefix)
set(TOOL_DIST    "bin")
set(INCLUDE_DIST "include")
set(LIBRARY_DIST "lib")
set(CONFIG_DIST  "cmake")

# Create install rules for vcruntime.dll, msvcp.dll, vcomp.dll etc.
set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

add_subdirectory(../src/solver ./solver_build)
add_subdirectory(../src/outfile ./output_build)

add_subdirectory(epaswmm)

