#
# CMakeLists.txt - CMake configuration file for swmm-solver/extern
#
# Created: March 16, 2020
# Updated: Oct 19, 2022
#
# Author: Michael E. Tryby
#         US EPA - ORD/CESER
#


if(WIN32)
    set(Boost_USE_STATIC_LIBS       ON)
else()
    set(Boost_USE_STATIC_LIBS       OFF)
    add_definitions(-DBOOST_ALL_DYN_LINK)
endif()


find_package(
  Boost
    COMPONENTS
        unit_test_framework
    )

include_directories(
    ${Boost_INCLUDE_DIRS}
)
