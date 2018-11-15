#include <cstdint>
#include <string>
#include <string_view>
#include <regex>
#include <filesystem>
#include <cppglob/fnmatch.hpp>

namespace cppglob {
  namespace detail {
    CPPGLOB_INLINE std::regex compile_pattern(const std::string_view& pat) {
      return std::regex(translate(pat));
    }

    CPPGLOB_INLINE std::string replace_all(const std::string_view& str,
                            const std::string_view& from,
                            const std::string_view& to) {
      std::string res;
      res.reserve(str.size());

      std::string::size_type pos_start = 0L;

      while (true) {
        std::string::size_type pos = str.find(from, pos_start);

        if (pos == std::string_view::npos) {
          res += str.substr(pos_start);
          break;
        }

        res += str.substr(pos_start, pos - pos_start);
        res += to;

        pos_start = pos + from.size();
      }

      return res;
    }

    CPPGLOB_INLINE bool should_be_escaped(char c) {
      static const char special_chars[] = R"([]-{}()*+?.\^$|)";
      for (const char& special_char : special_chars) {
        if (c == special_char) {
          return true;
        }
      }
      return false;
    }

    CPPGLOB_INLINE std::string normpath(const std::string_view& p) {
      return fs::path(p).lexically_normal().native();
    }
  }  // namespace detail

  void filter(std::vector<fs::path>& names, const std::string_view& pat) {
    std::string pat_str = detail::normpath(pat);
    std::regex re =
        detail::compile_pattern(std::string_view(&pat_str[0], pat_str.size()));
    auto filter_fn = [&](std::vector<fs::path>::value_type& p) {
      return !std::regex_match(p.lexically_normal().native(), re);
    };

    auto result = std::remove_if(names.begin(), names.end(), filter_fn);
    names.erase(result, names.end());
  }

  std::string translate(const std::string_view& pat) {
    std::size_t i = 0L, n = pat.size();
    std::string res;
    res.reserve(pat.size());

    while (i < n) {
      auto c = pat[i];
      ++i;

      if (c == '*') {
        res += ".*";
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
          res += "\\[";
        } else {
          std::string stuff = detail::replace_all(
              std::string_view(&pat[i], j - i), "\\", "\\\\");
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
