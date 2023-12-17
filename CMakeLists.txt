cmake_minimum_required (VERSION 3.21)

set(CMAKE_VERBOSE_MAKEFILE True)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True) 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin.dbg)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin.rel)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/bin.relwithdbg)

project(noi_est)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_subdirectory(src)

install(FILES pics/cat.png DESTINATION bin)
install(FILES pics/grid.png DESTINATION bin)
install(TARGETS test_app DESTINATION bin)

find_package(Doxygen)
option(BUILD_DOC "Build documentation" ON)

add_custom_target(install_${PROJECT_NAME}
        ${CMAKE_COMMAND} -DCMAKE_INSTALL_CONFIG_NAME=${CMAKE_BUILD_TYPE}
        -P cmake_install.cmake
        )

install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/
        DESTINATION bin/
        FILES_MATCHING 
        PATTERN "*.dll"
        PATTERN "*.so")

if (DOXYGEN_FOUND)
# set input and output files
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# request to configure the file
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
message("Doxygen build started")

add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc DESTINATION .)
else (DOXYGEN_FOUND)
   message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)