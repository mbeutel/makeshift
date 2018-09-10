
# makeshift C++ library
# Author: Moritz Beutel
# makeshift library target


cmake_minimum_required(VERSION 3.10)

# dependencies
find_package(MS-GSL REQUIRED)

# library target
add_library(makeshift
    "version.cpp"
    "serialize_enum.cpp"
    "string_compare.cpp"
    "constrained.cpp")

# library properties
target_include_directories(makeshift
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:include>")
target_compile_features(makeshift PUBLIC cxx_std_17)
target_compile_definitions(makeshift PRIVATE MAKESHIFT_BUILD)
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(makeshift PUBLIC MAKESHIFT_STATIC_LIBRARIES)
endif()

# library dependencies
target_link_libraries(makeshift
    PUBLIC
        MS-GSL::MS-GSL)

# define alias so we can use the qualified name everywhere
add_library(${PROJECT_NAME}::makeshift ALIAS makeshift)

# installation
install(
    TARGETS makeshift
    EXPORT ${PROJECT_NAME}Export
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib"
    INCLUDES DESTINATION "include")
