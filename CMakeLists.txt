# SWMM LIBRARY
cmake_minimum_required (VERSION 3.0.2)


project(SWMM)

# Adds a subdirectory to the build. Paths are relative to the directory 
# containing root CMakeLists.txt file. 
add_subdirectory(tools/swmm-output)

IF (BUILD_TESTS)
    add_subdirectory(tests)
ENDIF (BUILD_TESTS)


# Sets for output directory for executables and libraries. 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Sets the position independent code property for all targets. 
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Sets compiler flags when building with Visual Studio. 
if(MSVC)
    set(CMAKE_C_FLAGS "/fp:fast /fp:except- /EHsc ")
    set(CMAKE_C_FLAGS_DEBUG "/ZI /Od /Gm  /RTC1")
    set(CMAKE_C_FLAGS_RELEASE "/Zi /O2 /Oi /GL /MT /Gy")

    set(CMAKE_SHARED_LINKER_FLAGS "/LTCG")
    set(CMAKE_EXE_LINKER_FLAGS "/LTCG")
endif(MSVC)


# Set compiler flags when building on UNIX. 
if(UNIX AND NOT APPLE)
	set(CMAKE_C_FLAGS "-fopenmp")
	set(CMAKE_CXX_FLAGS "-std=c++11 -fopenmp -Wno-write-strings")
    # add_definitions(-std=c99)
endif(UNIX AND NOT APPLE)

# Set compiler flags when building on Apple 
# adapted from https://github.com/CLIUtils/cmake/blob/master/PatchOpenMPApple.cmake
if(APPLE)
	find_program(BREW NAMES brew)
    if(BREW)
    	execute_process(COMMAND ${BREW} ls libomp RESULT_VARIABLE BREW_RESULT_CODE OUTPUT_QUIET ERROR_QUIET)
    	if(BREW_RESULT_CODE)
    		message(STATUS "This program supports OpenMP on Mac through Brew. Please run \"brew install libomp\"")
		else()
    		execute_process(COMMAND ${BREW} --prefix libomp OUTPUT_VARIABLE BREW_LIBOMP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
       	 	set(OpenMP_C_FLAGS "-Ofast -Xpreprocessor -fopenmp -lomp -Wno-unused-command-line-argument")
        	set(OpenMP_C_LIB_NAMES "omp")
			set(OpenMP_CXX_FLAGS "-Ofast -Xpreprocessor -fopenmp -lomp -Wno-unused-command-line-argument")
        	set(OpenMP_CXX_LIB_NAMES "omp")
        	set(OpenMP_omp_LIBRARY "${BREW_LIBOMP_PREFIX}/lib/libomp.dylib")
        	include_directories("${BREW_LIBOMP_PREFIX}/include")
		    link_directories("${BREW_LIBOMP_PREFIX}/lib/")
        	message(STATUS "Using Homebrew libomp from ${BREW_LIBOMP_PREFIX}")
		endif()
    else()
    	message(STATUS "This program supports OpenMP on Mac through Homebrew, installing Homebrew recommmended https://brew.sh")
    endif()   
endif(APPLE)


# Loads settings for OpenMP and append any OpenMP compiler flags. 
FIND_PACKAGE(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif(OPENMP_FOUND)


# Set up file groups for exe and lib targets
file(GLOB SWMM_SOURCES RELATIVE ${PROJECT_SOURCE_DIR} src/*.c)
list(REMOVE_ITEM SWMM_SOURCES "src/swmm5.c")
set(SWMM_API_HEADERS include/swmm5.h; include/toolkitAPI.h) 
include_directories(include)
add_library(swmm_objs OBJECT ${SWMM_SOURCES})

# Creates swmm5 shared library
add_library(swmm5 SHARED src/swmm5.c $<TARGET_OBJECTS:swmm_objs>)

# Add includes and libraries to use when linking the swmm5 library
target_include_directories(swmm5 PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Create export headers for use when linking with MSVC
#include(GenerateExportHeader)
#GENERATE_EXPORT_HEADER(swmm5
#    BASE_NAME swmm5
#    EXPORT_MACRO_NAME DLLEXPORT
#    EXPORT_FILE_NAME swmm5_exports.h
#    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)


# Creates the swmm5 command line executable
add_executable(run-swmm src/swmm5.c $<TARGET_OBJECTS:swmm_objs> ${SWMM_API_HEADERS})
target_compile_definitions(run-swmm PRIVATE CLE=TRUE)
if(NOT WIN32)
    target_link_libraries(run-swmm PUBLIC m pthread)
endif(NOT WIN32)
