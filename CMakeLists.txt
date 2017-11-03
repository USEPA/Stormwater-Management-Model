# SWMM LIBRARY
cmake_minimum_required (VERSION 3.0)

project(SWMM)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(UNIX)
    set(CMAKE_C_FLAGS "-std=c99 -fopenmp")
    set(CMAKE_CXX_FLAGS "-std=c++11 -fopenmp -Wno-write-strings")
    # add_definitions(-std=c99)
endif(UNIX)

FIND_PACKAGE(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif(OPENMP_FOUND)

if(APPLE)
    set(CMAKE_INSTALL_NAME_DIR @executable_path)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif(APPLE)

# includes and source files
include_directories(src)
file(GLOB SWMM_SOURCES src/*.c)

# library
add_library(swmm5 SHARED ${SWMM_SOURCES})
target_link_libraries(swmm5)

# executable
add_executable(run-swmm ${SWMM_SOURCES})
target_compile_definitions(run-swmm PRIVATE CLE=TRUE)

if(WIN32)
    target_link_libraries(run-swmm)
else(WIN32)
    target_link_libraries(run-swmm m pthread)
endif(WIN32)

# install
install(TARGETS run-swmm DESTINATION bin)
install(TARGETS swmm5 DESTINATION lib)
install(FILES src/swmm5.h DESTINATION include)
