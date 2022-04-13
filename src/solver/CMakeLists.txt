#
# CMakeLists.txt - CMake configuration file for swmm-solver/library
#
# Created: Jul 11, 2019
# Updated: May  19, 2021
#
# Author: Michael E. Tryby
#         US EPA ORD/CESER
#


find_package(OpenMP
    OPTIONAL_COMPONENTS
        C
)

# Generate version header
include(../../extern/version.cmake)


# configure file groups
set(SWMM_PUBLIC_HEADERS
    include/swmm5.h
    include/toolkit.h
    include/toolkit_enums.h
    include/toolkit_structs.h
)

file(GLOB
    SWMM_SOURCES
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h
)


if(BUILD_DEF)
    # Builds library with def file interface for backward compatibility
    set_source_files_properties(${PROJECT_SOURCE_DIR}/bindings/swmm5.def
        PROPERTIES_HEADER_FILE_ONLY TRUE
    )

    add_library(swmm5        
            ${SWMM_SOURCES}
            ${PROJECT_SOURCE_DIR/bindings/swmm5.def}
            $<TARGET_OBJECTS:shared_objs>
    )

else()
    # Performs standard library build
    add_library(swmm5        
            ${SWMM_SOURCES}
            $<TARGET_OBJECTS:shared_objs>
    )

endif()

# Sets MSVC compiler flags
target_compile_options(swmm5
    PUBLIC
        "$<$<C_COMPILER_ID:MSVC>:"
            "$<$<CONFIG:Release>:/GL>"
            "$<$<CONFIG:Release>:/fp:fast>"
            "$<$<CONFIG:Release>:/Zi>"
        ">"
        $<$<C_COMPILER_ID:AppleClang>:
            $<$<STREQUAL:"${CMAKE_GENERATOR}","Ninja">:-O3>
        >
)

target_link_options(swmm5
    PUBLIC
        "$<$<C_COMPILER_ID:MSVC>:"
            "$<$<CONFIG:Release>:/LTCG:incremental>"
        ">"
)

target_link_libraries(swmm5
    PUBLIC
        $<$<NOT:$<BOOL:$<C_COMPILER_ID:MSVC>>>:m>
        $<$<BOOL:${OpenMP_C_FOUND}>:OpenMP::OpenMP_C>
        $<$<BOOL:${OpenMP_AVAILABLE}>:omp>
)

target_include_directories(swmm5
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${INCLUDE_DIST}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

include(GenerateExportHeader)
generate_export_header(swmm5
    BASE_NAME toolkit  
    EXPORT_MACRO_NAME EXPORT_TOOLKIT
    EXPORT_FILE_NAME toolkit_export.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

file(COPY ${CMAKE_CURRENT_BINARY_DIR}/toolkit_export.h
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include
)


set_target_properties(swmm5
    PROPERTIES
        MACOSX_RPATH TRUE
        SKIP_BUILD_RPATH FALSE
        BUILD_WITH_INSTALL_RPATH FALSE
        INSTALL_RPATH "${PACKAGE_RPATH}"
        INSTALL_RPATH_USE_LINK_PATH TRUE
)


install(TARGETS swmm5 EXPORT swmm5Targets
    RUNTIME    DESTINATION "${TOOL_DIST}"
    LIBRARY    DESTINATION "${LIBRARY_DIST}"
    ARCHIVE    DESTINATION "${LIBRARY_DIST}"
    FRAMEWORK  DESTINATION "${TOOL_DIST}"
)

install(
    EXPORT
        swmm5Targets
    DESTINATION
        "${CONFIG_DIST}"
    FILE
        swmm5-config.cmake
)

install(
    FILES
        ${SWMM_PUBLIC_HEADERS}
    DESTINATION
        "${INCLUDE_DIST}"
)


# copy swmm5 to build tree for testing
add_custom_command(TARGET swmm5 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:swmm5>
        ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/$<TARGET_FILE_NAME:swmm5>
)
