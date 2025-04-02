#
# CMakeLists.txt - CMake configuration file for swmm-solver
#
# Created:  July 11, 2019
# Modified: February 26, 2025
#
# Author: Michael E. Tryby
#         US EPA ORD/CESER
#
#         Caleb Buahin (Modified)
#         US EPA ORD/CESER 
#

cmake_minimum_required (VERSION 3.15...3.31)

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    message(FATAL_ERROR "In-source builds are disabled.") 
endif()

project(swmm
    VERSION 5.3.0
    LANGUAGES C CXX
)

set(CMAKE_MACOSX_RPATH ON)

# Sets the position independent code property for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Define build options
option(BUILD_TESTS "Builds component tests (requires Boost)" OFF)
option(BUILD_DEF   "Builds library with def file interface" OFF)


# Add project subdirectories
add_subdirectory(src/outfile)
add_subdirectory(src/solver)
add_subdirectory(src/run)

if(BUILD_TESTS)
    enable_testing() 
    add_subdirectory(tests)
endif()

# CONFIGURE BUILD_NUMBER, BUILD_ID, BUILD_COMMIT, BUILD_BRANCH, BUILD_DATE
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC")
set(BUILD_DATE "${BUILD_TIMESTAMP}" CACHE STRING "Build date" FORCE)

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the branch name
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(
    UUID BUILD_ID 
    NAMESPACE "00000000-0000-0000-0000-000000000000" 
    NAME "https://www.epa.gov/water-research/storm-water-management-model-swmm" 
    TYPE MD5
)
set(BUILD_ID "${BUILD_ID}" CACHE STRING "Build ID" FORCE)
set(BUILD_BRANCH "${GIT_BRANCH_NAME}" CACHE STRING "Build branch" FORCE)
set(BUILD_COMMIT "${GIT_COMMIT_HASH}" CACHE STRING "Build commit" FORCE)

# Create install rules for vcruntime.dll, msvcp.dll, vcomp.dll etc.
set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Configure the package configuration file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/swmmConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/swmmConfig.cmake
    @ONLY
)

# Generate the version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/swmmConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Define install locations (will be prepended by install prefix)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/swmmConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/swmmConfigVersion.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/swmm
)

# Install the config file
install(
    EXPORT
        swmmTargets
    NAMESPACE
        swmm::
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/swmm
)


# Configure CPack driven installer package
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VENDOR "US_EPA")
set(CPACK_ARCHIVE_FILE_NAME "swmm")

include(CPack)
