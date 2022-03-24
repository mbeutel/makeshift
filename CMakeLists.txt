
# makeshift C++ library
# Author: Moritz Beutel


cmake_minimum_required(VERSION 3.14)

project(makeshift
    VERSION 3.0.0
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Define build options.
option(BUILD_TESTING "Build tests" OFF)
option(BUILD_TESTING_CUDA "Build CUDA tests" OFF)
option(CMAKE_EXPORT_PACKAGE_REGISTRY "Export to user package registry" OFF)

# Include target definitions.
add_subdirectory(src)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
    if(BUILD_TESTING_CUDA)
        enable_language(CUDA)
        add_subdirectory(test/cuda)
    endif()
endif()

# Install.
include(InstallBasicPackageFiles)
cmakeshift_install_basic_package_files(makeshift
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
    DEPENDENCIES
        "gsl-lite 0.40"
)
