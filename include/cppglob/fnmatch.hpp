#ifndef CPPGLOB_FNMATCH_CPP
#define CPPGLOB_FNMATCH_CPP

#include <string>
#include <vector>
#include "config.hpp"

namespace cppglob {
  /**
   * @brief returns the subset of the vector names that matches pat
   * @param names vector of file names
   * @param pattern string
   */
  CPPGLOB_EXPORT void filter(std::vector<fs::path>& names,
                             const std::string_view& pat);

  /**
   * @brief translate shell PATTERN to regular expression
   * @param pat patten string
   *
   * There is no way to quote meta-characters
   */
  CPPGLOB_EXPORT std::string translate(const std::string_view& pat);
}  // namespace cppglob

#endif
