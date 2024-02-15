#
# CMakeLists.txt - CMake configuration file for swmm-solver/tests
#
# Created: Mar 4, 2020
# Updated: Feb 01, 2024
#
# Author: Michael E. Tryby
#         US EPA ORD/CESER
#


#Prep ourselves for compiling with boost
include(../extern/boost.cmake)


# Add solver tests
add_subdirectory(solver)


# Add output file tests
add_subdirectory(outfile)


# Setting up tests to run from build tree
set(TEST_BIN_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>")


# ctest doesn't like tests added in subdirectories so adding them here
add_test(
  NAME test_solver
  COMMAND "${TEST_BIN_DIRECTORY}/test_solver"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/solver/data
)

add_test(NAME test_output
    COMMAND "${TEST_BIN_DIRECTORY}/test_output"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/outfile/data
    )
