# makeshift::makeshift
# Author: Moritz Beutel
# Distributed under the BSD 3-Clause License. See accompanying file licensing.txt for details.


cmake_minimum_required(VERSION 3.8)

# library target
add_library(makeshift INTERFACE)

# library properties
target_include_directories(makeshift
    INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")

install(TARGETS makeshift
    EXPORT ${PROJECT_NAME}Export
    #ARCHIVE DESTINATION "lib${MAKESHIFT_LIB_SUFFIX}"
    #LIBRARY DESTINATION "lib${MAKESHIFT_LIB_SUFFIX}"
    INCLUDES DESTINATION "include")
