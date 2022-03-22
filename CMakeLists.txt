cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0048 NEW)
project(db2_programming_project VERSION 2.0 LANGUAGES CXX)

#download and include cmake package manager
set(CPM_DOWNLOAD_VERSION 0.34.0)

if(CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
            https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
            ${CPM_DOWNLOAD_LOCATION}
            )
endif()

include(${CPM_DOWNLOAD_LOCATION})

#add dependencies through cmake package manager

#unittests
CPMAddPackage("gh:catchorg/Catch2@2.13.8")

#serialization
CPMAddPackage(
        NAME cereal
        VERSION 1.3.2
        GITHUB_REPOSITORY USCiLab/cereal
        OPTIONS "SKIP_PORTABILITY_TEST ON" "JUST_INSTALL_CEREAL ON"
)

#documentation target
option(BUILD_DOC "Build documentation" ON)
if (BUILD_DOC)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target(doc_doxygen ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM)
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif()

include_directories(include)

configure_file(include/config.hpp.in include/config.hpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
#add source
add_subdirectory(src)

#TODO: custom release command with archiving
#rm -rf Release db2_programming_project_SoSe2015 db2_programming_project_SoSe2015.tgz
#svn export . Release/
#rm -rf Release/.gitignore
#mv Release db2_programming_project_SoSe2015
#tar -cvzf db2_programming_project_SoSe2015.tgz db2_programming_project_SoSe2015
