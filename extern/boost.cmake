#
# CMakeLists.txt - CMake configuration file for swmm-solver/extern
#
# Created: March 16, 2020
# Updated: May 21, 2020
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


# Environment variable "BOOST_ROOT_X_XX_X" points to local install location
if (DEFINED ENV{BOOST_ROOT_1_72_0})
    set(BOOST_ROOT $ENV{BOOST_ROOT_1_72_0})

elseif(DEFINED ENV{BOOST_ROOT_1_67_0})
    set(BOOST_ROOT $ENV{BOOST_ROOT_1_67_0})

endif()


find_package(Boost 1.67.0
    COMPONENTS
        unit_test_framework
    )

include_directories (${Boost_INCLUDE_DIRS})
