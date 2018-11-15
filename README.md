# C++ GLOB library

`cppglob` is a C++ port of Python `glob` module.

Currently `cppglob` supports the following functions.

- `glob(pattern, recursive = false)`
- `iglob(pattern, recursive = false)`

## Prerequisites

- OS: Unix-like system (Linux, OSX, WSL, Cygwin, etc.)
- Compiler: must support the following C++17 features.
    - `<filesystem>` library
    - `<string_view>` library
- Cmake >= 3.1.0

## Install

```console
$ mkdir -p build
$ cd build
$ cmake -DBUILD_SHARED_LIBS=ON ..
$ make
$ sudo make install
```

## Example

```cpp
#include <iostream>
#include <cppglob/glob.hpp>

int main() {
    // get all files and directories in the same directory
    std::vector<fs::path> entries = cppglob::glob("./*");
    
    // get all directories in the directory dir_a/
    std::vector<fs::path> dirs = cppglob::glob("dir_a/*/");
    
    // get all text files under the directory dir_b/ (recursively searched)
    std::vector<fs::path> files = cppglob::glob("dir_b/**/*.txt", true);
}
```

## TODO

- MSVC support
- C++14 support (with `<experimental/filesystem>`)
- Automatically generate single header file (in progress)
- `cppglob::escape` function support
- Test coverage analysis
- Performance improvement