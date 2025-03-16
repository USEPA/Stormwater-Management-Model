# CMake configuration for epaswmm python library
#
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19


# Create Python extension modules
add_custom_command(
    OUTPUT output.c
    COMMENT
        "Making ${CMAKE_CURRENT_BINARY_DIR}/output.c from ${CMAKE_CURRENT_SOURCE_DIR}/_output.pyx"
    COMMAND Python::Interpreter -m cython
          "${CMAKE_CURRENT_SOURCE_DIR}/_output.pyx" --output-file output.c
    DEPENDS _output.pyx
    VERBATIM
)   

# Add library target for output
python_add_library(output MODULE WITH_SOABI output.c)

# Link to SWMM and Python libraries
target_link_libraries(
    output
    PUBLIC
        swmm_output
    )


# Install the target
install(TARGETS output LIBRARY DESTINATION epaswmm/output)

# Include directories
target_include_directories(
    output PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<TARGET_PROPERTY:swmm_output,INCLUDE_DIRECTORIES>
)
