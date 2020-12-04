#
# CMakeLists.txt - CMake configuration file for swmm-solver/run
#
# Created: July 11, 2019
#
# Modified by: Michael E. Tryby
#              US EPA ORD/NRMRL
#


# Location of libs runswmm uses at runtime on MacOS/Linux
if(APPLE)
    set(RPATH_ROOT "@loader_path")
else()
    set(RPATH_ROOT "$ORIGIN")
endif()

set(PACKAGE_RPATH "${RPATH_ROOT}/../lib;${RPATH_ROOT}/../extern")


# Creates the EPANET command line executable
add_executable(runswmm
    main.c
    timer.c
)

target_include_directories(runswmm
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    PUBLIC
        include
)

target_link_libraries(runswmm
    LINK_PUBLIC
        swmm5
)

# Set up rpath for runswmm inside install package
set_target_properties(runswmm
    PROPERTIES
        MACOSX_RPATH TRUE
        SKIP_BUILD_RPATH FALSE
        BUILD_WITH_INSTALL_RPATH FALSE
        INSTALL_RPATH "${PACKAGE_RPATH}"
        INSTALL_RPATH_USE_LINK_PATH TRUE
)

install(TARGETS runswmm
    DESTINATION "${TOOL_DIST}"
)


# copy runswmm to build tree for testing
add_custom_command(TARGET runswmm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:runswmm>
        ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/$<TARGET_FILE_NAME:runswmm>
)
