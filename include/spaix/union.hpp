// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_UNION_HPP
#define SPAIX_UNION_HPP

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

#include <algorithm> // IWYU pragma: keep
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace spaix {
namespace detail {

template<class Lhs, class Rhs, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr auto
union_rec(const Lhs&, const Rhs&, EndIndex<n_dims>)
{
  return std::make_tuple();
}

template<class Lhs, class Rhs, size_t dim, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr auto
union_rec(const Lhs& lhs, const Rhs& rhs, Index<dim, n_dims> index)
{
  const auto l  = range<dim>(lhs);
  const auto r  = range<dim>(rhs);
  const auto lo = std::min(l.first, r.first);
  const auto hi = std::max(l.second, r.second);

  return std::tuple_cat(std::make_tuple(std::make_pair(lo, hi)),
                        union_rec(lhs, rhs, ++index));
}

template<class Lhs, class Rhs, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr void
union_rec(Lhs&, const Rhs&, EndIndex<n_dims>)
{}

template<class Lhs, class Rhs, size_t dim, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr void
union_rec(Lhs& lhs, const Rhs& rhs, Index<dim, n_dims> index)
{
  auto&      l = range<dim>(lhs);
  const auto r = range<dim>(rhs);

  l.first  = std::min(l.first, r.first);
  l.second = std::max(l.second, r.second);

  union_rec(lhs, rhs, ++index);
}

} // namespace detail

/// Return the geometric union of two points
template<class... Ts>
constexpr Rect<Ts...>
operator|(const Point<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, rhs.ibegin())};
}

/// Return the geometric union of two rectangles
template<class... Ts>
constexpr Rect<Ts...>
operator|(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric union of a point and a rectangle
template<class... Ts>
constexpr Rect<Ts...>
operator|(const Point<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric union of a rectangle and a point
template<class... Ts>
constexpr Rect<Ts...>
operator|(const Rect<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Expand `lhs` to include `rhs`
template<class... Ts>
constexpr Rect<Ts...>&
operator|=(Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  detail::union_rec(lhs, rhs, ibegin<Ts...>());
  return lhs;
}

/// Expand `lhs` to include `rhs`
template<class... Ts>
constexpr Rect<Ts...>
operator|=(Rect<Ts...>& lhs, const Point<Ts...>& rhs)
{
  detail::union_rec(lhs, rhs, ibegin<Ts...>());
  return lhs;
}

template<class K>
using UnionOf = decltype(std::declval<K>() | std::declval<K>());

} // namespace spaix

#endif // SPAIX_UNION_HPP
