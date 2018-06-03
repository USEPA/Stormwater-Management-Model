#
# CMakeLists.txt - CMake configuration file for swmm-output library
#
# Created: February 27, 2018
# Author: Michael E. Tryby
#         US EPA ORD/NRMRL
# 

cmake_minimum_required (VERSION 3.0)


# Sets for output directory for executables and libraries. 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# configure file groups
set(SWMM_OUT_SOURCES src/swmm_output.c src/errormanager.c)
set(SWMM_OUT_HEADER src/swmm_output.h)


# the binary output file API        
add_library(swmm-output SHARED ${SWMM_OUT_SOURCES} ${SWMM_OUT_HEADERS})
target_include_directories(swmm-output PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


include(GenerateExportHeader)
generate_export_header(swmm-output
  BASE_NAME swmm_output
  EXPORT_MACRO_NAME DLLEXPORT
  EXPORT_FILE_NAME swmm_output_export.h
  STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)

file(COPY ${CMAKE_CURRENT_BINARY_DIR}/swmm_output_export.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include)
