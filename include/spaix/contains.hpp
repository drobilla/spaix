// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_CONTAINS_HPP
#define SPAIX_CONTAINS_HPP

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

#include <cstddef>

namespace spaix {
namespace detail {

template<class Parent, class Child, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr bool
contains_rec(const Parent&, const Child&, EndIndex<n_dims>) noexcept
{
  return true;
}

template<class Parent, class Child, size_t dim, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr bool
contains_rec(const Parent&      parent,
             const Child&       child,
             Index<dim, n_dims> index) noexcept
{
  const auto& p = range<dim>(parent);
  const auto& c = range<dim>(child);

  return (!(c.lower < p.lower) && !(p.upper < c.upper) &&
          contains_rec(parent, child, ++index));
}

} // namespace detail

/// Return true iff `parent` contains `child`
template<class... Ts>
constexpr bool
contains(const Point<Ts...>& parent, const Point<Ts...>& child) noexcept
{
  return parent == child;
}

/// Return true iff `parent` contains `child`
template<class... Ts>
constexpr bool
contains(const Rect<Ts...>& parent, const Rect<Ts...>& child) noexcept
{
  return detail::contains_rec(parent, child, ibegin<Ts...>());
}

/// Return true iff `parent` contains `child`
template<class... Ts>
constexpr bool
contains(const Point<Ts...>& parent, const Rect<Ts...>& child) noexcept
{
  return detail::contains_rec(parent, child, ibegin<Ts...>());
}

/// Return true iff `parent` contains `child`
template<class... Ts>
constexpr bool
contains(const Rect<Ts...>& parent, const Point<Ts...>& child) noexcept
{
  return detail::contains_rec(parent, child, ibegin<Ts...>());
}

} // namespace spaix

#endif // SPAIX_CONTAINS_HPP
