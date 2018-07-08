
# makeshift C++ library
# Author: Moritz Beutel
# makeshift unit tests


cmake_minimum_required(VERSION 3.8)

# dependencies
find_package(Catch2 REQUIRED)

# test target
add_executable(test-makeshift
    "main.cpp"
#    "tests/experimental/test-property.cpp"
#    "tests/experimental/test-pointers.cpp"
    "tests/test-arithmetic.cpp"
    "tests/test-tuple.cpp"
    "tests/test-utility.cpp"
    "tests/test-metadata.cpp")

# test dependencies
target_link_libraries(test-makeshift
    PRIVATE
        makeshift
        Catch2::Catch)

# register tests
include(CMakeshift/ParseAndAddCatchTests)
set(PARSE_CATCH_TESTS_NO_HIDDEN_TESTS ON)
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS ON)
set(PARSE_CATCH_TESTS_ADD_TARGET_IN_TEST_NAME OFF)
ParseAndAddCatchTests(test-makeshift)
