#ifdef CPPGLOB_BUILDING
#  undef CPPGLOB_BUILDING
#endif

#include <cppglob/config.hpp>

#ifndef CPPGLOB_IS_WINDOWS

#include <cstdio>
#include <algorithm>
#include <string_view>
#include <stdexcept>
#include <filesystem>

#include <cppglob/fnmatch.hpp>
#include <cppglob/glob.hpp>
#include <cppglob/iglob.hpp>
#include "doctest.h"

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
  REQUIRE_EQ(actual.size(), corrects.size());

  for (auto&& elem : actual) {
    bool matched = false;

    for (auto&& elem2 : corrects) {
      if (elem == elem2) {
        matched = true;
        break;
      }
    }
    CHECK_MESSAGE(matched, elem << " doesn't exist in expected name list.");
  }
}

TEST_CASE("glob_iterator manipulation") {
  std::vector<fs::path> dirs = {"a/", "a/b/", "a/b/c/"};
  cppglob::glob_iterator it(dirs), end;
  CHECK_EQ(std::distance(it, end), 3);
  CHECK_EQ((*it), fs::path("a/"));
  it++;
  CHECK_EQ(*it, fs::path("a/b/"));

  const auto it2 = std::move(it);
  CHECK_EQ(std::distance(it2, end), 2);
  CHECK_EQ(*it2, fs::path("a/b/"));

  CHECK_NE(it2, end);

  cppglob::glob_iterator it3 = it2;
  it3.swap(it);

  ++it;
  it++;

  CHECK_EQ(it, end);

  cppglob::glob_iterator it4(std::move(dirs));
  it3 = std::move(it4);
  CHECK_EQ(std::distance(it3, end), 3);
}

TEST_CASE("translate() function") {
  CHECK_EQ(cppglob::translate("*"), ".*");
  CHECK_EQ(cppglob::translate("*.*"), ".*\\..*");
  CHECK_EQ(cppglob::translate("file[1-9].txt"), "file[1-9]\\.txt");
  CHECK_EQ(cppglob::translate("file[!1-9].txt"), "file[^1-9]\\.txt");
  CHECK_EQ(cppglob::translate("file[^1-9\\].txt"), "file[\\1-9\\\\]\\.txt");
  CHECK_EQ(cppglob::translate("[]-+{}()?$.a"), R"(\[\]\-\+\{\}\(\).\$\.a)");
}

TEST_CASE("filter() function") {
  std::vector<fs::path> names{".///./a/b", "./a/../a/b", "apple/"};
  std::string_view pattern{"./a/*"};
  cppglob::filter(names, pattern);

  REQUIRE_EQ(names.size(), 2L);

  CHECK_EQ(names[0], fs::path(".///./a/b"));
  CHECK_EQ(names[1], fs::path("./a/../a/b"));
}

TEST_CASE("iglob() function") {
  test_in_dir _;

  REQUIRE(fs::create_directories("apple"));
  REQUIRE(fs::create_directories("banana"));
  create_file("apple/apple.txt");
  create_file("banana/apple.txt");

  cppglob::glob_iterator it = cppglob::iglob("./*"), end;
  REQUIRE_NE(it, end);
  REQUIRE_EQ(end, cppglob::iglob());

  std::vector<fs::path> vec(it, end);
  unorderd_compare_results(vec, {"./apple", "./banana"});

  cppglob::glob_iterator it2 = cppglob::iglob("**", true);
  vec = std::vector<fs::path>(it2, end);

  unorderd_compare_results(
      vec, {"apple", "banana", "apple/apple.txt", "banana/apple.txt"});
}

TEST_CASE("glob() function") {
  test_in_dir _;

  REQUIRE(fs::create_directories("a/b/c/.d"));
  REQUIRE(fs::create_directories("e"));

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

TEST_CASE("recursive glob") {
  test_in_dir _;

  REQUIRE(fs::create_directories("a/b/c"));

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

TEST_CASE("escape() function") {
  CHECK_EQ(cppglob::escape("*"), fs::path("[*]"));
  CHECK_EQ(cppglob::escape("*.*"), fs::path("[*].[*]"));
  CHECK_EQ(cppglob::escape("file[1-9].txt"), fs::path("file[[]1-9].txt"));
  CHECK_EQ(cppglob::escape("file[!1-9].txt"), fs::path("file[[]!1-9].txt"));
  CHECK_EQ(cppglob::escape("file[^1-9\\].txt"), fs::path("file[[]^1-9\\].txt"));
  CHECK_EQ(cppglob::escape("[]-+{}()?$.a"), fs::path("[[]]-+{}()[?]$.a"));
}

#endif
