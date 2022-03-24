
# makeshift C++ library
# Author: Moritz Beutel
# makeshift library target


cmake_minimum_required(VERSION 3.14)

find_package(gsl-lite 0.40 REQUIRED)

add_library(makeshift INTERFACE)

target_compile_features(makeshift INTERFACE cxx_std_17)

target_include_directories(makeshift
    INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)

target_link_libraries(makeshift
    INTERFACE
        gsl::gsl-lite-v1
)

install(
    DIRECTORY
        "${PROJECT_SOURCE_DIR}/include/" # the trailing '/' is significant
    TYPE INCLUDE
)
install(
    TARGETS makeshift
    EXPORT makeshift
    INCLUDES
)
