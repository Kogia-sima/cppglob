/**
 * @file cppglob/glob_iterator.hpp
 * @brief glob_iterator class declaration
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

#ifndef CPPGLOB_GLOB_ITERATOR_HPP
#define CPPGLOB_GLOB_ITERATOR_HPP

#include <cstddef>
#include <iterator>
#include <vector>
#include <filesystem>
#include "config.hpp"

namespace cppglob {
  class CPPGLOB_LOCAL glob_iterator {
    using base = std::vector<fs::path>::iterator;

   public:
    using difference_type = typename base::difference_type;
    using value_type = typename base::value_type;
    using pointer = typename base::pointer;
    using const_pointer = const typename fs::path*;
    using reference = typename fs::path&;
    using const_reference = const typename fs::path&;
    using iterator_category = std::forward_iterator_tag;

    glob_iterator() noexcept = default;

    explicit glob_iterator(const std::vector<fs::path>& pathnames)
        : M_pathnames(pathnames) {}

    explicit glob_iterator(std::vector<fs::path>&& pathnames) noexcept
        : M_pathnames(std::move(pathnames)) {}

    glob_iterator(const glob_iterator& other) = default;

    glob_iterator(glob_iterator&& other) noexcept
        : M_pathnames(std::move(other.M_pathnames)) {
      M_index = other.M_index;
    }

    glob_iterator& operator=(const glob_iterator& other) = default;

    glob_iterator& operator=(glob_iterator&& other) {
      M_pathnames = std::move(other.M_pathnames);
      M_index = other.M_index;
      return *this;
    }

    reference operator*() { return M_pathnames[M_index]; }
    const_reference operator*() const { return M_pathnames[M_index]; }

    pointer operator->() { return &M_pathnames[M_index]; }
    const_pointer operator->() const { return &M_pathnames[M_index]; }

    bool operator==(const glob_iterator& other) const {
      return (finished() && other.finished()) ||
             (!finished() && !other.finished() && M_index == other.M_index);
    }

    bool operator!=(const glob_iterator& other) const {
      return !(*this == other);
    }

    glob_iterator& operator++() {
      ++M_index;
      return *this;
    }

    glob_iterator operator++(int) {
      glob_iterator old(*this);
      ++M_index;
      return old;
    }

    glob_iterator& swap(glob_iterator& other) {
      std::swap(M_pathnames, other.M_pathnames);
      std::swap(M_index, other.M_index);
      return *this;
    }

   protected:
    bool finished() const {
      return M_pathnames.empty() || M_index >= M_pathnames.size();
    }

   private:
    std::vector<fs::path> M_pathnames;
    std::size_t M_index = 0L;
  };
}  // namespace cppglob

#endif
