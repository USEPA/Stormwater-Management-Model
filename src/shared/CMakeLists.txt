#
# CMakeLists.txt - CMake configuration file for epanet/src/shared
#
# Created: 8/26/2020
#
# Author: Michael E. Tryby
#         US EPA - ORD/NRMRL
#


set(SHARED_SOURCES
    cstr_helper.c
    )

set(SHARED_HEADERS
    cstr_helper.h
    )

add_library(shared_objs OBJECT ${SHARED_SOURCES})
