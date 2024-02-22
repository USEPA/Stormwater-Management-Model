#
# CMakeLists.txt - CMake configuration file for tests/solver
#
# Created: Feb 02, 2024
# Updated: Feb 02, 2024
#
#


# Test solver api
add_executable(
    test_solver_api
        test_solver_api.cpp
)
     
target_include_directories(
    test_solver_api
    PUBLIC 
        ../
        ../../solver/include
        ../../outfile/include
    )

target_link_libraries(
    test_solver_api
        ${Boost_LIBRARIES}
        swmm5
        swmm-output 
 )

set_target_properties(
    test_solver_api
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Test solver errors
add_executable(
    test_solver_errors
        test_solver_errors.cpp 
)
     
target_include_directories(
    test_solver_errors
    PUBLIC 
        ../../solver/include
        ../../outfile/include
    )

target_link_libraries(
    test_solver_errors
        ${Boost_LIBRARIES}
        swmm5
        swmm-output 
 )

set_target_properties(
    test_solver_errors
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Test solver hotstart
add_executable(
    test_solver_hotstart
        test_solver_hotstart.cpp
)
     
target_include_directories(
    test_solver_hotstart
    PUBLIC 
        ../
        ../../solver/include
        ../../outfile/include
    )

target_link_libraries(
    test_solver_hotstart
        ${Boost_LIBRARIES}
        swmm5
        swmm-output 
 )

set_target_properties(
    test_solver_hotstart
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


# Test solver shapes
add_executable(
    test_solver_shapes
        test_solver_shapes.cpp
)
     
target_include_directories(
    test_solver_shapes
    PUBLIC 
        ../../solver/include
        ../../outfile/include
    )

target_link_libraries(
    test_solver_shapes
        ${Boost_LIBRARIES}
        swmm5
        swmm-output 
 )

set_target_properties( 
    test_solver_shapes
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)