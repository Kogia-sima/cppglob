# C++ GLOB library

[![made-with-python](https://img.shields.io/badge/Made%20with-C++-1f425f.svg)](http://www.cplusplus.com/)
[![Build Status](https://travis-ci.org/machida-mn/cppglob.svg?branch=master)](https://travis-ci.org/machida-mn/cppglob)
[![Build status](https://ci.appveyor.com/api/projects/status/01dto0a1vi94ayeu/branch/master?svg=true)](https://ci.appveyor.com/project/machida-mn/cppglob/branch/master)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/3ea442a1cd624c62af285e7de532319a)](https://www.codacy.com/app/machida-mn/cppglob?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=machida-mn/cppglob&amp;utm_campaign=Badge_Grade)
[![GitHub license](https://img.shields.io/github/license/machida-mn/cppglob.svg)](https://github.com/machida-mn/cppglob/blob/master/LICENSE.txt)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)

`cppglob` is a C++ port of Python `glob` module.

Currently `cppglob` supports the following functions.

-   `glob(pattern, recursive = false)`
-   `iglob(pattern, recursive = false)`

## Prerequisites

-   OS: Unix-like system (Linux, OSX, WSL, Cygwin, etc.)
-   Compiler: Currently the following compilers are supported.
    -   MSVC >= 19.14 (Visual Studio 2017 version 15.7.1)
    -   GCC >= 8.1
    -   Clang >= 3.5.0 (with libstdc++), >= 7.0.0 (with libc++)
-   Cmake >= 3.1.0

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

-   C++14 support (with `<experimental/filesystem>`)
-   Automatically generate single header file (in progress)
-   `cppglob::escape` function support
-   Performance improvement
