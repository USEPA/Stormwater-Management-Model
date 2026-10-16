#
# CMakeLists.txt - CMake configuration file for tests/solver
#
# Regression tests for the solver library (swmm5).
#

add_executable(test_solver
    test_solver.cpp
    fill_malloc.c
    )
target_include_directories(test_solver
    PUBLIC ../../src/solver/include ../../src/outfile/include
    )
target_link_libraries(test_solver
    ${Boost_LIBRARIES}
    swmm5
    swmm-output
    )

set_target_properties(test_solver
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
