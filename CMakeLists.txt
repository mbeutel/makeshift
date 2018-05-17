# makeshift C++ library
# Author: Moritz Beutel
# Distributed under the BSD 3-Clause License. See accompanying file licensing.txt for details.


cmake_minimum_required(VERSION 3.8)

# Disable in-source builds to avoid cluttering the source directory.
# If you attempted to build in the source directory and run into this error, you first need to clean up manually by deleting
# the CMakeFiles directory and the CMakeCache.txt file before proceeding.
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

include(cmake/DefaultFlags.cmake)
include(cmake/Makeshift.cmake)
#include(cmake/AddCatchTests.cmake)

# public header files
set(RELATIVE_INCLUDE_DIR "include/")
set(RELATIVE_INCLUDE_SUBDIR "makeshift/")

# project
# if possible, parse the library version from a header file
makeshift_parse_semantic_version(makeshift_VERSION MAKESHIFT "${RELATIVE_INCLUDE_DIR}${RELATIVE_INCLUDE_SUBDIR}version.hpp")
project(makeshift
    VERSION ${makeshift_VERSION}
    LANGUAGES CXX)

# Make CMake aware of the cmake folder for local FindXXX scripts.
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

# install directories
#set(INCLUDE_INSTALL_DIR "include/")
#set(LIB_INSTALL_DIR "lib/")
#set(SYSCONFIG_INSTALL_DIR "etc/${PROJECT_NAME}/")
set(MY_DOCS_INSTALL_DIR "share/doc/${PROJECT_NAME}" CACHE STRING "docs install path relative to CMAKE_INSTALL_PREFIX")
#set(MY_INSTALL_DESTINATION "${LIB_INSTALL_DIR}cmake/${PROJECT_NAME}")

# options
# NOTE: these options are named conventionally; they are checked by some helper functions in PackageHelpers.cmake
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_DOCUMENTATION "Build documentation (HTML)" OFF)
option(EXPORT_BUILD_DIR "Export build directory using CMake (enables external use without install)" OFF)

# default build configuration
if(NOT CMAKE_BUILD_TYPE)
    message("-- No build type specified; defaulting to CMAKE_BUILD_TYPE=RelWithDebInfo.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# targets
#add_subdirectory(src/base)
#add_subdirectory(src/sys)

# tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(src/tests)
endif()

# documentation (with Sphinx)
if(BUILD_DOCUMENTATION)
    find_package(Sphinx QUIET)
    if(SPHINX_FOUND)
        add_subdirectory(doc)
    else(SPHINX_FOUND)
        message("WARNING: Could not find Sphinx, not building documentation.")
        makeshift_update_cache_variable(BUILD_DOCUMENTATION OFF)
    endif(SPHINX_FOUND)
endif(BUILD_DOCUMENTATION)

# generate version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

# installation of public headers
makeshift_install(
    DIRECTORY "${RELATIVE_INCLUDE_DIR}"
    DESTINATION "${RELATIVE_INCLUDE_DIR}")

# Install method #1: Put library in CMAKE_INSTALL_PREFIX, i.e. /usr/local or equivalent.
makeshift_install_package()

# Install method #2: Put package build directory into local CMake registry. This allows the detection
# and use of the package without requiring that it be installed.
if(EXPORT_BUILD_DIR)
    makeshift_export_build_dir()
endif(EXPORT_BUILD_DIR)
