
# makeshift C++ library
# Author: Moritz Beutel
# makeshift library target


cmake_minimum_required(VERSION 3.14)

# dependencies
find_package(CMakeshift 3.5 REQUIRED)
find_package(gsl-lite REQUIRED)

# library target
add_library(makeshift
    "new.cpp"
    "memory.cpp"
    "string.cpp")

# library version header file
configure_file("${PROJECT_SOURCE_DIR}/cmake/detail_version.hpp.in"
    "${PROJECT_BINARY_DIR}/include/makeshift/detail/version.hpp" @ONLY)

# library export header
include(GenerateExportHeader)
generate_export_header(makeshift
    EXPORT_MACRO_NAME MAKESHIFT_PUBLIC
    EXPORT_FILE_NAME "${PROJECT_BINARY_DIR}/include/makeshift/detail/export.hpp"
    INCLUDE_GUARD_NAME "INCLUDED_MAKESHIFT_DETAIL_EXPORT_HPP_")

# library properties
target_include_directories(makeshift
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>")
target_compile_features(makeshift
    PUBLIC
        cxx_std_17)

# compiler settings
include(CMakeshift/TargetCompileSettings)
cmakeshift_target_compile_settings(makeshift
    PRIVATE
        default
        diagnostics
        $<$<CONFIG:Debug>:runtime-checks>)

# library version
include(CMakeshift/TargetLibraryVersion)
cmakeshift_target_library_version(makeshift ${PROJECT_VERSION} ${makeshift_APIVERSION})

# library dependencies
target_link_libraries(makeshift
    PUBLIC
        gsl::gsl-lite)

# installation: public header files, generated header files, target
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/" # the trailing '/' is significant
    TYPE INCLUDE)
install(
    DIRECTORY "${PROJECT_BINARY_DIR}/include/" # the trailing '/' is significant
    TYPE INCLUDE)
install(
    TARGETS makeshift
    EXPORT makeshift
    RUNTIME ARCHIVE LIBRARY INCLUDES)
