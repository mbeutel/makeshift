
# makeshift C++ library
# Author: Moritz Beutel


cmake_minimum_required(VERSION 3.14)

# Define the project.
project(makeshift
    VERSION 3.0.0
    LANGUAGES CXX)

# Define build options.
option(MAKESHIFT_BUILD_TESTS "Build tests" OFF)
option(MAKESHIFT_BUILD_CUDA_TESTS "Build CUDA tests" OFF)
option(CMAKE_EXPORT_PACKAGE_REGISTRY "Export to user package registry" OFF)

# Include target definitions.
add_subdirectory(src)
if(MAKESHIFT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
    if(MAKESHIFT_BUILD_CUDA_TESTS)
        enable_language(CUDA)
        add_subdirectory(test/cuda)
    endif()
endif()

# Install.
find_package(CMakeshift 3.8 REQUIRED)
include(CMakeshift/InstallBasicPackageFiles)
cmakeshift_install_basic_package_files(makeshift
    COMPATIBILITY SameMajorVersion
    DEPENDENCIES
        "gsl-lite 0.36")
