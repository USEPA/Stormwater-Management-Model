#
# CMakeLists.txt - CMake configuration file for swmm-solver/src/shared
#
# Created: October 8, 2020
#
# Author: Michael E. Tryby
#         US EPA - ORD/CESER
#


set(SHARED_SOURCES
    csio_helper.c
)

set(SHARED_HEADERS
    csio_helper.h
)

add_library(shared_objs
    OBJECT
        ${SHARED_SOURCES}
)
