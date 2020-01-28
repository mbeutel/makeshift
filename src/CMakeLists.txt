
# makeshift C++ library
# Author: Moritz Beutel
# makeshift library target


cmake_minimum_required(VERSION 3.14)

# dependencies
find_package(CMakeshift 3.7 REQUIRED)
find_package(gsl-lite 0.35.6 REQUIRED)

# library target
add_library(makeshift INTERFACE)
target_compile_features(makeshift INTERFACE cxx_std_14)
target_include_directories(makeshift
    INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>")
target_link_libraries(makeshift
    INTERFACE
        gsl::gsl-lite-v1)

# installation: public header files, target
install(
    DIRECTORY
        "${PROJECT_SOURCE_DIR}/include/" # the trailing '/' is significant
    TYPE INCLUDE)
install(
    TARGETS makeshift
    EXPORT makeshift
    INCLUDES)
