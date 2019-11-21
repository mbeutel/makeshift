
# makeshift C++ library
# Author: Moritz Beutel
# makeshift unit tests


cmake_minimum_required(VERSION 3.14)

# dependencies
find_package(Catch2 2.7 REQUIRED)
find_package(CMakeshift 3.5 REQUIRED)
find_package(gsl-lite REQUIRED)
find_package(mpark_variant REQUIRED)

# test targets
add_executable(test-makeshift-cxx14
    "main.cpp"
    "test-arithmetic.cpp"
    "test-array.cpp"
    "test-buffer.cpp"
    "test-constval.cpp"
    "test-enum.cpp"
    "test-fenv.cpp"
    "test-functional.cpp"
    "test-iostream.cpp"
    "test-memory.cpp"
    "test-new.cpp"
    "test-range.cpp"
    "test-reflect.cpp"
    "test-variant-mpark.cpp"
    "test-tuple.cpp"
    "test-type_traits.cpp"
    "test-utility.cpp")
add_executable(test-makeshift-cxx17
    "main.cpp"
    "test-arithmetic.cpp"
    "test-constval.cpp"
    "test-variant.cpp")

# compiler settings
include(CMakeshift/TargetCompileSettings)
cmakeshift_target_compile_settings(
    TARGETS
        test-makeshift-cxx14 test-makeshift-cxx17
    PRIVATE
        default)
target_compile_features(test-makeshift-cxx14
    PUBLIC
        cxx_std_14)
target_compile_features(test-makeshift-cxx17
    PUBLIC
        cxx_std_17)

# test properties
target_compile_definitions(test-makeshift-cxx14
    PRIVATE
        CATCH_CONFIG_CONSOLE_WIDTH=120
        CATCH_CLARA_CONFIG_CONSOLE_WIDTH=120
        gsl_CONFIG_CONTRACT_VIOLATION_THROWS) # this makes GSL's Expects()/Ensures() throw `gsl::fail_fast` on failure
target_compile_definitions(test-makeshift-cxx17
    PRIVATE
        CATCH_CONFIG_CONSOLE_WIDTH=120
        CATCH_CLARA_CONFIG_CONSOLE_WIDTH=120
        gsl_CONFIG_CONTRACT_VIOLATION_THROWS) # this makes GSL's Expects()/Ensures() throw `gsl::fail_fast` on failure

# test dependencies
target_link_libraries(test-makeshift-cxx14
    PRIVATE
        gsl::gsl-lite
        mpark_variant
        Catch2::Catch2
        makeshift)
target_link_libraries(test-makeshift-cxx17
    PRIVATE
        gsl::gsl-lite
        Catch2::Catch2
        makeshift)

# register tests
include(ParseAndAddCatchTests)
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS CACHE BOOL ON)
ParseAndAddCatchTests(test-makeshift-cxx14)
ParseAndAddCatchTests(test-makeshift-cxx17)
