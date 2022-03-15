// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_UNION_HPP
#define SPAIX_UNION_HPP

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

#include <algorithm>
#include <cstddef>
#include <tuple>
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
  const auto l = range<dim>(lhs);
  const auto r = range<dim>(rhs);

  return std::tuple_cat(
    std::make_tuple(
      std::make_pair(std::min(l.first, r.first), std::max(l.second, r.second))),
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

/// Return the geometric union of `lhs` and `rhs`
template<class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>
operator|(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric union of `lhs` and `rhs`
template<class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>
operator|(const Rect<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Expand `lhs` to include `rhs`
template<class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>&
operator|=(Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  detail::union_rec(lhs, rhs, ibegin<Ts...>());
  return lhs;
}

/// Expand `lhs` to include `rhs`
template<class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>
operator|=(Rect<Ts...>& lhs, const Point<Ts...>& rhs)
{
  detail::union_rec(lhs, rhs, ibegin<Ts...>());
  return lhs;
}

/// Return the geometric union of `lhs` and `rhs`
template<class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>
operator|(const Point<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric union of `lhs` and `rhs`
template<class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>
operator|(const Point<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, rhs.ibegin())};
}

} // namespace spaix

#endif // SPAIX_UNION_HPP
