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

if(APPLE)
    set(INSTALL_LIB_ROOT "@loader_path;@rpath;@loader_path/../../../..;${OUTFILE_BUILD_DIR}")
    set(BUILD_LIB_ROOT "@loader_path;@rpath;${OUTFILE_BUILD_DIR}")
elseif(UNIX)
    set(INSTALL_LIB_ROOT "$ORIGIN;$ORIGIN/../../../..;${OUTFILE_BUILD_DIR}")
    set(BUILD_LIB_ROOT "$ORIGIN;${OUTFILE_BUILD_DIR}")
endif()

# Set up rpath for runswmm inside install package
set_target_properties(_output
    PROPERTIES
        BUILD_RPATH "${BUILD_LIB_ROOT}"
        INSTALL_RPATH "${INSTALL_LIB_ROOT}"
        BUILD_WITH_INSTALL_RPATH True
)

# Install the target
install(TARGETS _output LIBRARY DESTINATION epaswmm/output)

# Include directories
target_include_directories(
    _output
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)