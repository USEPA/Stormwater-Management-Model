#
# CMakeLists.txt - CMake configuration file for swmm/outfile
#
# Created: July 11, 2019
#
# Author: Michael E. Tryby
#         US EPA ORD/CESER
#

# configure file groups
set(SWMM_OUT_PUBLIC_HEADERS
    include/swmm_output.h
    include/swmm_output_enums.h
    include/swmm_output_export.h
)


# the binary output file API
add_library(swmm-output
    SHARED
        swmm_output.c
        errormanager.c
)

target_include_directories(swmm-output
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${INCLUDE_DIST}>
)

include(GenerateExportHeader)
generate_export_header(swmm-output
    BASE_NAME swmm_output
    EXPORT_MACRO_NAME EXPORT_OUT_API
    EXPORT_FILE_NAME swmm_output_export.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

file(COPY ${CMAKE_CURRENT_BINARY_DIR}/swmm_output_export.h
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include
)


install(TARGETS swmm-output
    EXPORT
        swmm-outputTargets
    RUNTIME
        DESTINATION
            "${TOOL_DIST}"
    LIBRARY
        DESTINATION
            "${LIBRARY_DIST}"
    ARCHIVE
        DESTINATION
            "${LIBRARY_DIST}"
    FRAMEWORK
        DESTINATION
            "${TOOL_DIST}"
)

install(
    EXPORT
        swmm-outputTargets
    DESTINATION
        "${CONFIG_DIST}"
    FILE
        swmm-output-config.cmake
)


install(FILES ${SWMM_OUT_PUBLIC_HEADERS}
    DESTINATION
        "${INCLUDE_DIST}"
)


# copy epanet-output to build tree for testing
if(BUILD_TESTS)
    add_custom_command(TARGET swmm-output POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:swmm-output>
            ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/$<TARGET_FILE_NAME:swmm-output>
    )
endif()
