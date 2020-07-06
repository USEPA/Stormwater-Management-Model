#
# CMakeLists.txt - CMake configuration file for swmm-solver/run
#
# Created: July 11, 2019
#
# Modified by: Michael E. Tryby
#              US EPA ORD/NRMRL
#


# Creates the EPANET command line executable
add_executable(runswmm
    main.c
)

target_include_directories(runswmm
    PUBLIC
        include
)

target_link_libraries(runswmm
    LINK_PUBLIC
        swmm5
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
