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
page_dir_fanout(const size_t page_size)
{
  return (page_size - sizeof(NodeType) - sizeof(ChildCount)) /
         (sizeof(DirKey) + sizeof(void*));
}

/// Return a leaf fanout where nodes nodes fit within `page_size` bytes
template<class DatKey, class Data, DataPlacement placement>
constexpr ChildCount
page_dat_fanout(const size_t page_size)
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

/// Return b^e (b raised to the power e)
template<class T>
constexpr T
power(const T b, const T e)
{
  return !e ? 1 : b * power(b, e - 1);
}

/**
   Return the maximum possible height of a tree on this system.

   For typical configurations, this is something like 7 on 32-bit and 20 on
   64-bit, storing hundreds of millions or hundreds of quadrillions of
   elements, respectively.
*/
template<class DirectoryNodeType, class DataNodeType, DataPlacement placement>
constexpr size_t
max_height(const ChildCount min_fanout)
{
  constexpr auto total_space = std::numeric_limits<size_t>::max();
  constexpr auto dir_size    = sizeof(DirectoryNodeType);
  constexpr auto dat_size    = sizeof(DataNodeType);

  switch (placement) {
  case DataPlacement::inlined: {
    const auto n_most_dirs = total_space / dir_size;

    return log_b(n_most_dirs, size_t{min_fanout});
  }

  case DataPlacement::separate: {
    const auto n_most_dats         = total_space / dat_size;
    const auto n_most_dirs         = log_b(n_most_dats, size_t{min_fanout});
    const auto needed_dir_space    = n_most_dirs * dir_size;
    const auto available_dat_space = total_space - needed_dir_space;
    const auto max_n_dats          = available_dat_space / dat_size;

    return log_b(max_n_dats, size_t{min_fanout});
  }
  }
}

} // namespace spaix

#endif // SPAIX_SIZES_HPP
