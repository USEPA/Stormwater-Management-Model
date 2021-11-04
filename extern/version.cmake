#
# version.cmake - generate SWMM version information header
#
# Created: November 2, 2021
# Updated:
#
# Author:  see CONTRIBUTORS
#
# Usage:
#   GEN_VER_HEADER=ON -- overwrites version.h file with current info 
#


option(GEN_VER_HEADER "Automatically update version header" ON)

if(GEN_VER_HEADER)

    # Get the latest commit hash for the working branch
    execute_process(
        COMMAND 
            git rev-parse HEAD
        WORKING_DIRECTORY 
            ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE 
            GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Generate a build id
    string(
        TIMESTAMP BUILD_ID UTC
    )

    # Configure the version header
    configure_file(
        ../../extern/version.h.in version.h
    )

    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/version.h
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/
    )

endif()
