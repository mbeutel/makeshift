
# makeshift C++ library
# Author: Moritz Beutel
# makeshift unit tests


cmake_minimum_required(VERSION 3.14)

# dependencies
find_package(Catch2 2.4 REQUIRED)
find_package(CMakeshift 3.5 REQUIRED)
find_package(MS-GSL REQUIRED)

# test target
add_executable(test-makeshift
    "main.cpp"
    "test-arithmetic.cpp"
    "test-array2.cpp"
    "test-new.cpp"
    "test-buffer.cpp"
    "test-constval.cpp"
    "test-quantity.cpp"
    "test-tuple.cpp"
    "test-tuple2.cpp"
    "test-reflect2.cpp"
    "test-variant.cpp"
    "test-variant2.cpp"
    "test-utility.cpp"
    "test-utility2.cpp"
    "test-metadata.cpp"
#    "demo.cpp"
)

# compiler settings
include(CMakeshift/TargetCompileSettings)
cmakeshift_target_compile_settings(test-makeshift
    PRIVATE
        default
        diagnostics
        $<$<CONFIG:Debug>:runtime-checks>)
target_compile_features(test-makeshift
    PUBLIC
        cxx_std_20)

# test properties
target_compile_definitions(test-makeshift
    PRIVATE
        CATCH_CONFIG_CONSOLE_WIDTH=120
        CATCH_CLARA_CONFIG_CONSOLE_WIDTH=120
        GSL_THROW_ON_CONTRACT_VIOLATION) # so we can check programmatically if violations are caught

# test dependencies
target_link_libraries(test-makeshift
    PRIVATE
        MS-GSL::MS-GSL
        Catch2::Catch2
        makeshift)

# register tests
include(ParseAndAddCatchTests)
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS CACHE BOOL ON)
ParseAndAddCatchTests(test-makeshift)
