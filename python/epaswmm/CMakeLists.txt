# Description: CMake configuration for epaswmm python library
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

# Add Cython targets for epaswmm
add_cython_target(_epaswmm _epaswmm.pyx CXX PY3)

# Create Python extension modules
add_library(_epaswmm MODULE ${_epaswmm})

set_target_properties(_epaswmm
    PROPERTIES
        VERSION ${LIBRARY_VERSION}
        SOVERSION ${LIBRARY_SOVERSION}
)

python_extension_module(_epaswmm)

# Install the epaswmm module
install(TARGETS _epaswmm LIBRARY DESTINATION epaswmm)

# Include the current source directory
target_include_directories(
    _epaswmm
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add subdirectories
add_subdirectory(solver)
add_subdirectory(output)