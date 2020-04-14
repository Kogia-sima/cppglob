# C++ GLOB library

[![made-with-cpp](https://img.shields.io/badge/Made%20with-C++-1f425f.svg)](http://www.cplusplus.com/)
[![Build Status](https://travis-ci.org/Kogia-sima/cppglob.svg?branch=master)](https://travis-ci.org/Kogia-sima/cppglob)
[![Build status](https://ci.appveyor.com/api/projects/status/3g83j7fyxak52dvp?svg=true)](https://ci.appveyor.com/project/Kogiasima/cppglob)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/55beda5e61824389bb78489f44a07a6a)](https://www.codacy.com/app/orcinus4627/cppglob?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Kogia-sima/cppglob&amp;utm_campaign=Badge_Grade)
[![Report](https://inspecode.rocro.com/badges/github.com/Kogia-sima/cppglob/report?token=y0OJyNYwrlrq2ON7VwNjczZEOqvWZykWH9g71RJlpwQ&branch=master)](https://inspecode.rocro.com/reports/github.com/Kogia-sima/cppglob/branch/master/summary)
[![codecov](https://codecov.io/gh/Kogia-sima/cppglob/branch/master/graph/badge.svg)](https://codecov.io/gh/Kogia-sima/cppglob)
[![GitHub license](https://img.shields.io/github/license/Kogia-sima/cppglob.svg)](https://github.com/Kogia-sima/cppglob/blob/master/LICENSE.txt)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)](http://makeapullrequest.com)
[![GitHub last commit](https://img.shields.io/github/last-commit/Kogia-sima/cppglob.svg?style=flat)](https://github.com/Kogia-sima/cppglob/commits/master)

`cppglob` is a C++ port of Python `glob` module.

Currently `cppglob` supports the following functions.

-   `glob(pattern, recursive = false)`
-   `iglob(pattern, recursive = false)`
-   `escape(pathname)`

:warning: This project is no longer maintained. If anyone is interested in continuing the project, let me know so that I can transfer ownership of this repository.

## Prerequisites

-   OS: Windows, OSX, Linux
-   Compiler: Currently the following compilers are supported.
    -   MSVC >= 19.14 (Visual Studio 2017 version 15.7.1)
    -   GCC >= 8.1
    -   Clang >= 3.5.0 (with libstdc++), >= 7.0.0 (with libc++)
-   Cmake >= 3.1.0

## Install

### Using CMake

```console
$ mkdir -p build
$ cd build
$ cmake -DBUILD_SHARED_LIBS=ON ..
$ make
$ sudo make install
```

### Integrate with VC++ project

Currently you must place all header files and sources into the proper directories.

- headers: `include/*/*.hpp`
- sources: `src/*.cpp`

I'm developing the [Nuget](https://docs.microsoft.com/en-us/nuget/) package.

If the package released, you can easily integrate this project using GUI plugin.

## Example

```cpp
#include <vector>
#include <list>
#include <algorithm>
#include <filesystem>
#include <cppglob/glob.hpp>  // cppglob::glob
#include <cppglob/iglob.hpp>  // cppglob::iglob

namespace fs = std::filesystem;

int main() {
    // get all files and directories in the same directory
    std::vector<fs::path> entries = cppglob::glob("./*");
    
    // get all directories in the directory dir_a/
    std::vector<fs::path> dirs = cppglob::glob("dir_a/*/");
    
    // get all text files under the directory dir_b/ (recursively searched)
    std::vector<fs::path> files = cppglob::glob("dir_b/**/*.txt", true);

    // get all pdf files in docs/ directory in order of file name.
    std::vector<fs::path> pdf_files = cppglob::glob("docs/*.pdf");
    std::sort(pdf_files.begin(), pdf_files.end());

    // get all pdf files in docs/ directory in order of file name. (with iglob)
    cppglob::glob_iterator it = cppglob::iglob("docs/*.pdf"), end;
    std::list<fs::path> pdf_file_list(it, end);
    pdf_file_list.sort();
}
```

## TODO

-   Conan package
-   Nuget package
-   C++14 support (with `<experimental/filesystem>` or `<boost/filesystem>`)
-   support CPPGLOB\_HEADER\_ONLY macro
-   Performance improvement
