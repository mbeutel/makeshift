
# makeshift C++ library
# Author: Moritz Beutel
# makeshift unit tests


cmake_minimum_required(VERSION 3.20)

find_package(Catch2 3.0 REQUIRED)
find_package(gsl-lite 0.40 REQUIRED)

include(TargetCompileSettings)

# common settings target
add_library(test-makeshift-settings INTERFACE)
target_compile_definitions(test-makeshift-settings
    INTERFACE
        #CATCH_CONFIG_CONSOLE_WIDTH=120
        #CATCH_CLARA_CONFIG_CONSOLE_WIDTH=120
        CATCH_CONFIG_CPP17_UNCAUGHT_EXCEPTIONS
        CATCH_CONFIG_CPP17_STRING_VIEW
        CATCH_CONFIG_CPP17_VARIANT
        CATCH_CONFIG_CPP17_OPTIONAL
        CATCH_CONFIG_CPP17_BYTE
        gsl_CONFIG_CONTRACT_CHECKING_AUDIT  # run audit-level checks
        gsl_CONFIG_CONTRACT_VIOLATION_THROWS  # make gsl-lite's precondition assertions throw `gsl::fail_fast` on failure
)
target_link_libraries(test-makeshift-settings
    INTERFACE
        gsl::gsl-lite-v1
        Catch2::Catch2WithMain
        makeshift
)
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
        <gsl-lite/gsl-lite.hpp>
        #<catch2/catch_test_macros.hpp>
)

# C++17 tests
add_executable(test-makeshift-cxx17
    "test-algorithm.cpp"
    "test-array.cpp"
    "test-constval.cpp"
    "test-functional.cpp"
    "test-iostream.cpp"
    "test-metadata.cpp"
    "test-ranges.cpp"
    "test-serialize.cpp"
    "test-tuple.cpp"
    "test-type_traits.cpp"
    "test-utility.cpp"
    "test-variant.cpp"
    "experimental/test-algorithm.cpp"
    "experimental/test-buffer.cpp"
    "experimental/test-enum.cpp"
    "experimental/test-functional.cpp"
    "experimental/test-tuple.cpp"
    "experimental/test-type_traits.cpp"
    "experimental/test-span.cpp"
    "experimental/test-utility.cpp"
    "experimental/test-variant.cpp"
)
target_compile_features(test-makeshift-cxx17 PRIVATE cxx_std_17)
cmakeshift_target_compile_settings(test-makeshift-cxx17
    SOURCE_FILE_ENCODING "UTF-8"
)
target_link_libraries(test-makeshift-cxx17
    PRIVATE
        test-makeshift-settings
)
add_test(NAME test-makeshift-cxx17 COMMAND test-makeshift-cxx17)
set_property(TEST test-makeshift-cxx17 PROPERTY FAIL_REGULAR_EXPRESSION "Sanitizer")

# C++20 tests
if(cxx_std_20 IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    add_executable(test-makeshift-cxx20
        "test-concepts.cpp"
        "test-constval.cpp"
        "test-metadata.cpp"
    )
    target_compile_features(test-makeshift-cxx20 PRIVATE cxx_std_20)
    cmakeshift_target_compile_settings(test-makeshift-cxx20
        SOURCE_FILE_ENCODING "UTF-8"
    )
    target_link_libraries(test-makeshift-cxx20
        PRIVATE
            test-makeshift-settings
    )
    add_test(NAME test-makeshift-cxx20 COMMAND test-makeshift-cxx20)
    set_property(TEST test-makeshift-cxx20 PROPERTY FAIL_REGULAR_EXPRESSION "Sanitizer")
endif()
