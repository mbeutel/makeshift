# makeshift: lightweight metaprogramming for C++

| metadata | build  | tests  |
| -------- | ------ | ------ |
| [![Language](https://badgen.net/badge/C++/17+/blue)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) <br> [![License](https://badgen.net/github/license/mbeutel/makeshift)](https://opensource.org/licenses/BSL-1.0) <br> [![Version](https://badgen.net/github/release/mbeutel/makeshift)](https://github.com/mbeutel/makeshift/releases)   |   [![Build Status](https://dev.azure.com/moritzbeutel/makeshift/_apis/build/status/mbeutel.makeshift?branchName=master)](https://dev.azure.com/moritzbeutel/makeshift/_build/latest?definitionId=2&branchName=master)   |   [![Azure DevOps tests](https://img.shields.io/azure-devops/tests/moritzbeutel/makeshift/2)](https://dev.azure.com/moritzbeutel/makeshift/_testManagement/runs)  |


*makeshift* is a C++ library which augments the C++ Standard Library with the following:
static range algorithms, constvals, a metadata mechanism, and variant transforms.

## Contents

- [Example usage](#example-usage)
- [License](#license)
- [Compiler and platform support](#compiler-and-platform-support)
- [Dependencies](#dependencies)
- [Installation and use](#installation-and-use)
- [Version semantics](#version-semantics)
- [Features](#features)
- [Reference](#reference)


## Example usage

```c++
// assuming C++23
#include <array>
#include <string>
#include <ranges>
#include <tuple>

#include <makeshift/iomanip.hpp>   // for as_enum()
#include <makeshift/metadata.hpp>
#include <makeshift/string.hpp>    // for enum_to_string()

enum class Boundary
{
    dirichlet,
    periodic
};
constexpr auto reflect(gsl::type_identity<Boundary>)
{
    return std::array{
        std::tuple{ Boundary::dirichlet, "Dirichlet" },
        std::tuple{ Boundary::periodic, "periodic" }
    };
}

int main()
{
    constexpr std::array boundaryValues = makeshift::metadata::values<Boundary>();
    std::string boundaryValueList = boundaryValues
        | std::views::transform(
              [](Boundary b)
              {
                  return makeshift::enum_to_string(b);
              })
        | std::views::join_with(", ")
        | std::ranges::to<std::string>();

    std::cout << "Which kind of boundary to use? (" << boundaryValueList << ")\n";
    Boundary b;
    if (!std::cin >> makeshift::as_enum(b))
    {
        std::cerr << "Invalid input.\n";
        return 1;
    }
    std::cout << "Selected boundary: " << makeshift::as_enum(b) << "\n";
}
```


## License

*makeshift* uses the [Boost Software License](LICENSE.txt).
 

## Dependencies

* [gsl-lite](https://github.com/gsl-lite/gsl-lite), an implementation of the [C++ Core Guidelines Support Library](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-gsl)
* optional (for testing only): [Catch2](https://github.com/catchorg/Catch2)


## Installation and use

### As CMake package

The recommended way to consume *makeshift* in your CMake project is to use `find_package()` and `target_link_libraries()`:

```CMake
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
    
find_package(makeshift 4.0 REQUIRED)
    
project(my-program LANGUAGES CXX)
    
add_executable(my-program main.cpp)
target_link_libraries(my-program PRIVATE makeshift::makeshift)
```

The easiest way to set up the dependencies is to use the [Vcpkg](https://vcpkg.io/en/index.html)
package manager. With Vcpkg available, clone the *makeshift* repository and configure a build
directory with CMake:

    git clone git@github.com:mbeutel/makeshift.git <makeshift-source-dir>
    cd <makeshift-source-dir>
    mkdir build
    cd build
    cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE="<vcpkg-dir>/scripts/buildsystems/vcpkg.cmake" ..

Now, configure your project passing the CMake build directory as a parameter:

    cd <my-program-source-dir>
    mkdir build
    cd build
    cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE="<vcpkg-dir>/scripts/buildsystems/vcpkg.cmake" -Dmakeshift_DIR:PATH=<makeshift-source-dir>/build ..
    cmake --build .


## Version semantics

*makeshift* follows [Semantic Versioning](https://semver.org/) guidelines with regard to its
[API](https://en.wikipedia.org/wiki/Application_programming_interface). We aim to retain full
API compatibility and to avoid breaking changes in minor and patch releases.

We do not guarantee to maintain [ABI](https://en.wikipedia.org/wiki/Application_binary_interface)
compatibility except in patch releases.

Development happens in the `master` branch. Versioning semantics apply only to tagged releases:
there is no stability guarantee between individual commits in the `master` branch, that is, anything
added since the last tagged release may be renamed, removed, have the semantics changed, etc. without
further notice.

A minor-version release will be API-compatible with the previous minor-version release. Thus, once
a change is released, it becomes part of the API.


## Features

TODO


## Reference

TODO
