
#
# CMakeLists.txt - CMake configuration file for tests/solver
#
# Date Created: July 7, 2020
#
# Author: Michael E. Tryby
#         US EPA - ORD/CESER
#


# LID Test Module
set(lid_test_srcs
    test_lid_results.cpp
    test_lid.cpp
)

add_executable(test_lid
    ${lid_test_srcs}
)

target_link_libraries(test_lid
    ${Boost_LIBRARIES}
    swmm5
)

set_target_properties(test_lid
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)


# Toolkit Test Module
set(solver_test_srcs
    test_canonical.cpp
    test_gage.cpp
    test_pollut.cpp
    test_toolkit.cpp
    test_solver.cpp
    test_stats.cpp
    # ADD NEW TEST SUITES TO EXISTING TOOLKIT TEST MODULE
)

add_executable(test_solver
    ${solver_test_srcs}
)

target_compile_features(test_solver
    PUBLIC
      cxx_generalized_initializers
)
  
target_link_libraries(test_solver
    ${Boost_LIBRARIES}
    swmm5
)

set_target_properties(test_solver
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
