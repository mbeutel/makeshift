
# makeshift C++ library
# Author: Moritz Beutel
# makeshift CUDA unit tests


cmake_minimum_required(VERSION 3.30)

# CUDA tests
add_executable(test-makeshift-cuda20)
target_compile_features(test-makeshift-cuda20 PRIVATE cxx_std_20 cuda_std_20)
target_link_libraries(test-makeshift-cuda20
    PRIVATE
        test-makeshift-settings
)
add_test(NAME test-makeshift-cuda20 COMMAND test-makeshift-cuda20)
set_property(TEST test-makeshift-cuda20 PROPERTY FAIL_REGULAR_EXPRESSION "Sanitizer")

# All the sources in the C++20 test target should also compile with NVCC.
get_target_property(SOURCES_cuda20 test-makeshift-cxx20 SOURCES)
list(TRANSFORM SOURCES_cuda20 PREPEND "../")
target_sources(test-makeshift-cuda20 PRIVATE ${SOURCES_cuda20})
set_property(SOURCE ${SOURCES_cuda20} PROPERTY LANGUAGE CUDA)
