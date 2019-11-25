#
# CMakeLists.txt - CMake configuration file for swmm-solver/library
#
# Created: July 11, 2019
# Modified: Nov 25, 2019
#
# Author: Michael E. Tryby
#         US EPA ORD/CESER
#


find_package(OpenMP)


# configure file groups
set(SWMM_PUBLIC_HEADERS
    include/swmm5.h
    )

file(GLOB SWMM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

add_library(swmm5
    SHARED
        ${SWMM_SOURCES}
    )

if(OpenMP_C_FOUND)
    target_link_libraries(swmm5
        PUBLIC
            OpenMP::OpenMP_C
        )
endif()

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

install(FILES ${SWMM_PUBLIC_HEADERS} DESTINATION "${INCLUDE_DIST}")


# copy swmm5 to build tree for testing
add_custom_command(TARGET swmm5 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:swmm5>
        ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/$<TARGET_FILE_NAME:swmm5>
    )
