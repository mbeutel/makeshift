
# makeshift C++ library
# Author: Moritz Beutel
# makeshift library target


cmake_minimum_required(VERSION 3.10)

# dependencies
find_package(CMakeshift 2.0 REQUIRED)
find_package(MS-GSL REQUIRED)

# library target
add_library(makeshift
    "version.cpp"
    "serialize_enum.cpp"
    "serializer_bool.cpp"
    "serializer_string.cpp"
    "string_compare.cpp"
    "constrained.cpp")

# library version header file
configure_file("${PROJECT_SOURCE_DIR}/cmake/detail_version.hpp.in"
    "${PROJECT_BINARY_DIR}/include/makeshift/detail/version.hpp" @ONLY)

# library properties
target_include_directories(makeshift
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>")
target_compile_features(makeshift PUBLIC cxx_std_17)
target_compile_definitions(makeshift PRIVATE MAKESHIFT_BUILD)
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(makeshift PUBLIC MAKESHIFT_STATIC_LIBRARIES)
endif()

# compiler settings
include(CMakeshift/SetTargetCompilerSettings)
cmakeshift_set_target_compiler_settings(makeshift PRIVATE default pedantic)

# library version
include(CMakeshift/SetLibraryVersion)
cmakeshift_set_library_version(makeshift ${PROJECT_VERSION} ${makeshift_APIVERSION})

# library dependencies
target_link_libraries(makeshift
    PUBLIC
        MS-GSL::MS-GSL)

# define alias so we can use the qualified name everywhere
add_library(${PROJECT_NAME}::makeshift ALIAS makeshift)

# installation: public header files, generated header files, target
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "include")
install(
    DIRECTORY "${PROJECT_BINARY_DIR}/include/"
    DESTINATION "include")
install(
    TARGETS makeshift
    EXPORT ${PROJECT_NAME}Export
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib"
    INCLUDES DESTINATION "include")
