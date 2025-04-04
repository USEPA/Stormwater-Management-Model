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
    swmm5
)

# Specify that this is a Python extension module
python_extension_module(_solver)

if(APPLE)
    set(INSTALL_LIB_ROOT "@loader_path;@rpath;@loader_path/../../../..;${SOLVER_BUILD_DIR}")
    set(BUILD_LIB_ROOT "@loader_path;@rpath;${SOLVER_BUILD_DIR}")
elseif(UNIX)
    set(INSTALL_LIB_ROOT "$ORIGIN;$ORIGIN/../../../..;${SOLVER_BUILD_DIR}")
    set(BUILD_LIB_ROOT "$ORIGIN;${SOLVER_BUILD_DIR}")
endif()

# Set up rpath for runswmm inside install package
set_target_properties(_solver
    PROPERTIES
        BUILD_RPATH "${BUILD_LIB_ROOT}"
        INSTALL_RPATH "${INSTALL_LIB_ROOT}"
        BUILD_WITH_INSTALL_RPATH True
)

# Install the target
install(TARGETS _solver LIBRARY DESTINATION epaswmm/solver)



# Include directories
target_include_directories(
    _solver
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

