# makeshift::test
# Author: Moritz Beutel
# Distributed under the BSD 3-Clause License. See accompanying file licensing.txt for details.


cmake_minimum_required(VERSION 3.8)

find_package(Catch2)

# test target
add_executable(test-makeshift
    "tests/main.cpp"
    "tests/test-types.cpp"
    "tests/test-metadata.cpp")

# test dependencies
target_link_libraries(test-makeshift
    PRIVATE
        makeshift Catch2::Catch)