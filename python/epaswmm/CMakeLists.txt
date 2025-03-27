# Description: CMake configuration for epaswmm python library
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

# Add Cython targets for epaswmm
add_cython_target(_epaswmm _epaswmm.pyx CXX PY3)

# Create Python extension modules
add_library(_epaswmm MODULE ${_epaswmm})

python_extension_module(_epaswmm)


# Install the epaswmm module
install(TARGETS _epaswmm LIBRARY DESTINATION epaswmm)

# Include the current source directory
target_include_directories(
    _epaswmm
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Location of package at runtime on MacOS/Linux
if(APPLE)
    set(LIB_ROOT "@loader_path")
else()
    set(LIB_ROOT "$ORIGIN")
endif()

# Set up rpath on MacOS and Linux
set(PACKAGE_RPATH
    "${LIB_ROOT}/../../epaswmm/"
)


# Add subdirectories
add_subdirectory(solver)
add_subdirectory(output)