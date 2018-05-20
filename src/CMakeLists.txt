# makeshift::makeshift
# Author: Moritz Beutel
# Distributed under the BSD 3-Clause License. See accompanying file licensing.txt for details.


cmake_minimum_required(VERSION 3.8)

# dependencies
find_package(GSL REQUIRED)

# library target
add_library(makeshift
    "serialize.cpp")

# library properties
target_include_directories(makeshift
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")
target_compile_definitions(makeshift PRIVATE MAKESHIFT_BUILD_MAKESHIFT_SYS)
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(makeshift PUBLIC MAKESHIFT_STATIC_LIBRARIES)
endif()

# library dependencies
target_link_libraries(makeshift
    PUBLIC
        GSL::GSL)

# test target
add_executable(compilerbugs
    "compilerbug.cpp")
target_link_libraries(compilerbugs
    PUBLIC
        makeshift)

install(TARGETS makeshift
    EXPORT ${PROJECT_NAME}Export
    ARCHIVE DESTINATION "lib${MAKESHIFT_LIB_SUFFIX}"
    LIBRARY DESTINATION "lib${MAKESHIFT_LIB_SUFFIX}"
    INCLUDES DESTINATION "include")
