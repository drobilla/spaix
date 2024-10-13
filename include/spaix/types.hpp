// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TYPES_HPP
#define SPAIX_TYPES_HPP

#include <cstddef>

namespace spaix {

using ChildIndex = size_t;
using ChildCount = size_t;

enum class NodeType : unsigned char {
  directory, ///< Internal directory node
  data,      ///< Leaf data node
};

enum class Side : unsigned char { left, right };

/// A range in a dimension from a low value (first) to a high value (second)
template<class T>
struct DimRange {
  T lower; ///< Lowest coordinate value in dimension
  T upper; ///< Highest coordinate value in dimension
};

template<class T>
constexpr DimRange<T>
make_dim_range(T lower, T upper) noexcept
{
  return DimRange<T>{lower, upper};
}

template<class T>
constexpr bool
operator==(const DimRange<T>& lhs, const DimRange<T>& rhs) noexcept
{
  return lhs.lower == rhs.lower && lhs.upper == rhs.upper;
}

} // namespace spaix

#endif // SPAIX_TYPES_HPP
