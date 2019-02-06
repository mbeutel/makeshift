
# makeshift C++ library
# Author: Moritz Beutel
# makeshift library target


cmake_minimum_required(VERSION 3.12)

# dependencies
find_package(CMakeshift 3.3 REQUIRED)
find_package(MS-GSL REQUIRED)

# library target
add_library(makeshift
    "quantity.cpp"
    "serialize.cpp"
    "serialize_enum.cpp"
    "serializer_bool.cpp"
    "serializer_quantity.cpp"
    "serializer_string.cpp"
    "serializer_stream.cpp"
    "streamable-string.cpp"
    "string_compare.cpp"
    "constrained.cpp")

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
        pedantic
        $<$<CONFIG:Debug>:runtime-checks>)

# library version
include(CMakeshift/TargetLibraryVersion)
cmakeshift_target_library_version(makeshift ${PROJECT_VERSION} ${makeshift_APIVERSION})

# library dependencies
target_link_libraries(makeshift
    PUBLIC
        MS-GSL::MS-GSL)

# installation: public header files, generated header files, target
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "include")
install(
    DIRECTORY "${PROJECT_BINARY_DIR}/include/"
    DESTINATION "include")
install(
    TARGETS makeshift
    EXPORT makeshiftExport
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    INCLUDES DESTINATION "include")
