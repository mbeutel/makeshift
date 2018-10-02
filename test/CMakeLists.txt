
# makeshift C++ library
# Author: Moritz Beutel
# makeshift unit tests


cmake_minimum_required(VERSION 3.10)

# dependencies
find_package(Catch2 2.3 REQUIRED)
find_package(CMakeshift 2.1 REQUIRED)

# test target
add_executable(test-makeshift
    "main.cpp"
    "tests/test-arithmetic.cpp"
    "tests/test-quantity.cpp"
    "tests/test-tuple.cpp"
    "tests/test-variant.cpp"
    "tests/test-utility.cpp"
    "tests/test-metadata.cpp")

# compiler settings
include(CMakeshift/TargetCompileSettings)
cmakeshift_target_compile_settings(test-makeshift PRIVATE default pedantic)

# test properties
target_compile_definitions(test-makeshift
    PRIVATE
        CATCH_CONFIG_CONSOLE_WIDTH=120
        CATCH_CLARA_CONFIG_CONSOLE_WIDTH=120)
set_target_properties(test-makeshift PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

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
