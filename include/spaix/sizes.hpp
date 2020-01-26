/*
  Copyright 2013-2020 David Robillard <d@drobilla.net>

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SPAIX_SIZES_HPP
#define SPAIX_SIZES_HPP

#include "spaix/types.hpp"

#include <cstddef>
#include <limits>

namespace spaix {

/// Return a directory fanout where nodes fit within `page_size` bytes
template <class DirKey>
constexpr ChildCount
internal_fanout(const size_t page_size = 128u)
{
  return static_cast<ChildCount>(
      (page_size - sizeof(NodeType) - sizeof(ChildCount)) /
      (sizeof(DirKey) + sizeof(void*)));
}

/// Return a leaf fanout where nodes nodes fit within `page_size` bytes
template <class DatKey, class Data>
constexpr ChildCount
leaf_fanout(const size_t page_size = 128u)
{
  return static_cast<ChildCount>(
      (page_size - sizeof(NodeType) - sizeof(ChildCount)) /
      (sizeof(DatKey) + sizeof(Data)));
}

/// Return log_2(n)
template <class T>
constexpr T
log_2(const T n)
{
  return (n < 2) ? 1 : 1 + log_2(n / 2);
}

/// Return log_b(n)
template <class T>
constexpr T
log_b(const T n, const T b)
{
  return log_2(n) / log_2(b);
}

/// Return an upper bound on the maximum number of elements in a tree
constexpr size_t
max_size(const size_t dat_size)
{
  return std::numeric_limits<size_t>::max() / dat_size;
}

/// Return the maximum height of a tree
constexpr size_t
max_height(const size_t dat_size, const ChildCount min_fanout)
{
  return log_b(max_size(dat_size), size_t{min_fanout});
}

} // namespace spaix

#endif // SPAIX_SIZES_HPP
