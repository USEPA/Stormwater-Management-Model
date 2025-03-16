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
find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)

# Sets the position independent code property for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Sets default install prefix when cmakecache is initialized for first time
if(SKBUILD)
    message(STATUS "Building with scikit-build-core")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_property (TARGET ${name} PROPERTY SUFFIX ".${SKBUILD_SOABI}.pyd")
else()
    set_property (TARGET ${name} PROPERTY SUFFIX ".${SKBUILD_SOABI}${CMAKE_SHARED_MODULE_SUFFIX}")
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

# Recursivily search for solver and output libraries in parent directory
list(APPEND CMAKE_MODULE_PATH "C:/Users/CBUAHIN/SourceCodes/cbuahin/Stormwater-Management-Model/build/_CPack_Packages/win64/ZIP/swmm-solver-5.3.0-win64/cmake")

find_package(swmm5 REQUIRED)
find_package(swmm_output REQUIRED)

add_subdirectory(epaswmm)

