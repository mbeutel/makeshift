
# makeshift C++ library
# Author: Moritz Beutel


## 1. Prologue and public interface
## ================================

cmake_minimum_required(VERSION 3.12)

# Define the project.
project(makeshift
    VERSION 3.0.0
    LANGUAGES CXX)
set(makeshift_APIVERSION 3)

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

find_package(CMakeshift 3.4 REQUIRED)
include(CMakeshift/InstallSimpleLibraryPackage)
cmakeshift_install_simple_library_package(
    EXPORT makeshiftExport
    VERSION_COMPATIBILITY SameMajorVersion
    CONFIG_TEMPLATE "cmake/makeshiftConfig.cmake.in")
