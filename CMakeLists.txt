#
# CMakeLists.txt - CMake configuration file for swmm-solver
#
# Created: July 11, 2019
#
# Modified by: Michael E. Tryby
#              US EPA ORD/NRMRL
#

cmake_minimum_required (VERSION 3.0)


project(swmm-solver
    VERSION 5.1.14
    LANGUAGES C CXX
    )


# Sets default install prefix when cmakecache is initialized for first time
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "..." FORCE)
endif()


# Define install locations (will be prepended by install prefix)
set(TOOL_DIST "bin")
set(INCLUDE_DIST "include")
set(LIBRARY_DIST "lib")
set(CONFIG_DIST  "cmake")


# Add project subdirectories
add_subdirectory(outfile)
add_subdirectory(src/solver)
add_subdirectory(src/run)


# Create target import scripts so other cmake projects can use swmm libraries
install(
    EXPORT
        swmm5Targets
    DESTINATION
        "${CONFIG_DIST}"
    FILE
        swmm5-config.cmake
    )


# Configure CPack driven installer package
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VENDOR "US_EPA")
set(CPACK_ARCHIVE_FILE_NAME "swmm")

include(CPack)
