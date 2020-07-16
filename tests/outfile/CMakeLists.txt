#
# CMakeLists.txt - CMake configuration file for tests/outfile
#
# Created: July 11, 2019
#
# Author: Michael E. Tryby
#         US EPA ORD/CESER
#

add_executable(test_output
    test_output.cpp
)

target_include_directories(test_output
    PUBLIC ../../outfile/include
)

target_link_libraries(test_output
    ${Boost_LIBRARIES}
    swmm-output
)

set_target_properties(test_output
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
