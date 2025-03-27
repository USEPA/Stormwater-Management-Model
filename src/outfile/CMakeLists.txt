#
# CMakeLists.txt - CMake configuration file for swmm/outfile
#
# Created:  July 11, 2019
# Modified: February 26, 2025
#
# Author:   Michael E. Tryby
#           US EPA ORD/CESER
#
# Modified: Caleb Buahin
#           US EPA ORD/CESER
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

if (APPLE)
    set_target_properties(
        swmm-output
        PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

target_include_directories(swmm-output
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

include(GenerateExportHeader)
generate_export_header(swmm-output
    BASE_NAME swmm_output
    EXPORT_MACRO_NAME EXPORT_SWMM_OUTPUT_API
    EXPORT_FILE_NAME swmm_output_export.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

file(COPY ${CMAKE_CURRENT_BINARY_DIR}/swmm_output_export.h
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(APPLE)
    set(LIB_ROOT "@loader_path")
else()
    set(LIB_ROOT "$ORIGIN")
endif()

set_target_properties(swmm-output
    PROPERTIES
        MACOSX_RPATH TRUE
        SKIP_BUILD_RPATH FALSE
        BUILD_WITH_INSTALL_RPATH FALSE
        INSTALL_RPATH "${LIB_ROOT};${PACKAGE_RPATH};"
        INSTALL_RPATH_USE_LINK_PATH TRUE
)

install(
    FILES 
        ${SWMM_OUT_PUBLIC_HEADERS} 
    DESTINATION 
        include
)

install(
    TARGETS
        swmm-output
    EXPORT
        swmmTargets
)

# copy epanet-output to build tree for testing
if(BUILD_TESTS)
    add_custom_command(TARGET swmm-output POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:swmm-output>
            ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/$<TARGET_FILE_NAME:swmm-output>
        )
endif()