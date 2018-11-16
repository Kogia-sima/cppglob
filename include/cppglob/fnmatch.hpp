#ifndef CPPGLOB_FNMATCH_HPP
#define CPPGLOB_FNMATCH_HPP

#include <string>
#include <string_view>
#include <vector>
#include "config.hpp"

namespace cppglob {
#ifndef CPPGLOB_IS_WINDOWS
  using char_type = char;
  using string_type = std::string;
  using string_view_type = std::string_view;
#else
  using char_type = wchar_t;
  using string_type = std::wstring;
  using string_view_type = std::wstring_view;
#endif

  /**
   * @brief returns the subset of the vector names that matches pat
   * @param names vector of file names
   * @param pattern string
   */
  CPPGLOB_EXPORT void filter(std::vector<fs::path>& names,
                             const string_view_type& pat);

  /**
   * @brief translate shell PATTERN to regular expression
   * @param pat patten string
   *
   * There is no way to quote meta-characters
   */
  CPPGLOB_EXPORT string_type translate(const string_view_type& pat);
}  // namespace cppglob

#endif
