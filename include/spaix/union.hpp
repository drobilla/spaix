// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_UNION_HPP
#define SPAIX_UNION_HPP

#include <spaix/Point.hpp>
#include <spaix/Rect.hpp>
#include <spaix/detail/meta.hpp>
#include <spaix/types.hpp>

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <utility>

namespace spaix {
namespace detail {

using std::max;
using std::min;

template<class Lhs, class Rhs, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr auto
union_rec(const Lhs&, const Rhs&, EndIndex<n_dims>) noexcept
{
  return std::make_tuple();
}

template<class Lhs, class Rhs, size_t dim, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr auto
union_rec(const Lhs& lhs, const Rhs& rhs, Index<dim, n_dims> index) noexcept
{
  const auto l  = range<dim>(lhs);
  const auto r  = range<dim>(rhs);
  const auto lo = min(l.lower, r.lower);
  const auto hi = max(l.upper, r.upper);

  return std::tuple_cat(std::make_tuple(make_dim_range(lo, hi)),
                        union_rec(lhs, rhs, ++index));
}

template<class Lhs, class Rhs, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr void
union_rec(Lhs&, const Rhs&, EndIndex<n_dims>) noexcept
{}

template<class Lhs, class Rhs, size_t dim, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr void
union_rec(Lhs& lhs, const Rhs& rhs, Index<dim, n_dims> index) noexcept
{
  auto&      l = range<dim>(lhs);
  const auto r = range<dim>(rhs);

  l.lower = min(l.lower, r.lower);
  l.upper = max(l.upper, r.upper);

  union_rec(lhs, rhs, ++index);
}

} // namespace detail

/// Return the geometric union of two points
template<class... Ts>
constexpr Rect<Ts...>
operator|(const Point<Ts...>& lhs, const Point<Ts...>& rhs) noexcept
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, rhs.ibegin())};
}

/// Return the geometric union of two rectangles
template<class... Ts>
constexpr Rect<Ts...>
operator|(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs) noexcept
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric union of a point and a rectangle
template<class... Ts>
constexpr Rect<Ts...>
operator|(const Point<Ts...>& lhs, const Rect<Ts...>& rhs) noexcept
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric union of a rectangle and a point
template<class... Ts>
constexpr Rect<Ts...>
operator|(const Rect<Ts...>& lhs, const Point<Ts...>& rhs) noexcept
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Expand `lhs` to include `rhs`
template<class... Ts>
constexpr Rect<Ts...>&
operator|=(Rect<Ts...>& lhs, const Rect<Ts...>& rhs) noexcept
{
  detail::union_rec(lhs, rhs, ibegin<Ts...>());
  return lhs;
}

/// Expand `lhs` to include `rhs`
template<class... Ts>
constexpr Rect<Ts...>
operator|=(Rect<Ts...>& lhs, const Point<Ts...>& rhs) noexcept
{
  detail::union_rec(lhs, rhs, ibegin<Ts...>());
  return lhs;
}

template<class K>
using UnionOf = decltype(std::declval<K>() | std::declval<K>());

} // namespace spaix

#endif // SPAIX_UNION_HPP
