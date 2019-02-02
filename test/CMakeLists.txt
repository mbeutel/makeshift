
# makeshift C++ library
# Author: Moritz Beutel
# makeshift unit tests


cmake_minimum_required(VERSION 3.12)

# dependencies
find_package(Catch2 2.4 REQUIRED)
find_package(CMakeshift 3.3 REQUIRED)

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
cmakeshift_target_compile_settings(test-makeshift
    PRIVATE
        default
        pedantic
        $<$<CONFIG:Debug>:runtime-checks>)

# test properties
target_compile_definitions(test-makeshift
    PRIVATE
        CATCH_CONFIG_CONSOLE_WIDTH=120
        CATCH_CLARA_CONFIG_CONSOLE_WIDTH=120)

# test dependencies
target_link_libraries(test-makeshift
    PRIVATE
        Catch2::Catch2
        makeshift)

# register tests
include(ParseAndAddCatchTests)
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS CACHE BOOL ON)
ParseAndAddCatchTests(test-makeshift)
