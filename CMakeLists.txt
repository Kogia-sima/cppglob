cmake_minimum_required(VERSION 3.1.0)

project(cppglob VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_STATIC "Build static library" OFF)
option(BUILD_SHARED "Build shared library" ON)
option(BUILD_TEST "Build test" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(cotire)

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  set(BUILD_TEST ON)
endif()

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
add_definitions("-DCPPGLOB_BUILDING")
file(GLOB cpp_sources ${PROJECT_SOURCE_DIR}/src/*.cpp)

if(MSVC)
  set(CompilerFlags
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
  )
  if(BUILD_SHARED_LIBS)
    foreach(CompilerFlag ${CompilerFlags})
      string(REPLACE "/MT" "/MD" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
  else()
    foreach(CompilerFlag ${CompilerFlags})
      string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
  endif()
endif()

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Coverage option enabled!")
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 3)
      message(FATAL_ERROR "Clang version must be 3.0.0 or greater! Aborting...")
    else()
      message(STATUS "Coverage option enabled!")
    endif()
  else()
    message(FATAL_ERROR "Coverage report requires GNU gcc compiler")
  endif()

  set(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
endif()

include_directories(include)

add_subdirectory(src)

if(BUILD_TEST)
  enable_testing()
  add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/googletest)
  add_subdirectory(test)
endif()

install(
  DIRECTORY include/cppglob
  DESTINATION include
  COMPONENT headers)

install(
  EXPORT cppglob-config
  DESTINATION share/cmake/cppglob)
