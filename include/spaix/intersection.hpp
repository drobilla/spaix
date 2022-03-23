// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_INTERSECTION_HPP
#define SPAIX_INTERSECTION_HPP

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <utility>

namespace spaix {
namespace detail {

template<class Lhs, class Rhs, size_t num_dims>
SPAIX_ALWAYS_INLINE constexpr auto
intersection_rec(const Lhs&, const Rhs&, EndIndex<num_dims>)
{
  return std::make_tuple();
}

template<class Lhs, class Rhs, size_t dim, size_t num_dims>
SPAIX_ALWAYS_INLINE constexpr auto
intersection_rec(const Lhs& lhs, const Rhs& rhs, Index<dim, num_dims> index)
{
  using std::max;
  using std::min;

  const auto l  = range<dim>(lhs);
  const auto r  = range<dim>(rhs);
  const auto lo = max(l.first, r.first);
  const auto hi = min(l.second, r.second);

  return std::tuple_cat(std::make_tuple(std::make_pair(lo, hi)),
                        intersection_rec(lhs, rhs, ++index));
}

} // namespace detail

/// Return the geometric intersection of two points
template<class... Ts>
constexpr Rect<Ts...>
operator&(const Point<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return lhs == rhs ? Rect<Ts...>{lhs} : Rect<Ts...>{};
}

/// Return the geometric intersection of two rectangles
template<class... Ts>
constexpr Rect<Ts...>
operator&(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric intersection of a point and a rectangle
template<class... Ts>
constexpr Rect<Ts...>
operator&(const Point<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric intersection of a rectangle and a point
template<class... Ts>
constexpr Rect<Ts...>
operator&(const Rect<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

} // namespace spaix

#endif // SPAIX_INTERSECTION_HPP
