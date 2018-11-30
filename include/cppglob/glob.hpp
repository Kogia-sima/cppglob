/**
 * @file cppglob/glob.hpp
 * @brief glob() function declaration
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

#ifndef CPPGLOB_GLOB_HPP
#define CPPGLOB_GLOB_HPP

#include <vector>
#include "config.hpp"

namespace cppglob {
  /**
   * @brief Return a list of paths matching a pathname pattern.
   * @param pathname pattern string
   * @param recursive allow recursive pattern string
   *
   * The pattern may contain simple shell-style wildcards a la
   * fnmatch. However, unlike fnmatch, filenames starting with a
   * dot are special cases that are not matched by '*' and '?'
   * patterns.
   *
   * If recursive is true, the pattern '**' will match any files and
   * zero or more directories and subdirectories.
   */
  CPPGLOB_EXPORT std::vector<fs::path> glob(const fs::path& pathname,
                                            bool recursive = false);

  /**
   * @brief Escape all special characters.
   * @param pathname pattern string to be escaped
   */
  CPPGLOB_EXPORT fs::path escape(const fs::path& pathname);
}  // namespace cppglob

#endif
