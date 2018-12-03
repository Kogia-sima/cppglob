/*
 * copyright: 2018 Ryohei Machida
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <cppglob/fnmatch.hpp>
#include <cppglob/glob.hpp>
#include <cppglob/iglob.hpp>

namespace cppglob {
  namespace detail {
    CPPGLOB_INLINE bool has_magic(const string_type& str) {
      static const string_view_type magics = CStr("*?[");
      for (const char& c : str) {
        for (const char& magic : magics) {
          if (c == magic) {
            return true;
          }
        }
      }

      return false;
    }

    CPPGLOB_INLINE bool ishidden(const fs::path& pathname) {
      return pathname.native()[0] == '.';
    }

    CPPGLOB_INLINE bool isrecursive(const fs::path& pathname) {
      return pathname.native() == CStr("**");
    }

    CPPGLOB_INLINE std::vector<fs::path> iterdir(const fs::path& dirname,
                                                 bool dironly) {
      fs::path base_dir = (dirname.empty()) ? fs::current_path() : dirname;

      if (!fs::is_directory(base_dir)) {
        return std::vector<fs::path>();
      }

      fs::directory_iterator files(base_dir);

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

      if (result.empty()) {
        result.emplace_back("");
      } else {
        result.push_back(std::move(result.front()));
        result[0] = fs::path("");
      }

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

#ifdef CPPGLOB_IS_WINDOWS
    CPPGLOB_INLINE string_view_type drive_name(const fs::path& pathname) {
      const string_type& path_str = pathname.native();
      if (path_str.size() == 2 && path_str[1] == L':') {
        return string_view_type(&path_str[0], 2);
      }
      if (path_str.size() >= 3) {
        const auto is_sep = [](const char_type& c) { c == L'\\' || c == L'/'; };
        if (path_str.find_first_not_of(L"\\/") == 2) {
          // is a UNC path:
          // vvvvvvvvvvvvvvvvvvvv drive letter or UNC path
          // \\machine\mountpoint\directory\etc\...
          //           directory ^^^^^^^^^^^^^^^
          const auto index = path_str.find_first_of(L"\\/", 3);
          if (index == string_type::npos) {
            return string_view_type();
          }
          auto index2 = path_str.find_first_of(L"\\/", index + 1);
          // a UNC path can't have two slashes in a row
          // (after the initial two)
          if (index2 == index + 1) {
            return string_view_type();
          }
          if (index2 == string_type::npos) {
            index2 = path_str.size();
          }
          return string_view_type(&path_str[0], index2);
        }
      }
      return string_view_type();
    }
#endif

    CPPGLOB_INLINE string_type& escape_magic(const string_view_type& pathname,
                                             string_type& output) {
      static const auto is_magic = [](const char_type& c) -> bool {
        return c == CStr('*') || c == CStr('?') || c == CStr('[');
      };

      output.reserve(output.size() + pathname.size());

      for (const char_type& c : pathname) {
        if (is_magic(c)) {
          output.push_back('[');
          output.push_back(c);
          output.push_back(']');
        } else {
          output.push_back(c);
        }
      }
      return output;
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

  fs::path escape(const fs::path& pathname) {
#ifndef CPPGLOB_IS_WINDOWS
    const string_view_type main_path{&pathname.native()[0],
                                     pathname.native().size()};
    string_type output;
#else
    const string_view_type drive = detail::drive_name(pathname);
    string_type output(drive);
    const string_view_type main_path{&pathname.native()[drive.size()],
                                     pathname.native().size() - drive.size()};
#endif
    detail::escape_magic(main_path, output);
    return fs::path(std::move(output));
  }
}  // namespace cppglob
