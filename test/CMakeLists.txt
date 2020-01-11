
# makeshift C++ library
# Author: Moritz Beutel
# makeshift unit tests


cmake_minimum_required(VERSION 3.14)

# dependencies
find_package(Catch2 2.9 REQUIRED)
find_package(CMakeshift 3.7 REQUIRED)
find_package(gsl-lite 0.35.5 REQUIRED)
find_package(mpark_variant REQUIRED)

include(CMakeshift/TargetCompileSettings)

# test runner target
add_library(test-makeshift-runner OBJECT "main.cpp")
target_compile_features(test-makeshift-runner PRIVATE cxx_std_14)
cmakeshift_target_compile_settings(test-makeshift-runner PRIVATE default)
target_link_libraries(test-makeshift-runner
    PRIVATE
        Catch2::Catch2)

# common settings target
add_library(test-makeshift-settings INTERFACE)
target_compile_definitions(test-makeshift-settings
    INTERFACE
        CATCH_CONFIG_CONSOLE_WIDTH=120
        CATCH_CLARA_CONFIG_CONSOLE_WIDTH=120
        gsl_CONFIG_CONTRACT_CHECKING_AUDIT # run audit-level checks
        gsl_CONFIG_CONTRACT_VIOLATION_THROWS) # make gsl-lite's precondition assertions throw `gsl::fail_fast` on failure
target_link_libraries(test-makeshift-settings
    INTERFACE
        gsl::gsl-lite-v1
        Catch2::Catch2
        makeshift)
if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0.0)
    target_compile_options(test-makeshift-settings
        INTERFACE
            -Wno-missing-braces)
endif()
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(test-makeshift-settings
        INTERFACE
            # C standard library
            <cstddef>
            <cstdint>
            # C++ standard library
            <array>
            <tuple>
            <iosfwd>
            <utility>
            <iterator>
            <exception>
            <type_traits>
            # other libraries
            <gsl/gsl-lite.hpp>
            <catch2/catch.hpp>)
endif()

# C++14 tests
add_executable(test-makeshift-cxx14
    "test-algorithm.cpp"
    "test-array.cpp"
    "test-buffer.cpp"
    "test-constval.cpp"
    "test-enum.cpp"
    "test-functional.cpp"
    "test-iostream.cpp"
    "test-ranges.cpp"
    "test-reflect.cpp"
    "test-span.cpp"
    "test-mpark-variant.cpp"
    "test-tuple.cpp"
    "test-type_traits.cpp"
    "test-utility.cpp")
target_compile_features(test-makeshift-cxx14 PRIVATE cxx_std_14)
cmakeshift_target_compile_settings(test-makeshift-cxx14 PRIVATE default)
target_link_libraries(test-makeshift-cxx14
    PRIVATE
        test-makeshift-runner
        test-makeshift-settings
        mpark_variant)
add_test(NAME test-makeshift-cxx14 COMMAND test-makeshift-cxx14)

# C++17 tests
if(cxx_std_17 IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    add_executable(test-makeshift-cxx17
        "test-constval.cpp"
        "test-variant.cpp")
    target_compile_features(test-makeshift-cxx17 PRIVATE cxx_std_17)
    cmakeshift_target_compile_settings(test-makeshift-cxx17 PRIVATE default)
    target_link_libraries(test-makeshift-cxx17
        PRIVATE
            test-makeshift-runner
            test-makeshift-settings)
    add_test(NAME test-makeshift-cxx17 COMMAND test-makeshift-cxx17)
endif()

# C++20 tests
if(cxx_std_20 IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    add_executable(test-makeshift-cxx20
        "test-concepts.cpp"
        "test-constval.cpp")
    target_compile_features(test-makeshift-cxx20 PRIVATE cxx_std_20)
    cmakeshift_target_compile_settings(test-makeshift-cxx20 PRIVATE default)
    target_link_libraries(test-makeshift-cxx20
        PRIVATE
            test-makeshift-runner
            test-makeshift-settings)
    add_test(NAME test-makeshift-cxx20 COMMAND test-makeshift-cxx20)
endif()
