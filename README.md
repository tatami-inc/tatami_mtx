# Create `tatami` matrices from Matrix Market files

![Unit tests](https://github.com/tatami-inc/tatami_mtx/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/tatami-inc/tatami_mtx/actions/workflows/doxygenate.yaml/badge.svg)
[![Codecov](https://codecov.io/gh/tatami-inc/tatami_mtx/branch/master/graph/badge.svg?token=Z189ORCLLR)](https://codecov.io/gh/tatami-inc/tatami_mtx)

## Overview

Pretty much as it says on the tin.
This provides one-line wrappers around the [**eminem**](https://github.com/tatami-inc/eminem) parser
to enable quick creation of [**tatami**](https://github.com/tatami-inc/tatami) matrices from Matrix Market files.

## Quick start

The `load_matrix_from_file()` function will create a `tatami::Matrix` from a Matrix Market file:

```cpp
#include "tatami_mtx/tatami_mtx.hpp"

auto mat = tatami_mtx::load_matrix_from_file<false, double, int>("some_matrix.mtx");

// If compiled with Zlib support:
auto mat2 = tatami_mtx::load_matrix_from_file<false, double, int>("some_matrix.mtx.gz", 1);

// If the compression is unknown:
auto mat3 = tatami_mtx::load_matrix_from_file<false, double, int>("some_matrix.mtx.??", -1);
```

This will return a compressed sparse column matrix for coordinate formats and a column-major dense matrix for array formats.
If the first template argument is `true`, row-based matrices are returned instead.

The next template arguments control the interface and storage types - for example, the above call will return a `tatami::Matrix<double, int>` interface
The storage types are automatically chosen based on the Matrix Market field (for data) and the size of the matrix (for indices, only relevant for sparse outputs).
Users can customize these by passing the desired types directly:

```cpp
auto mat2 = tatami_mtx::load_matrix_from_file<
    true, /* Row-based */
    double, /* Data interface */
    int, /* Index interface */
    int32_t, /* Data storage */
    uint16_t /* Index storage */
>("some_matrix.mtx");
```

Check out the [reference documentation](https://tatami-inc.github.io/tatami_mtx) for more details.

## Building projects

### CMake with `FetchContent`

If you're using CMake, you just need to add something like this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  tatami_mtx
  GIT_REPOSITORY https://github.com/tatami-inc/tatami_mtx
  GIT_TAG master # or any version of interest 
)

FetchContent_MakeAvailable(tatami_mtx)
```

Then you can link to **tatami_mtx** to make the headers available during compilation:

```cmake
# For executables:
target_link_libraries(myexe tatami_mtx)

# For libaries
target_link_libraries(mylib INTERFACE tatami_mtx)
```

### CMake using `find_package()`

You can install the library by cloning a suitable version of this repository and running the following commands:

```sh
mkdir build && cd build
cmake .. -DTATAMI_MTX_TESTS=OFF
cmake --build . --target install
```

Then you can use `find_package()` as usual:

```cmake
find_package(tatami_tatami_mtx CONFIG REQUIRED)
target_link_libraries(mylib INTERFACE tatami::tatami_mtx)
```

### Manual

If you're not using CMake, the simple approach is to just copy the files the `include/` subdirectory -
either directly or with Git submodules - and include their path during compilation with, e.g., GCC's `-I`.
You'll also need to link to the [**tatami**](https://github.com/tatami-inc/tatami) and [**eminem**](https://github.com/tatami-inc/eminem) libraries (and their dependencies).

