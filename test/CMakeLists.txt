
# makeshift C++ library
# Author: Moritz Beutel
# makeshift unit tests


cmake_minimum_required(VERSION 3.14)

# dependencies
find_package(Catch2 2.4 REQUIRED)
find_package(CMakeshift 3.5 REQUIRED)
find_package(gsl-lite REQUIRED)

# test target
add_executable(test-makeshift
    "main.cpp"
    "test-arithmetic.cpp"
    "test-array.cpp"
    "test-buffer.cpp"
    "test-constval.cpp"
    "test-enum.cpp"
    "test-functional.cpp"
    "test-iostream.cpp"
    "test-iterator.cpp"
    "test-memory.cpp"
    "test-new.cpp"
    "test-range.cpp"
    "test-reflect.cpp"
    #"test-reflect2.cpp"
    "test-string.cpp"
    "test-tuple.cpp"
    "test-type_traits.cpp"
    "test-utility.cpp"
    "test-variant.cpp")
    #"test-variant2.cpp")

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
        gsl_CONFIG_CONTRACT_VIOLATION_THROWS) # so we can check programmatically if violations are caught

# test dependencies
target_link_libraries(test-makeshift
    PRIVATE
        gsl::gsl-lite
        Catch2::Catch2
        makeshift)

# register tests
include(ParseAndAddCatchTests)
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS CACHE BOOL ON)
ParseAndAddCatchTests(test-makeshift)
