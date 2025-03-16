# CMake configuration for epaswmm library
#
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19
#

# Create Python extension modules
add_custom_command(
    OUTPUT solver.c
    COMMENT
        "Making ${CMAKE_CURRENT_BINARY_DIR}/solver.c from ${CMAKE_CURRENT_SOURCE_DIR}/_solver.pyx"
    COMMAND Python::Interpreter -m cython
          "${CMAKE_CURRENT_SOURCE_DIR}/_solver.pyx" --output-file solver.c
    DEPENDS _solver.pyx
    VERBATIM
)   

# Add library target for solver
python_add_library(solver MODULE WITH_SOABI solver.c)

# Link to SWMM and Python libraries
target_link_libraries(
    solver
    PUBLIC
        swmm5
    )

# Install the target
install(TARGETS solver LIBRARY DESTINATION epaswmm/solver)

# Include directories
target_include_directories(
    solver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<TARGET_PROPERTY:swmm5,INCLUDE_DIRECTORIES>
    ${Python3_INCLUDE_DIRS}
)