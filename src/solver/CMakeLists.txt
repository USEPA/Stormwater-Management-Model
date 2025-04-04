#
# CMakeLists.txt - CMake configuration file for swmm-solver/library
#
# Created: Jul 11, 2019
# Updated: Feb 26, 2025
#
# Author: Michael E. Tryby
#         US EPA ORD/CESER
#         
#         Caleb Buahin (Modified)
#         US EPA ORD/CESER 
#

if(APPLE)
    find_package(OpenMP)
    if(NOT OpenMP_FOUND)
        # libomp 15.0+ from brew is keg-only, so have to search in other locations.
        # See https://github.com/Homebrew/homebrew-core/issues/112107#issuecomment-1278042927.
        execute_process(COMMAND brew --prefix libomp
                    OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
        set(OpenMP_C_LIB_NAMES omp)
        set(OpenMP_CXX_LIB_NAMES omp)
        set(OpenMP_omp_LIBRARY ${HOMEBREW_LIBOMP_PREFIX}/lib/libomp.dylib)
        find_package(OpenMP REQUIRED)
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    find_package(OpenMP REQUIRED)
endif()


# configure file groups
set(SWMM_PUBLIC_HEADERS
    include/swmm5_export.h
    include/swmm5.h
)

file(GLOB
    SWMM_SOURCES
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h
)

if(BUILD_DEF)
    # Build library with def file interface for backward compatibility
    set_source_files_properties(${PROJECT_SOURCE_DIR}/bindings/swmm5.def
        PROPERTIES_HEADER_FILE_ONLY TRUE
    )

    add_library(swmm5
        SHARED
            ${SWMM_SOURCES}
            ${PROJECT_SOURCE_DIR}/bindings/swmm5.def
    )

else()

    add_library(swmm5
        SHARED
            ${SWMM_SOURCES}
    )

endif()

if (APPLE)
    set_target_properties(
        swmm5
        PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()


configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h
)


# Define preprocessor macros for different platforms
if(WIN32)
    add_compile_definitions(OS_WINDOWS)
elseif(APPLE)
    add_compile_definitions(OS_MAC)
elseif(UNIX)
    add_compile_definitions(OS_UNIX)
endif()

include(GenerateExportHeader)
generate_export_header(swmm5
    BASE_NAME swmm5
    EXPORT_MACRO_NAME EXPORT_SWMM_SOLVER_API
    EXPORT_FILE_NAME swmm5_export.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

file(COPY ${CMAKE_CURRENT_BINARY_DIR}/swmm5_export.h
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    swmm5
        PRIVATE
    $<$<NOT:$<BOOL:$<C_COMPILER_ID:MSVC>>>:m>
    OpenMP::OpenMP_C
)

# Set public include directories for both build and install time
target_include_directories(
    swmm5
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE 
            ${OpenMP_C_INCLUDE_DIRS}
)

if(APPLE)
    set(LIB_ROOT "@loader_path/../lib;@rpath")
else()
    set(LIB_ROOT "$ORIGIN/../lib;$ORIGIN")
endif()

set_target_properties(swmm5
    PROPERTIES
        INSTALL_RPATH "${LIB_ROOT}"
        BUILD_RPATH "${LIB_ROOT}"
        BUILD_WITH_INSTALL_RPATH TRUE
)

install(
    FILES 
        ${SWMM_PUBLIC_HEADERS} 
    DESTINATION 
        include
)

install(
    TARGETS
        swmm5
    EXPORT
        swmmTargets
)

# copy swmm5 to build tree for testing
add_custom_command(TARGET swmm5 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:swmm5>
        ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/$<TARGET_FILE_NAME:swmm5>
)
