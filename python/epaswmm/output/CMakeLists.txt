# CMake configuration for epaswmm python library
#
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

add_cython_target(_output _output.pyx LANGUAGE CXX PY3)

# Add Cython target
add_library(_output MODULE ${_output})

# Add library
target_link_libraries(
    _output
     swmm-output
)

# Specify that this is a Python extension module
python_extension_module(_output)

# --- Set RPATH handling for macOS ---
# if(APPLE)
#     set_target_properties(
#         _solver PROPERTIES
#         BUILD_WITH_INSTALL_RPATH TRUE
#         INSTALL_RPATH "@loader_path/../../../../../lib"
#     )
# endif()

# Install the target
install(TARGETS _output LIBRARY DESTINATION epaswmm/output)

# Include directories
target_include_directories(
    _output
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)