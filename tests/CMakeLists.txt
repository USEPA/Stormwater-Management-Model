#
# CMakeLists.txt - CMake configuration file for epanet/tests
#


#Prep ourselves for compiling with boost
if(WIN32)
    set(Boost_USE_STATIC_LIBS       ON)
else()
    set(Boost_USE_STATIC_LIBS       OFF)
    add_definitions(-DBOOST_ALL_DYN_LINK)
endif()

find_package(Boost COMPONENTS unit_test_framework system thread filesystem)
include_directories (${Boost_INCLUDE_DIRS})


add_subdirectory(outfile)


# Setting up tests to run from build tree
set(TEST_BIN_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>")


# ctest doesn't like tests added in subdirectories so adding them here
add_test(NAME test_output
    COMMAND "${TEST_BIN_DIRECTORY}/test_output"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/outfile/data
    )
