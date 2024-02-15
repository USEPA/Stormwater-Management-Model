#
# CMakeLists.txt - CMake configuration file for tests/solver
#
# Created: Feb 02, 2024
# Updated: Feb 02, 2024
#
#

add_executable(test_solver
    test_solver_hotstart.cpp
#    test_solver_api.cpp
#    test_solver_errors.cpp 
#   test_solver_shapes.cpp
    )

target_include_directories(
    test_solver
    PUBLIC ../../solver/include
    PUBLIC ../../outfile/include
    )

target_link_libraries(
    test_solver
    ${Boost_LIBRARIES}
    swmm5
    swmm-output
    )

set_target_properties(
    test_solver
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)