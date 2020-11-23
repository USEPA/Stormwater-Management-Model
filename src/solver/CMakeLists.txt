#
# CMakeLists.txt - CMake configuration file for swmm-solver/library
#
# Created: Jul 11, 2019
# Updated: Mar  5, 2020
#
# Author: Michael E. Tryby
#         US EPA ORD/CESER
#


find_package(OpenMP REQUIRED)


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
    # Build library with def file interface for backward compatibility
    set_source_files_properties(${PROJECT_SOURCE_DIR}/bindings/swmm5.def
        PROPERTIES_HEADER_FILE_ONLY TRUE
    )

    add_library(swmm5
        SHARED
            ${SWMM_SOURCES}
            ${PROJECT_SOURCE_DIR/bindings/swmm5.def}
            $<TARGET_OBJECTS:shared_objs>
    )

else()

    add_library(swmm5
        SHARED
            ${SWMM_SOURCES}
            $<TARGET_OBJECTS:shared_objs>
    )

endif()

target_compile_options(swmm5
    PUBLIC
        "$<$<C_COMPILER_ID:MSVC>:"
            "$<$<CONFIG:Release>:/GL>"
            "$<$<CONFIG:Release>:/fp:fast>"
            "$<$<CONFIG:Release>:/Zi>"
        ">"
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
        $<$<BOOL:OpenMP_C_FOUND>:OpenMP::OpenMP_C>
)

target_include_directories(swmm5
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${INCLUDE_DIST}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
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


# TODO: Figure out why this doesn't work for package target
# When building on MacOS relocate libomp to install package
if(APPLE)
    get_filename_component(EXTERN_LIB_PATH ${OpenMP_libomp_LIBRARY} REALPATH)

    install(
        FILES
            ${EXTERN_LIB_PATH}
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/extern
    )

    # Pass the var up the project tree
    set(EXTERN_LIB_PATH ${EXTERN_LIB_PATH} PARENT_SCOPE)
endif()


# copy swmm5 to build tree for testing
add_custom_command(TARGET swmm5 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:swmm5>
        ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/$<TARGET_FILE_NAME:swmm5>
)
