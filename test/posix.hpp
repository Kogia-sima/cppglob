#include <cstdio>
#include <algorithm>
#include <string_view>
#include <stdexcept>
#include <gtest/gtest.h>

#include <cppglob/fnmatch.hpp>
#include <cppglob/glob.hpp>
#include <cppglob/iglob.hpp>

namespace fs = std::filesystem;

struct test_in_dir {
  fs::path old_dir;

  test_in_dir() : old_dir(fs::current_path()) {
    fs::create_directory("cppglob_test_tmpdir");
    fs::current_path("cppglob_test_tmpdir");
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

TEST(glob_iterator, generic) {
  std::vector<fs::path> dirs = {"a/", "a/b/", "a/b/c/"};
  cppglob::glob_iterator it(dirs), end;
  EXPECT_EQ(std::distance(it, end), 3);
  EXPECT_EQ((*it).native(), "a/");
  it++;
  EXPECT_EQ(it->native(), "a/b/");

  const auto it2 = std::move(it);
  EXPECT_EQ(std::distance(it2, end), 2);
  EXPECT_EQ((*it2).native(), "a/b/");
  EXPECT_EQ(it2->native(), "a/b/");

  EXPECT_NE(it2, end);

  cppglob::glob_iterator it3 = it2;
  it3.swap(it);

  ++it;
  it++;

  EXPECT_TRUE(it == end);

  cppglob::glob_iterator it4(std::move(dirs));
  it3 = std::move(it4);
  EXPECT_EQ(std::distance(it3, end), 3);
}

TEST(fnmatch, translate) {
  EXPECT_EQ(cppglob::translate("*"), ".*");
  EXPECT_EQ(cppglob::translate("*.*"), ".*\\..*");
  EXPECT_EQ(cppglob::translate("file[1-9].txt"), "file[1-9]\\.txt");
  EXPECT_EQ(cppglob::translate("file[!1-9].txt"), "file[^1-9]\\.txt");
  EXPECT_EQ(cppglob::translate("file[^1-9\\].txt"), "file[\\1-9\\\\]\\.txt");
  EXPECT_EQ(cppglob::translate("[]-+{}()?$.a"), R"(\[\]\-\+\{\}\(\).\$\.a)");
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
  test_in_dir _;

  ASSERT_TRUE(fs::create_directories("apple"));
  ASSERT_TRUE(fs::create_directories("banana"));
  create_file("apple/apple.txt");
  create_file("banana/apple.txt");

  cppglob::glob_iterator it = cppglob::iglob("./*"), end;
  ASSERT_NE(it, end);
  ASSERT_EQ(end, cppglob::iglob());

  std::vector<fs::path> vec(it, end);
  unorderd_compare_results(vec, {"./apple", "./banana"});

  cppglob::glob_iterator it2 = cppglob::iglob("**", true);
  vec = std::vector<fs::path>(it2, end);

  unorderd_compare_results(
      vec, {"apple", "banana", "apple/apple.txt", "banana/apple.txt"});
}

TEST(glob, glob) {
  test_in_dir _;

  ASSERT_TRUE(fs::create_directories("a/b/c/.d"));
  ASSERT_TRUE(fs::create_directories("e"));

  // create file
  create_file("f.txt");
  create_file("a/g.txt");
  create_file("a/h.txt");
  create_file("e/g.txt");

  std::vector<fs::path> vec = cppglob::glob("a/g.txt");
  unorderd_compare_results(vec, {"a/g.txt"});

  vec = cppglob::glob("a/b/");
  unorderd_compare_results(vec, {"a/b/"});

  vec = cppglob::glob("a/b/c/.*");
  unorderd_compare_results(vec, {});

  vec = cppglob::glob("j.txt");
  unorderd_compare_results(vec, {});

  // platform dependent
  // vec = cppglob::glob("././///./*");
  // std::vector<fs::path> corrects = {"./././a", "./././f.txt", "./././e"};
  // unorderd_compare_results(vec, corrects);

  vec = cppglob::glob("./a/../a/b/../*/");
  unorderd_compare_results(vec, {"./a/../a/b/../b/"});

  vec = cppglob::glob("a/?.*");
  std::vector<fs::path> corrects = {"a/g.txt", "a/h.txt"};
  unorderd_compare_results(vec, corrects);

  vec = cppglob::glob("*/g.txt");
  corrects = {"a/g.txt", "e/g.txt"};
  unorderd_compare_results(vec, corrects);

  vec = cppglob::glob("*/i.txt");
  unorderd_compare_results(vec, {});
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

  vec = cppglob::glob("**", true);
  corrects = {"a", "a/b", "a/b/c", "d.txt", "a/e.txt", "a/f.txt", "a/b/g.txt"};

  unorderd_compare_results(vec, corrects);
}

TEST(glob, escape) {
  EXPECT_EQ(cppglob::escape("*").native(), "[*]");
  EXPECT_EQ(cppglob::escape("*.*").native(), "[*].[*]");
  EXPECT_EQ(cppglob::escape("file[1-9].txt").native(), "file[[]1-9].txt");
  EXPECT_EQ(cppglob::escape("file[!1-9].txt"), "file[[]!1-9].txt");
  EXPECT_EQ(cppglob::escape("file[^1-9\\].txt").native(), "file[[]^1-9\\].txt");
  EXPECT_EQ(cppglob::escape("[]-+{}()?$.a").native(), "[[]]-+{}()[?]$.a");
}
