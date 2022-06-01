
# makeshift C++ library
# Author: Moritz Beutel
# makeshift CUDA unit tests


cmake_minimum_required(VERSION 3.20)

include(TargetCompileSettings)

# CUDA tests
add_executable(test-makeshift-cuda17)
target_compile_features(test-makeshift-cuda17 PRIVATE cxx_std_17 cuda_std_17)
cmakeshift_target_compile_settings(test-makeshift-cuda17 SOURCE_FILE_ENCODING "UTF-8")
target_link_libraries(test-makeshift-cuda17
    PRIVATE
        test-makeshift-settings
)
add_test(NAME test-makeshift-cuda17 COMMAND test-makeshift-cuda17)
set_property(TEST test-makeshift-cuda17 PROPERTY FAIL_REGULAR_EXPRESSION "Sanitizer")

# All the sources in the C++17 test target should also compile with NVCC.
get_target_property(SOURCES_CUDA17 test-makeshift-cxx17 SOURCES)
list(TRANSFORM SOURCES_CUDA17 PREPEND "../")
target_sources(test-makeshift-cuda17 PRIVATE ${SOURCES_CUDA17})
set_property(SOURCE ${SOURCES_CUDA17} PROPERTY LANGUAGE CUDA)
