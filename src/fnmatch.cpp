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
#include <string>
#include <string_view>
#include <regex>
#include <filesystem>
#include <cppglob/fnmatch.hpp>

namespace cppglob {
  using regex_type = std::basic_regex<char_type>;

  namespace detail {
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

  void filter(std::vector<fs::path>& names, const string_view_type& pat) {
    string_type pat_str = detail::normpath(pat);
    regex_type re =
        detail::compile_pattern(string_view_type(&pat_str[0], pat_str.size()));
    auto filter_fn = [&](std::vector<fs::path>::value_type& p) -> bool {
      return !std::regex_match(p.lexically_normal().native(), re);
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
