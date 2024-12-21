// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_INTERSECTS_HPP
#define SPAIX_INTERSECTS_HPP

#include <spaix/Point.hpp>
#include <spaix/Rect.hpp>
#include <spaix/detail/meta.hpp>

#include <cstddef>

namespace spaix {
namespace detail {

template<class Lhs, class Rhs, size_t num_dims>
SPAIX_ALWAYS_INLINE constexpr bool
intersects_rec(const Lhs&, const Rhs&, EndIndex<num_dims>) noexcept
{
  return true;
}

template<class Lhs, class Rhs, size_t dim, size_t num_dims>
SPAIX_ALWAYS_INLINE constexpr bool
intersects_rec(const Lhs&           lhs,
               const Rhs&           rhs,
               Index<dim, num_dims> index) noexcept
{
  const auto& l = range<dim>(lhs);
  const auto& r = range<dim>(rhs);

  return (!(r.upper < l.lower) && !(l.upper < r.lower) &&
          intersects_rec(lhs, rhs, ++index));
}

} // namespace detail

/// Return true iff `lhs` has a non-empty intersection with `rhs`
template<class... Ts>
constexpr bool
intersects(const Point<Ts...>& lhs, const Point<Ts...>& rhs) noexcept
{
  return lhs == rhs;
}

/// Return true iff `lhs` has a non-empty intersection with `rhs`
template<class... Ts>
constexpr bool
intersects(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs) noexcept
{
  return detail::intersects_rec(lhs, rhs, ibegin<Ts...>());
}

/// Return true iff `lhs` has a non-empty intersection with `rhs`
template<class... Ts>
constexpr bool
intersects(const Point<Ts...>& lhs, const Rect<Ts...>& rhs) noexcept
{
  return detail::intersects_rec(rhs, lhs, ibegin<Ts...>());
}

/// Return true iff `lhs` has a non-empty intersection with `rhs`
template<class... Ts>
constexpr bool
intersects(const Rect<Ts...>& lhs, const Point<Ts...>& rhs) noexcept
{
  return detail::intersects_rec(lhs, rhs, ibegin<Ts...>());
}

} // namespace spaix

#endif // SPAIX_INTERSECTS_HPP
