Setting up makeshift for development on Linux
=============================================

Install package prerequisites:

```bash
VCPKG_ROOT=$(vcpkg-root <the-root>)
vcpkg install ms-gsl catch2
```

Configure project (from project directory):

```bash
$ git submodule init
$ git submodule update
$ mkdir build
$ cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=$(vcpkg-toolchain <the-toolchain>) \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DEXPORT_BUILD_DIR=ON \
      -DBUILD_TESTS=ON \
      ..
```

Build (from build subdirectory):

```bash
$ cmake --build .
```

