#
# CMakeLists.txt - CMake configuration file for swmm-solver
#
# Created: July 11, 2019
# Updated: May 19, 2021
#
# Authors: Michael E. Tryby
#          US EPA ORD/CESER
#

################################################################################
##################    CMAKELISTS FOR SWMM-SOLVER PROJECT    ####################
################################################################################


cmake_minimum_required (VERSION 3.13)

# Disable in-source builds (they are messy)
if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    message(FATAL_ERROR "In-source builds are disabled.")
endif()


project(swmm-solver
    VERSION 5.1.14
    LANGUAGES C CXX
)

# Append local dir to module search path
# list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Sets the position independent code property for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Sets default install prefix when cmakecache is initialized for first time
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "..." FORCE)
endif()


# Define install locations (will be prepended by install prefix)
set(TOOL_DIST "bin")
set(INCLUDE_DIST "include")
set(LIBRARY_DIST "lib")
set(CONFIG_DIST  "cmake")


# Define build options
option(BUILD_TESTS "Build component tests (requires Boost)" OFF)
option(BUILD_DOCS "Build toolkit docs (requires Doxygen)" OFF)

# Added option to statically link libraries to address GitHub Ubuntu 20.04 symbol errors (issue #340)
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Add project subdirectories
if(BUILD_DOCS)
  add_subdirectory(docs)
endif()

add_subdirectory(src/outfile)
add_subdirectory(src/solver)
add_subdirectory(src/run)
add_subdirectory(src/shared)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()


# Create install rules for vcruntime.dll, msvcp.dll, vcomp.dll etc.
if(OpenMP_FOUND)
    set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
endif()
include(InstallRequiredSystemLibraries)


# Create install target for License and Contributors files
install(
    FILES
        "LICENSE"
        "CONTRIBUTORS"
    DESTINATION
        "."
)


# Configure CPack driven installer package
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_PACKAGE_VENDOR "OWA")
set(CPACK_ARCHIVE_FILE_NAME "swmm")

include(CPack)
