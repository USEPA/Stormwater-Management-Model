# Description: CMake configuration for epaswmm python library
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

# Create Python extension modules
add_custom_command(
    OUTPUT epaswmm.c
    COMMENT
        "Making ${CMAKE_CURRENT_BINARY_DIR}/epaswmm.c from ${CMAKE_CURRENT_SOURCE_DIR}/_epaswmm.pyx"
    COMMAND Python::Interpreter -m cython
          "${CMAKE_CURRENT_SOURCE_DIR}/_epaswmm.pyx" --output-file epaswmm.c
    DEPENDS _epaswmm.pyx
    VERBATIM
)   

python_add_library(epaswmm MODULE WITH_SOABI epaswmm.c)

# Install the epaswmm module
install(TARGETS epaswmm LIBRARY DESTINATION epaswmm)

# Add subdirectories
add_subdirectory(solver)
add_subdirectory(output)