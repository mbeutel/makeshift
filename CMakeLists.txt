
# makeshift C++ library
# Author: Moritz Beutel


## 1. Prologue and public interface
## ================================

cmake_minimum_required(VERSION 3.12)

# Disable in-source builds to avoid cluttering the source directory.
# If you attempted to build in the source directory and run into this error, you first need to clean up
# manually by deleting the CMakeFiles directory and the CMakeCache.txt file before proceeding.
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Define the project.
project(makeshift
    VERSION 3.0.1
    LANGUAGES CXX)
set(makeshift_APIVERSION 2)

# Define build options.
option(BUILD_TESTS "Build tests" OFF)


## 2. Targets
## ==========

# Define project targets.
add_subdirectory(src)

# Define tests.
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()


## 3. Installation
## ===============

find_package(CMakeshift 3.3 REQUIRED)
include(CMakeshift/InstallSimpleLibraryPackage)
cmakeshift_install_simple_library_package(
    EXPORT makeshiftExport
    VERSION_COMPATIBILITY SameMajorVersion
    CONFIG_TEMPLATE "cmake/makeshiftConfig.cmake.in")
