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
    using reference = typename fs::path&;
    using iterator_category = std::random_access_iterator_tag;

    glob_iterator() noexcept = default;

    glob_iterator(const std::vector<fs::path>& pathnames)
        : M_pathnames(pathnames) {}

    glob_iterator(std::vector<fs::path>&& pathnames) noexcept
        : M_pathnames(std::move(pathnames)) {}

    glob_iterator(const glob_iterator& other) = default;

    glob_iterator(glob_iterator&& other) noexcept
        : M_pathnames(std::move(other.M_pathnames)) {
      M_index = other.M_index;
    }

    glob_iterator& operator=(const glob_iterator& other) {
      M_pathnames = other.M_pathnames;
      M_index = other.M_index;
      return *this;
    }

    glob_iterator& operator=(glob_iterator&& other) {
      M_pathnames = std::move(other.M_pathnames);
      M_index = other.M_index;
      return *this;
    }

    reference operator*() { return M_pathnames[M_index]; }
    auto operator*() const { return M_pathnames[M_index]; }

    pointer operator->() { return &M_pathnames[M_index]; }
    auto operator->() const { return &M_pathnames[M_index]; }

    reference operator[](difference_type idx) {
      return M_pathnames[idx + M_index];
    }
    auto operator[](difference_type idx) const {
      return M_pathnames[idx + M_index];
    }

    bool operator==(const glob_iterator& other) const {
      return (finished() && other.finished()) ||
             (!finished() && !other.finished() && M_index == other.M_index);
    }

    bool operator!=(const glob_iterator& other) const {
      return !(*this == other);
    }

    bool operator<(const glob_iterator& other) const {
      return !finished() && (other.finished() || (M_index < other.M_index));
    }

    bool operator<=(const glob_iterator& other) const {
      return other.finished() || (!finished() && M_index <= other.M_index);
    }

    bool operator>(const glob_iterator& other) const {
      return !(*this <= other);
    }

    bool operator>=(const glob_iterator& other) const {
      return !(*this < other);
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

    glob_iterator& operator--() {
      --M_index;
      return *this;
    }

    glob_iterator operator--(int) {
      glob_iterator old(*this);
      --M_index;
      return old;
    }

    glob_iterator& operator+=(difference_type offset) {
      M_index += offset;
      return *this;
    }

    glob_iterator& operator-=(difference_type offset) {
      M_index -= offset;
      return *this;
    }

    friend glob_iterator operator+(const glob_iterator& lhs,
                                   difference_type rhs) {
      glob_iterator tmp(lhs);
      tmp += rhs;
      return tmp;
    }

    friend glob_iterator operator+(difference_type lhs,
                                   const glob_iterator& rhs) {
      glob_iterator tmp(rhs);
      tmp += lhs;
      return tmp;
    }

    friend glob_iterator operator-(const glob_iterator& lhs,
                                   difference_type rhs) {
      glob_iterator tmp(lhs);
      tmp -= rhs;
      return tmp;
    }

    friend difference_type operator-(const glob_iterator& lhs,
                                     const glob_iterator& rhs) {
      if (rhs == lhs) {
        return 0;
      } else if (rhs.finished()) {
        return -(lhs.M_pathnames.size() - lhs.M_index);
      } else if (lhs.finished()) {
        return rhs.M_pathnames.size() - rhs.M_index;
      } else {
        return lhs.M_index - rhs.M_index;
      }
    }

    friend void swap(glob_iterator& lhs, glob_iterator& rhs) {
      std::swap(lhs.M_pathnames, rhs.M_pathnames);
      std::swap(lhs.M_index, rhs.M_index);
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
