// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_EMPTY_HPP
#define SPAIX_EMPTY_HPP

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

#include <cstddef>

namespace spaix {
namespace detail {

template<class... Ts, size_t last_dim>
constexpr auto
empty_rec(const Rect<Ts...>& rect, LastIndex<last_dim>)
{
  const auto r = range<last_dim>(rect);

  return !(r.lower < r.upper);
}

template<class... Ts, size_t dim, size_t last_dim>
constexpr auto
empty_rec(const Rect<Ts...>& rect, InclusiveIndex<dim, last_dim> index)
{
  const auto r = range<dim>(rect);

  return !(r.lower < r.upper) || empty_rec(rect, ++index);
}

} // namespace detail

template<class... Ts>
constexpr bool
empty(const Rect<Ts...>& rect)
{
  return detail::empty_rec(rect, ibegin_inclusive<Ts...>());
}

template<class... Ts>
constexpr bool
empty(const Point<Ts...>&)
{
  return true;
}

} // namespace spaix

#endif // SPAIX_EMPTY_HPP
