# CMake configuration for epaswmm python library
#
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

# Include the CMake module for building Python extensions
include(UseCython)

add_cython_target(_output _output.pyx LANGUAGE CXX PY3)

# Add Cython target
add_library(_output MODULE ${_output})

# Add library
target_link_libraries(_output swmm-output)

# Specify that this is a Python extension module
python_extension_module(_output)

# Install the target
install(TARGETS _output LIBRARY DESTINATION epaswmm/output)

# Include directories
target_include_directories(
    _output PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<TARGET_PROPERTY:swmm-output,INCLUDE_DIRECTORIES>
)
