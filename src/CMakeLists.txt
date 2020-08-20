#
# CMakeLists.txt - CMake configuration file for swmm-solver/library
#
# Created: Jul 11, 2019
# Updated: Mar  5, 2020
#
# Author: Michael E. Tryby
#         US EPA ORD/CESER
#


find_package(OpenMP)

find_package(OpenMP REQUIRED) 
if(NOT TARGET OpenMP::OpenMP_CXX) 
    add_library(OpenMP_TARGET INTERFACE) 
    add_library(OpenMP::OpenMP_CXX ALIAS OpenMP_TARGET) 
    target_compile_options(OpenMP_TARGET INTERFACE ${OpenMP_CXX_FLAGS}) 
    find_package(Threads REQUIRED) 
    target_link_libraries(OpenMP_TARGET INTERFACE Threads::Threads) 
    target_link_libraries(OpenMP_TARGET INTERFACE ${OpenMP_CXX_FLAGS}) 
endif()

# configure file groups
set(SWMM_PUBLIC_HEADERS
    include/swmm5.h
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
    )

else()

    add_library(swmm5
        SHARED
            ${SWMM_SOURCES}
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
)

install(TARGETS swmm5 EXPORT swmm5Targets
    RUNTIME    DESTINATION "${TOOL_DIST}"
    LIBRARY    DESTINATION "${TOOL_DIST}"
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
