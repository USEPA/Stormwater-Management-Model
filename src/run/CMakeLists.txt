#
# CMakeLists.txt - CMake configuration file for swmm-solver/run
#
# Created: July 11, 2019
#
# Modified by: Michael E. Tryby
#              US EPA ORD/NRMRL
#


# Creates the EPANET command line executable
add_executable(run_swmm
    main.c
    )
target_include_directories(run_swmm
    PUBLIC
        include
    )
target_link_libraries(run_swmm
    LINK_PUBLIC
        swmm5
    )

install(TARGETS run_swmm DESTINATION "${TOOL_DIST}")
