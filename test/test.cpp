#include <cstdio>
#include <string_view>
#include <stdexcept>
#include <gtest/gtest.h>

#include <cppglob/fnmatch.hpp>
#include <cppglob/glob.hpp>
#include <cppglob/glob_iterator.hpp>

namespace fs = std::filesystem;

struct test_in_dir {
  fs::path old_dir;

  test_in_dir() : old_dir(fs::current_path()) {
    fs::create_directory("cppglob_test_tmpdir");
    fs::current_path("./cppglob_test_tmpdir");
  }

  ~test_in_dir() {
    fs::current_path(old_dir);
    fs::remove_all("cppglob_test_tmpdir");
  }
};

void create_file(const char* str) {
  FILE* fp = fopen(str, "w");
  if (fp == nullptr) {
    throw std::runtime_error("Failed to create file.");
  }
  fclose(fp);
}

void unorderd_compare_results(const std::vector<fs::path>& actual,
                              const std::vector<fs::path>& corrects) {
  ASSERT_EQ(actual.size(), corrects.size());

  for (auto&& elem : actual) {
    bool matched = false;

    for (auto&& elem2 : corrects) {
      if (elem.native() == elem2.native()) {
        matched = true;
        break;
      }
    }
    EXPECT_TRUE(matched);
  }
}

TEST(fnmatch, translate) {
  EXPECT_EQ(cppglob::translate("*"), ".*");
  EXPECT_EQ(cppglob::translate("*.*"), ".*\\..*");
  EXPECT_EQ(cppglob::translate("file[1-9].txt"), "file[1-9]\\.txt");
  EXPECT_EQ(cppglob::translate("file[!1-9].txt"), "file[^1-9]\\.txt");
  EXPECT_EQ(cppglob::translate("file[^1-9\\].txt"), "file[\\1-9\\\\]\\.txt");
  EXPECT_EQ(cppglob::translate("[-+{}()?[$.a"), R"(\[\-\+\{\}\(\).\[\$\.a)");
}

TEST(fnmatch, filter) {
  std::vector<fs::path> names{".///./a/b", "./a/../a/b", "apple/"};
  std::string_view pattern{"./a/*"};
  cppglob::filter(names, pattern);

  ASSERT_EQ(names.size(), 2L);

  EXPECT_EQ(names[0].native(), ".///./a/b");
  EXPECT_EQ(names[1].native(), "./a/../a/b");
}

TEST(glob, iglob) {
  cppglob::glob_iterator it = cppglob::iglob("./*"), end;
  ASSERT_NE(it, end);

  std::vector<fs::path> vec(it, end);
  EXPECT_GT(vec.size(), 0L);

  for (auto&& elem : vec) {
    ASSERT_FALSE(elem.empty());
  }
}

TEST(glob, glob) {
  test_in_dir _;

  ASSERT_TRUE(fs::create_directories("a/b/c"));

  // create file
  create_file("d.txt");
  create_file("a/e.txt");
  create_file("a/f.txt");

  std::vector<fs::path> vec = cppglob::glob("a/e.txt");
  unorderd_compare_results(vec, {"a/e.txt"});

  vec = cppglob::glob("././///./*");
  std::vector<fs::path> corrects = {"./././a", "./././d.txt"};
  unorderd_compare_results(vec, corrects);

  vec = cppglob::glob("./a/../a/b/../*/");
  unorderd_compare_results(vec, {"./a/../a/b/../b/"});

  vec = cppglob::glob("a/?.*");
  corrects = {"a/e.txt", "a/f.txt"};
  unorderd_compare_results(vec, corrects);
}

TEST(glob, recursive) {
  test_in_dir _;

  ASSERT_TRUE(fs::create_directories("a/b/c"));

  // create file
  create_file("d.txt");
  create_file("a/e.txt");
  create_file("a/f.txt");
  create_file("a/b/g.txt");

  std::vector<fs::path> vec = cppglob::glob("a/**/*.txt", true);
  std::vector<fs::path> corrects = {"a/e.txt", "a/f.txt", "a/b/g.txt"};

  unorderd_compare_results(vec, corrects);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
