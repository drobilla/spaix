// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_SIZES_HPP
#define SPAIX_SIZES_HPP

#include "spaix/DataPlacement.hpp"
#include "spaix/types.hpp"

#include <cstddef>
#include <limits>

namespace spaix {

/// Return a directory fanout where nodes fit within `page_size` bytes
template<class DirKey>
constexpr ChildCount
page_internal_fanout(const size_t page_size)
{
  return (page_size - sizeof(NodeType) - sizeof(ChildCount)) /
         (sizeof(DirKey) + sizeof(void*));
}

/// Return a leaf fanout where nodes nodes fit within `page_size` bytes
template<class DatKey, class Data, DataPlacement placement>
constexpr ChildCount
page_leaf_fanout(const size_t page_size)
{
  switch (placement) {
  case DataPlacement::inlined:
    return (page_size - sizeof(NodeType) - sizeof(ChildCount)) /
           (sizeof(DatKey) + sizeof(Data));
  case DataPlacement::separate:
    return (page_size - sizeof(NodeType) - sizeof(ChildCount)) /
           (sizeof(void*));
  }
}

/// Return log_2(n)
template<class T>
constexpr T
log_2(const T n)
{
  return (n < 2) ? 1 : 1 + log_2(n / 2);
}

/// Return log_b(n)
template<class T>
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
