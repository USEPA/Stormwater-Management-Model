#
# CMakeLists.txt - CMake configuration file for swmm/tests
#
# Created: February 23, 2018
# Author:  Constantin Savtchenko
# Ref: http://neyasystems.com/an-engineers-guide-to-unit-testing-cmake-and-boost-unit-tests/
#
# Modified by: Michael E. Tryby
#              US EPA ORD/NRMRL
# 

cmake_minimum_required (VERSION 3.0)


#Setup CMake to run tests
enable_testing()


# Sets for output directory for executables and libraries. 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
endif(UNIX)


#Prep ourselves for compiling boost
find_package(Boost REQUIRED)
include_directories (${Boost_INCLUDE_DIRS})


# Test files are a separate source directory called tests
file(GLOB TEST_SRCS . test_*.cpp)


#Run through each source
foreach(testSrc ${TEST_SRCS})
        #Extract the filename without an extension (NAME_WE)
        get_filename_component(testName ${testSrc} NAME_WE)

        #Add compile target
        add_executable(${testName} ${testSrc})

        #link to Boost libraries AND your targets and dependencies
        target_link_libraries(${testName} ${Boost_LIBRARIES} swmm5 swmm-output)

        #Finally add it to test execution - 
        #Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName}
                 COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${testName}  
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)
endforeach(testSrc)
