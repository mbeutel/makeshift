
# makeshift C++ library
# Author: Moritz Beutel
# makeshift library target


cmake_minimum_required(VERSION 3.14)

# dependencies
find_package(CMakeshift 3.7 REQUIRED)
find_package(gsl-lite 0.35.5 REQUIRED)

# library target
add_library(makeshift INTERFACE)

# library properties
target_include_directories(makeshift
    INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")
target_compile_features(makeshift
    INTERFACE
        cxx_std_14)

# library dependencies
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
