#include <gtest/gtest.h>

#include <cppglob/config.hpp>

#ifndef CPPGLOB_IS_WINDOWS
#include "posix.hpp"
#else
#include "windows.hpp"
#endif

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
