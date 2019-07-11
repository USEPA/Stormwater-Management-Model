#
# CMakeLists.txt - CMake configuration file for swmm-solver/library
#
# Created: July 11, 2019
#
# Modified by: Michael E. Tryby
#              US EPA ORD/NRMRL
#

# configure file groups
set(SWMM_PUBLIC_HEADERS
    ${PROJECT_SOURCE_DIR}/include/swmm5.h
    )

file(GLOB SWMM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

add_library(swmm5
    SHARED
        ${SWMM_SOURCES}
    )
target_include_directories(swmm5
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${INCLUDE_DIST}>
    )

install(TARGETS swmm5 EXPORT swmm5Targets
    RUNTIME    DESTINATION "${TOOL_DIST}"
    LIBRARY    DESTINATION "${TOOL_DIST}"
    ARCHIVE    DESTINATION "${LIBRARY_DIST}"
    FRAMEWORK  DESTINATION "${TOOL_DIST}"
    )

install(FILES ${SWMM_PUBLIC_HEADERS} DESTINATION "${INCLUDE_DIST}")
