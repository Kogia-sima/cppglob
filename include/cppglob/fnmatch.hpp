/**
 * @file cppglob/fnmatch.hpp
 * @brief Filename matching with shell patterns
 * @copyright 2018 Ryohei Machida
 *
 * @par License
 * @parblock
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
 * @endparblock
 */

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
   * @brief Test whether name matches pattern.
   * @param name filename
   * @param pattern string
   *
   * Patterns are Unix shell style:
   *
   * *       matches everything
   * ?       matches any single character
   * [seq]   matches any character in seq
   * [!seq]  matches any char not in seq
   *
   * An initial period in FILENAME is not special.
   * Both FILENAME and PATTERN are first case-normalized
   * if the operating system requires it.
   * If you don't want this, use fnmatchcase(FILENAME, PATTERN).
   */
  CPPGLOB_EXPORT bool fnmatch(const fs::path& name,
                              const string_view_type& pat);

  /**
   * @brief returns the subset of the vector names that matches pat
   * @param names vector of file names
   * @param pattern string
   */
  CPPGLOB_EXPORT void filter(std::vector<fs::path>& names,
                             const string_view_type& pat);
}  // namespace cppglob

#endif
