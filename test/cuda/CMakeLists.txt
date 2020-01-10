
# makeshift C++ library
# Author: Moritz Beutel
# makeshift CUDA unit tests


cmake_minimum_required(VERSION 3.14)

# dependencies
find_package(Catch2 2.9 REQUIRED)
find_package(CMakeshift 3.7 REQUIRED)
find_package(gsl-lite 0.35.5 REQUIRED)
find_package(mpark_variant REQUIRED)

include(CMakeshift/TargetCompileSettings)

# CUDA tests
get_target_property(SOURCES_CUDA14 test-makeshift-cxx14 SOURCES)
list(TRANSFORM SOURCES_CUDA14 PREPEND "../")
add_executable(test-makeshift-cuda14 ${SOURCES_CUDA14})
set_source_files_properties(${SOURCES_CUDA14} PROPERTIES LANGUAGE CUDA)
target_compile_features(test-makeshift-cuda14 PRIVATE cxx_std_14)
set_target_properties(test-makeshift-cuda14 PROPERTIES CUDA_STANDARD 14)
cmakeshift_target_compile_settings(test-makeshift-cuda14 PRIVATE default)
target_link_libraries(test-makeshift-cuda14
    PRIVATE
        test-makeshift-runner
        gsl::gsl-lite-v1
        Catch2::Catch2
        makeshift
        mpark_variant)
target_compile_definitions(test-makeshift-cuda14
    INTERFACE
        CATCH_CONFIG_CONSOLE_WIDTH=120
        CATCH_CLARA_CONFIG_CONSOLE_WIDTH=120)
