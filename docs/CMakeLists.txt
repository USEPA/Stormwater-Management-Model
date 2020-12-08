#
# CMakeLists.txt - CMake configuration file for swmm/docs
#
# Date Created:  Dec 4, 2020
#
# Author: Michael E. Tryby
#         US EPA - ORD/CESER
#
# Purpose: Create build target for documentation
#


find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen not found")
endif()


set(DOXY_SOURCES
    ./
    ../src/outfile/include/swmm_output.h
    ../src/outfile/include/swmm_output_enums.h

    ../src/solver/include/swmm5.h
    ../src/solver/include/toolkit.h
    ../src/solver/include/toolkit_enums.h
    ../src/solver/include/toolkit_structs.h

)


set(DOXYGEN_PROJECT_NAME "OWA-SWMM Toolkit")
set(DOXYGEN_PROJECT_NUMBER "5.2")
set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C "YES")
set(DOXYGEN_HIDE_SCOPE_NAMES "YES")
set(DOXYGEN_SHOW_INCLUDE_FILES "NO")
set(DOXYGEN_SHOW_USED_FILES  "NO")
set(DOXYGEN_LAYOUT_FILE "DoxygenLayout.xml")
set(DOXYGEN_IMAGE_PATH ".")
set(DOXYGEN_HTML_FOOTER "newfooter.html")
set(DOXYGEN_HTML_EXTRA_STYLESHEET "extrastylesheet.css")
set(DOXYGEN_DISABLE_INDEX "YES")
set(DOXYGEN_GENERATE_TREEVIEW "YES")
set(DOXYGEN_ENUM_VALUES_PER_LINE "0")
set(DOXYGEN_MACRO_EXPANSION "YES")
set(DOXYGEN_PREDEFINED "__declspec(x)=")
set(DOXYGEN_CLASS_DIAGRAMS "NO")


doxygen_add_docs(
    doxygen
    ${DOXY_SOURCES}
)


install(
    DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION
        "./docs"
)
