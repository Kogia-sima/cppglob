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

#include <cstdint>
#include <cctype>
#include <string>
#include <string_view>
#include <regex>
#include <filesystem>
#include <cppglob/fnmatch.hpp>

namespace cppglob {
  using regex_type = std::basic_regex<char_type>;

  namespace detail {
    CPPGLOB_INLINE bool charequal(const char_type c1, const char_type c2,
                                 bool caseSensitive = true) {
      return caseSensitive ? (c1 == c2) :
                             (tolower(c2) == tolower(c1));
    }

    CPPGLOB_INLINE bool charincluded(const char_type ch,
                                     const string_view_type charlist,
                                     bool caseSensitive = true) {
      for (const char_type& ch2 : charlist) {
        if (charequal(ch, ch2, caseSensitive)) {
          return true;
        }
      }

      return false;
    }

    CPPGLOB_INLINE bool wildcmp(const string_view_type& str,
                                const string_view_type& wild,
                                bool caseSensitive) {
      string_view_type::iterator it1 = str.begin(), it2 = wild.begin();
      string_view_type::iterator cp, mp;

      while ((it1 != str.end()) && (*it2 != '*')) {
        if (*it2 == '[') {
          const auto pos = std::find(it2 + 1, wild.end(), ']');
          if (pos != wild.end()) {
            const bool is_not = *(++it2) == '!';
            const string_view_type charlist =
                wild.substr(it2 - wild.begin() + (is_not ? 1 : 0));
            if (is_not == charincluded(*it1, charlist, caseSensitive)) {
              it2 = pos;
            } else {
              return 0;
            }
          }
        } else if (!charequal(*it2, *it1, caseSensitive) && (*it2 != '?')) {
          return false;
        }
        ++it2;
        ++it1;
      }

      while (it1 != str.end()) {
        if (*it2 == '*') {
          if (++it2 == wild.end()) {
            return 1;
          }
          mp = it2;
          cp = it1 + 1;
        } else if (*it2 == '[') {
          const auto pos = std::find(it2 + 1, wild.end(), ']');
          if (pos != wild.end()) {
            const bool is_not = *(++it2) == '!';
            const string_view_type charlist =
                wild.substr(it2 - wild.begin() + (is_not ? 1 : 0));
            if (is_not == charincluded(*it1, charlist, caseSensitive)) {
              return 0;
            } else {
              it2 = pos;
            }
          }
        } else if (charequal(*it2, *it1, caseSensitive)) {
          ++it2;
          ++it1;
        } else {
          it2 = mp;   //! OCLINT parameter reassignment
          it1 = cp++;  //! OCLINT parameter reassignment
        }
      }

      while (*it2 == '*') {
        it2++;
      }
      return (*it2 == CStr('\0'));
    }

    CPPGLOB_INLINE regex_type compile_pattern(const string_view_type& pat) {
      return regex_type(translate(pat));
    }

    CPPGLOB_INLINE string_type replace_all(const string_view_type& str,
                                           const string_view_type& from,
                                           const string_view_type& to) {
      string_type res;
      res.reserve(str.size());

      string_type::size_type pos_start = 0L;

      while (true) {
        string_type::size_type pos = str.find(from, pos_start);

        if (pos == string_view_type::npos) {
          res += str.substr(pos_start);
          break;
        }

        res += str.substr(pos_start, pos - pos_start);
        res += to;

        pos_start = pos + from.size();
      }

      return res;
    }

    CPPGLOB_INLINE bool should_be_escaped(char_type c) {
      static const string_view_type special_chars = CStr(R"([]-{}()*+?.\^$|)");
      for (const char_type& special_char : special_chars) {
        if (c == special_char) {
          return true;
        }
      }
      return false;
    }

    CPPGLOB_INLINE string_type normpath(const string_view_type& p) {
      return fs::path(p).lexically_normal().native();
    }
  }  // namespace detail

  bool fnmatch(const fs::path& name, const string_view_type& pat) {
#ifndef CPPGLOB_IS_WINDOWS
    return detail::wildcmp(name.native().c_str(), pat.data(), true);
#else
    return detail::wildcmp(name.native().c_str(), pat.data(), false);
#endif
  }

  void filter(std::vector<fs::path>& names, const string_view_type& pat) {
    const string_type pat_str = detail::normpath(pat);
    const string_view_type pat_view(&pat_str[0], pat_str.size());
    auto filter_fn = [&](std::vector<fs::path>::value_type& p) -> bool {
      return !fnmatch(p.lexically_normal().native(), pat_str);
    };

    auto result = std::remove_if(names.begin(), names.end(), filter_fn);
    names.erase(result, names.end());
  }

  string_type translate(const string_view_type& pat) {
    std::size_t i = 0L, n = pat.size();
    string_type res;
    res.reserve(pat.size() + 5);

    while (i < n) {
      auto c = pat[i];
      ++i;

      if (c == '*') {
        res += CStr(".*");
      } else if (c == '?') {
        res += '.';
      } else if (c == '[') {
        std::size_t j = i;
        if (j < n && pat[j] == '!') {
          ++j;
        }
        if (j < n && pat[j] == ']') {
          ++j;
        }
        while (j < n && pat[j] != ']') {
          ++j;
        }

        if (j >= n) {
          // close parenthesis not found.
          res += CStr("\\[");
        } else {
          string_type stuff = detail::replace_all(
              string_view_type(&pat[i], j - i), CStr("\\"), CStr("\\\\"));
          i = j + 1;
          if (stuff[0] == '!') {
            stuff[0] = '^';
          } else if (stuff[0] == '^') {
            stuff[0] = '\\';
          }
          res += '[';
          res += stuff;
          res += ']';
        }
      } else {
        if (detail::should_be_escaped(c)) {
          res += '\\';
        }
        res += c;
      }
    }

    return res;
  }
}  // namespace cppglob
