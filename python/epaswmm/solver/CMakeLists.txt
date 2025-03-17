# CMake configuration for epaswmm library
#
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19
#

# Add Cython target
add_cython_target(_solver _solver.pyx CXX PY3)

# Add library
add_library(_solver MODULE ${_solver})

# Link to SWMM and Python libraries
target_link_libraries(
    _solver
    swmm::swmm5
)

# Specify that this is a Python extension module
python_extension_module(_solver)

# Install the target
install(TARGETS _solver LIBRARY DESTINATION epaswmm/solver)

# Include directories
target_include_directories(
    _solver
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
        # ${Python3_INCLUDE_DIRS}
)