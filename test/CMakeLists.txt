
# makeshift C++ library
# Author: Moritz Beutel
# makeshift unit tests


cmake_minimum_required(VERSION 3.10)

# dependencies
find_package(Catch2 2.3 REQUIRED)
find_package(CMakeshift 2.0 REQUIRED)

# test target
add_executable(test-makeshift
    "main.cpp"
    "tests/test-arithmetic.cpp"
    "tests/test-tuple.cpp"
    "tests/test-utility.cpp"
    "tests/test-metadata.cpp")

# compiler settings
include(CMakeshift/SetTargetCompilerSettings)
cmakeshift_set_target_compiler_settings(test-makeshift PRIVATE default pedantic)

# test dependencies
target_link_libraries(test-makeshift
    PRIVATE
        Catch2::Catch2
        makeshift)

# register tests
include(ParseAndAddCatchTests)
set(PARSE_CATCH_TESTS_NO_HIDDEN_TESTS CACHE BOOL ON)
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS CACHE BOOL ON)
set(PARSE_CATCH_TESTS_ADD_TARGET_IN_TEST_NAME CACHE BOOL OFF)
ParseAndAddCatchTests(test-makeshift)
