
# makeshift C++ library
# Author: Moritz Beutel
# makeshift library target


cmake_minimum_required(VERSION 3.14)

# dependencies
find_package(CMakeshift 3.5 REQUIRED)
find_package(gsl-lite 0.35.3 REQUIRED)

# library target
add_library(makeshift STATIC
    "fenv.cpp"
    "memory.cpp"
    "new.cpp"
    "variant.cpp")

# library properties
target_include_directories(makeshift
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")
target_compile_features(makeshift
    INTERFACE
        cxx_std_14
    PRIVATE
        cxx_std_17)

# compiler settings
include(CMakeshift/TargetCompileSettings)
cmakeshift_target_compile_settings(makeshift
    PRIVATE
        default)

if (NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Sized deallocation must be explicitly enabled on Clang, cf. https://bcain-llvm.readthedocs.io/projects/clang/en/release_37/ReleaseNotes/#new-compiler-flags.
    target_compile_options(makeshift
        PRIVATE
            -fsized-deallocation)
endif()

# library dependencies
target_link_libraries(makeshift
    PUBLIC
        gsl::gsl-lite-v1)

# installation: public header files, generated header files, target
install(
    DIRECTORY
        "${PROJECT_SOURCE_DIR}/include/" # the trailing '/' is significant
    TYPE INCLUDE)
install(
    TARGETS makeshift
    EXPORT makeshift
    ARCHIVE INCLUDES)
