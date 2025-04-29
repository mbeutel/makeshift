
# makeshift C++ library
# Author: Moritz Beutel


cmake_minimum_required(VERSION 3.30)

project(makeshift
    VERSION 4.0.0
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Define build options.
option(MAKESHIFT_BUILD_TESTING "Build tests" OFF)
option(MAKESHIFT_BUILD_TESTING_CUDA "Build CUDA tests" OFF)

# Obtain source dependencies.
# We use CPM to fetch dependencies in a stand-alone build but configure it to prefer packages provided through other means.
set(CPM_USE_LOCAL_PACKAGES ON)
set(CPM_SOURCE_CACHE "${CMAKE_BINARY_DIR}/CPM-cache")
include(CPM)
CPMAddPackage(NAME gsl-lite      VERSION 0.42.0 GITHUB_REPOSITORY gsl-lite/gsl-lite   SYSTEM OFF)
if(MAKESHIFT_BUILD_TESTING)
    CPMAddPackage(NAME catch2    VERSION  3.8.1 GITHUB_REPOSITORY catchorg/Catch2     SYSTEM OFF)
    # workaround, cf. https://github.com/catchorg/Catch2/issues/2046
    find_package(Catch2 REQUIRED)
    target_compile_features(Catch2 PUBLIC cxx_std_17)
endif()

# Include target definitions.
add_subdirectory(src)
if(MAKESHIFT_BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
    if(MAKESHIFT_BUILD_TESTING_CUDA)
        enable_language(CUDA)
        add_subdirectory(test/cuda)
    endif()
endif()

# Install.
include(InstallBasicPackageFiles)
install_basic_package_files(makeshift
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
    DEPENDENCIES
        "gsl-lite 0.42"
)
