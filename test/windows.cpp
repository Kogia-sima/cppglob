#ifdef CPPGLOB_BUILDING
#  undef CPPGLOB_BUILDING
#endif

#include <cppglob/config.hpp>

#ifdef CPPGLOB_IS_WINDOWS

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
    fs::create_directory(L"cppglob_test_tmpdir");
    fs::current_path(L"cppglob_test_tmpdir");
  }

  ~test_in_dir() {
    fs::current_path(old_dir);
    fs::remove_all(L"cppglob_test_tmpdir");
  }
};

void create_file(const wchar_t* str) {
  FILE* fp = _wfopen(str, L"w");
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
  std::vector<fs::path> dirs = {L"a/", L"a/b/", L"a/b/c/"};
  cppglob::glob_iterator it(dirs), end;
  CHECK_EQ(std::distance(it, end), 3);
  CHECK_EQ((*it), fs::path(L"a/"));
  it++;
  CHECK_EQ(*it, fs::path(L"a/b/"));

  const auto it2 = std::move(it);
  CHECK_EQ(std::distance(it2, end), 2);
  CHECK_EQ(*it2, fs::path(L"a/b/"));

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
  CHECK_EQ(cppglob::translate(L"*"), L".*");
  CHECK_EQ(cppglob::translate(L"*.*"), L".*\\..*");
  CHECK_EQ(cppglob::translate(L"file[1-9].txt"), L"file[1-9]\\.txt");
  CHECK_EQ(cppglob::translate(L"file[!1-9].txt"), L"file[^1-9]\\.txt");
  CHECK_EQ(cppglob::translate(L"file[^1-9\\].txt"), L"file[\\1-9\\\\]\\.txt");
  CHECK_EQ(cppglob::translate(L"[]-+{}()?$.a"), LR"(\[\]\-\+\{\}\(\).\$\.a)");
}

TEST_CASE("filter() function") {
  std::vector<fs::path> names{L".\\\\\\.\\a\\b", L".\\a\\..\\a\\b", L"apple\\"};
  std::wstring_view pattern{L".\\a\\*"};
  cppglob::filter(names, pattern);

  REQUIRE_EQ(names.size(), 2L);

  CHECK_EQ(names[0], fs::path(L".\\\\\\.\\a\\b"));
  CHECK_EQ(names[1], fs::path(L".\\a\\..\\a\\b"));
}

TEST_CASE("iglob() function") {
  test_in_dir _;

  REQUIRE(fs::create_directories(L"apple"));
  REQUIRE(fs::create_directories(L"banana"));
  create_file(L"apple\\apple.txt");
  create_file(L"banana\\apple.txt");

  cppglob::glob_iterator it = cppglob::iglob(L".\\*"), end;
  REQUIRE_NE(it, end);
  REQUIRE_EQ(end, cppglob::iglob());

  std::vector<fs::path> vec(it, end);
  unorderd_compare_results(vec, {L".\\apple", L".\\banana"});

  cppglob::glob_iterator it2 = cppglob::iglob(L"**", true);
  vec = std::vector<fs::path>(it2, end);

  unorderd_compare_results(vec, {L"apple", L"banana", L"apple\\apple.txt",
                                 L"banana\\apple.txt"});
}

TEST_CASE("glob() function") {
  test_in_dir _;

  REQUIRE(fs::create_directories(L"a\\b\\c\\.d"));
  REQUIRE(fs::create_directories(L"e"));

  // create file
  create_file(L"f.txt");
  create_file(L"a\\g.txt");
  create_file(L"a\\h.txt");
  create_file(L"e\\g.txt");

  std::vector<fs::path> vec = cppglob::glob(L"a\\g.txt");
  unorderd_compare_results(vec, {L"a\\g.txt"});

  vec = cppglob::glob(L"a\\b\\");
  unorderd_compare_results(vec, {L"a\\b\\"});

  vec = cppglob::glob(L"a\\b\\c\\.*");
  unorderd_compare_results(vec, {});

  vec = cppglob::glob(L"j.txt");
  unorderd_compare_results(vec, {});

  // platform dependent
  // vec = cppglob::glob(L".\\.\\\\\\\\.\\*");
  // std::vector<fs::path> corrects = {L".\\.\\\\\\\\.\\a",
  //                                   L".\\.\\\\\\\\.\\f.txt",
  //                                   L".\\.\\\\\\\\.\\e"};
  // unorderd_compare_results(vec, corrects);

  vec = cppglob::glob(L".\\a\\..\\a\\b\\..\\*\\");
  unorderd_compare_results(vec, {L".\\a\\..\\a\\b\\..\\b\\"});

  vec = cppglob::glob(L"a\\?.*");
  std::vector<fs::path> corrects = {L"a\\g.txt", L"a\\h.txt"};
  unorderd_compare_results(vec, corrects);

  vec = cppglob::glob(L"*\\g.txt");
  corrects = {L"a\\g.txt", L"e\\g.txt"};
  unorderd_compare_results(vec, corrects);

  vec = cppglob::glob(L"*\\i.txt");
  unorderd_compare_results(vec, {});
}

TEST_CASE("recursive glob") {
  test_in_dir _;

  REQUIRE(fs::create_directories(L"a\\b\\c"));

  // create file
  create_file(L"d.txt");
  create_file(L"a\\e.txt");
  create_file(L"a\\f.txt");
  create_file(L"a\\b\\g.txt");

  std::vector<fs::path> vec = cppglob::glob(L"a\\**\\*.txt", true);
  std::vector<fs::path> corrects = {L"a\\e.txt", L"a\\f.txt", L"a\\b\\g.txt"};

  unorderd_compare_results(vec, corrects);

  vec = cppglob::glob(L"**", true);
  corrects = {L"a",        L"a\\b",     L"a\\b\\c",    L"d.txt",
              L"a\\e.txt", L"a\\f.txt", L"a\\b\\g.txt"};

  unorderd_compare_results(vec, corrects);
}

TEST_CASE("escape() function") {
  CHECK_EQ(cppglob::escape(L"*"), fs::path(L"[*]"));
  CHECK_EQ(cppglob::escape(L"*.*"), fs::path(L"[*].[*]"));
  CHECK_EQ(cppglob::escape(L"file[1-9].txt"), fs::path(L"file[[]1-9].txt"));
  CHECK_EQ(cppglob::escape(L"file[!1-9].txt"), fs::path(L"file[[]!1-9].txt"));
  CHECK_EQ(cppglob::escape(L"file[^1-9\\].txt"),
            fs::path(L"file[[]^1-9\\].txt"));
  CHECK_EQ(cppglob::escape("[]-+{}()?$.a"), fs::path(L"[[]]-+{}()[?]$.a"));
}

#endif
