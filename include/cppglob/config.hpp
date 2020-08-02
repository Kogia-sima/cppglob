/**
 * @file cppglob/fnmatch.hpp
 * @brief Filename matching with shell patterns
 * @copyright 2018 Ryohei Machida
 */

#ifndef CPPGLOB_CONFIG_HPP
#define CPPGLOB_CONFIG_HPP

#if !defined(_MSC_VER) && __cplusplus < 201703
#  error This file requires compiler and library support \
for the ISO C++ 2017 standard. This support must be enabled \
with the -std=c++17 or -std=gnu++17 compiler options.
#endif

#include <filesystem>

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <cwchar>
#  define CStr(x) L##x
#  define CPPGLOB_IS_WINDOWS 1
#else
#  define CStr(x) x
#endif

#ifdef CPPGLOB_COVERAGE
#  define CPPGLOB_EXPORT __attribute__((visibility("default")))
#  define CPPGLOB_LOCAL
#  define CPPGLOB_INLINE CPPGLOB_LOCAL
#elif (defined _WIN32 || defined WINCE || defined __CYGWIN__)
#  if defined(CPPGLOB_STATIC)
#    define CPPGLOB_EXPORT
#  else
#    ifdef CPPGLOB_BUILDING
#      define CPPGLOB_EXPORT __declspec(dllexport)
#    else
#      define CPPGLOB_EXPORT __declspec(dllimport)
#    endif
#	endif
#  define CPPGLOB_LOCAL
#  define CPPGLOB_INLINE inline
#elif defined __GNUC__ && __GNUC__ >= 4
#  define CPPGLOB_EXPORT __attribute__((visibility("default")))
#  define CPPGLOB_LOCAL __attribute__((visibility("hidden")))
#  define CPPGLOB_INLINE __attribute__((visibility("hidden"))) inline
#else
#  define CPPGLOB_EXPORT
#  define CPPGLOB_LOCAL
#  define CPPGLOB_INLINE inline
#endif


namespace cppglob {
  namespace fs = std::filesystem;
}  // namespace cppglob

#endif
