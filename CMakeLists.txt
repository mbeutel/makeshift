
# makeshift C++ library
# Author: Moritz Beutel


cmake_minimum_required(VERSION 3.14)

# Define the project.
project(makeshift
    VERSION 3.0.0
    LANGUAGES CXX)
set(makeshift_APIVERSION 3)

# Define build options.
option(BUILD_TESTS "Build tests" OFF)

# Include target definitions.
add_subdirectory(src)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

## Install.
find_package(CMakeshift 3.5 REQUIRED)
include(CMakeshift/InstallBasicPackageFiles)
install_basic_package_files(makeshift
    EXPORT makeshift
    COMPATIBILITY SameMajorVersion
    DEPENDENCIES
        "CMakeshift 3.5"
        "gsl-lite"
        "Hedley")
