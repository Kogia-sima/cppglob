cmake_minimum_required(VERSION 3.1.0)

project(cppglob VERSION 1.1.0 LANGUAGES CXX)

option(BUILD_STATIC "Build static library" OFF)
option(BUILD_SHARED "Build shared library" ON)
option(BUILD_TEST "Build test" OFF)
option(WITH_COTIRE "Use cotire to create precompiled header before build" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(use_gold_linker)
if (WITH_COTIRE)
  include(cotire)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  set(BUILD_TEST ON)
endif()

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
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

# compiler requirements
if (MSVC)
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.14)
    message(WARNING "MSVC < 19.14 is currently not supported. Some errors may be caused during build")
  endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.1)
    message(WARNING "GCC < 8.1 is currently not supported. Some errors may be caused during build")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5.0)
    message(WARNING "Clang < 3.5.0 is currently not supported. Some errors may be caused during build")
  endif()
else()
  message(WARNING "Your compiler is currently not supported. Some errors may be caused during build")
endif()

# enable C++17 support
if (CMAKE_VERSION VERSION_LESS "3.8.2")
  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.1)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    endif()
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0.0)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    endif()
  else()
    message(FATAL_ERROR "Your cmake version is not supported. Please update cmake.")
  endif()
else()
  set(CMAKE_CXX_STANDARD 17)
endif()

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

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
  add_definitions("-DCPPGLOB_COVERAGE")
endif()

# Use libc++ in Mac OSX
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

include_directories(include)

add_subdirectory(src)

if(BUILD_TEST)
  enable_testing()
  add_subdirectory(test)
endif()

install(
  DIRECTORY include/cppglob
  DESTINATION include
  COMPONENT headers)

install(
  EXPORT cppglob-config
  DESTINATION share/cmake/cppglob)

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake
)
