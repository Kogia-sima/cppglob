#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <cppglob/fnmatch.hpp>
#include <cppglob/glob.hpp>
#include <cppglob/glob_iterator.hpp>

namespace cppglob {
  using regex_type = std::basic_regex<char_type>;

  static regex_type magic_check(CStr("([*?[])"));

  namespace detail {
    CPPGLOB_INLINE bool has_magic(const string_type& str) {
      return std::regex_search(str, magic_check);
    }

    CPPGLOB_INLINE bool ishidden(const fs::path& pathname) {
      return pathname.native()[0] == '.';
    }

    CPPGLOB_INLINE bool isrecursive(const fs::path& pathname) {
      return pathname.native() == CStr("**");
    }

    CPPGLOB_INLINE std::vector<fs::path> iterdir(const fs::path& dirname,
                                                 bool dironly) {
      fs::directory_iterator files{(dirname.empty()) ? fs::current_path() :
                                                       dirname};

      std::vector<fs::path> ret;

      for (auto&& file : files) {
        if (!dironly || fs::is_directory(file)) {
          ret.push_back(file.path().filename());
        }
      }
      return ret;
    }

    CPPGLOB_INLINE std::vector<fs::path> rlistdir(const fs::path& dirname,
                                                  bool dironly) {
      std::vector<fs::path> ret;

      for (auto&& x : iterdir(dirname, dironly)) {
        if (!ishidden(x)) {
          ret.push_back(x);
          fs::path path = (dirname.empty()) ? x : (dirname / x);

          if (fs::is_directory(path)) {
            for (auto&& y : rlistdir(path, dironly)) {
              ret.push_back(x / y);
            }
          }
        }
      }

      return ret;
    }

    CPPGLOB_INLINE std::vector<fs::path> glob0(const fs::path& dirname,
                                               const fs::path& basename,
                                               bool dironly) {
      if (basename.empty()) {
        if (fs::is_directory(dirname)) {
          return std::vector<fs::path>({basename});
        }
      } else if (fs::exists(dirname / basename)) {
        return std::vector<fs::path>({basename});
      }

      return std::vector<fs::path>();
    }

    CPPGLOB_INLINE std::vector<fs::path> glob1(const fs::path& dirname,
                                               const fs::path& pattern,
                                               bool dironly) {
      std::vector<fs::path> names = iterdir(dirname, dironly);

      if (ishidden(pattern)) {
        auto result =
            std::remove_if(names.begin(), names.end(),
                           [](fs::path& p) -> bool { return ishidden(p); });
        names.erase(result, names.end());
      }

      const string_type& pattern_str = pattern.native();
      ::cppglob::filter(names,
                        string_view_type(&pattern_str[0], pattern_str.size()));
      return names;
    }

    CPPGLOB_INLINE std::vector<fs::path> glob2(const fs::path& dirname,
                                               const fs::path& pattern,
                                               bool dironly) {
      assert(isrecursive(pattern));
      std::vector<fs::path> result = rlistdir(dirname, dironly);
      result.push_back(std::move(result.front()));
      result[0] = fs::path("");

      return result;
    }

    CPPGLOB_INLINE std::vector<fs::path> iglob(const fs::path& pathname,
                                               bool recursive, bool dironly) {
      fs::path dirname = pathname.parent_path();
      fs::path basename = pathname.filename();

      if (!has_magic(pathname)) {
        assert(!dironly);
        if (!basename.empty()) {
          if (fs::exists(pathname)) {
            return std::vector<fs::path>({pathname});
          }
        } else {
          if (fs::is_directory(dirname)) {
            return std::vector<fs::path>({pathname});
          }
        }

        return std::vector<fs::path>();
      }

      if (dirname.empty()) {
        if (recursive && isrecursive(basename)) {
          return glob2(dirname, basename, dironly);
        } else {
          return glob1(dirname, basename, dironly);
        }
      }

      std::vector<fs::path> dirs;
      if (dirname != pathname && has_magic(dirname)) {
        dirs = iglob(dirname, recursive, true);
      } else {
        dirs.push_back(dirname);
      }

      std::vector<fs::path> (*glob_in_dir)(const fs::path&, const fs::path&,
                                           bool);
      if (has_magic(basename)) {
        if (recursive && isrecursive(basename)) {
          glob_in_dir = glob2;
        } else {
          glob_in_dir = glob1;
        }
      } else {
        glob_in_dir = glob0;
      }

      std::vector<fs::path> files;
      for (auto&& dirname : dirs) {
        for (auto&& name : glob_in_dir(dirname, basename, dironly)) {
          files.emplace_back(dirname / name);
        }
      }

      return files;
    }
  }  // namespace detail

  std::vector<fs::path> glob(const fs::path& pathname, bool recursive) {
    std::vector<fs::path> files = detail::iglob(pathname, recursive, false);
    if (recursive && detail::isrecursive(pathname)) {
      assert(files.size() > 0L);

      // remove first value
      files[0] = std::move(files.back());
      files.pop_back();
    }

    return files;
  }

  glob_iterator iglob(const fs::path& pathname, bool recursive) {
    std::vector<fs::path> files = detail::iglob(pathname, recursive, false);
    glob_iterator it(std::move(files));

    if (recursive && detail::isrecursive(pathname)) {
      ++it;
    }

    return it;
  }

  glob_iterator iglob() { return glob_iterator(); }
}  // namespace cppglob
