
# makeshift C++ library
# Author: Moritz Beutel
# makeshift CUDA unit tests


cmake_minimum_required(VERSION 3.14)

# dependencies
find_package(CMakeshift 3.7 REQUIRED)
find_package(mpark_variant REQUIRED)

include(CMakeshift/TargetCompileSettings)

# CUDA tests
add_executable(test-makeshift-cuda14)
target_compile_features(test-makeshift-cuda14 PRIVATE cxx_std_14)
set_target_properties(test-makeshift-cuda14 PROPERTIES CUDA_STANDARD 14)
cmakeshift_target_compile_settings(test-makeshift-cuda14 PRIVATE default)
target_link_libraries(test-makeshift-cuda14
    PRIVATE
        test-makeshift-runner
        test-makeshift-settings
        mpark_variant)
add_test(NAME test-makeshift-cuda14 COMMAND test-makeshift-cuda14)
set_property(TEST test-makeshift-cuda14 PROPERTY FAIL_REGULAR_EXPRESSION "Sanitizer")

# All the sources in the C++14 test target should also compile with NVCC.
get_target_property(SOURCES_CUDA14 test-makeshift-cxx14 SOURCES)
list(TRANSFORM SOURCES_CUDA14 PREPEND "../")
target_sources(test-makeshift-cuda14 PRIVATE ${SOURCES_CUDA14})
set_property(SOURCE ${SOURCES_CUDA14} PROPERTY LANGUAGE CUDA)
