// Copyright 2013-2020 David Robillard <d@drobilla.net>
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
constexpr auto
intersection_rec(const Lhs&, const Rhs&, EndIndex<num_dims>)
{
  return std::make_tuple();
}

template<class Lhs, class Rhs, size_t dim, size_t num_dims>
constexpr auto
intersection_rec(const Lhs& lhs, const Rhs& rhs, Index<dim, num_dims> index)
{
  const auto l = range<dim>(lhs);
  const auto r = range<dim>(rhs);

  return std::tuple_cat(
    std::make_tuple(
      std::make_pair(std::max(l.first, r.first), std::min(l.second, r.second))),
    intersection_rec(lhs, rhs, ++index));
}

} // namespace detail

/// Return the geometric intersection of `lhs` and `rhs`
template<class... Ts>
constexpr Rect<Ts...>
operator&(const Point<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric intersection of `lhs` and `rhs`
template<class... Ts>
constexpr Rect<Ts...>
operator&(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric intersection of `lhs` and `rhs`
template<class... Ts>
constexpr Rect<Ts...>
operator&(const Rect<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric intersection of `lhs` and `rhs`
template<class... Ts>
constexpr Rect<Ts...>
operator&(const Point<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

} // namespace spaix

#endif // SPAIX_INTERSECTION_HPP
