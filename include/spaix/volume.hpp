// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_VOLUME_HPP
#define SPAIX_VOLUME_HPP

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

#include <cstddef>

namespace spaix {
namespace detail {

template<class... Ts, size_t last_dim>
constexpr auto
volume_rec(const Rect<Ts...>& rect, LastIndex<last_dim>) noexcept
{
  return span<last_dim>(rect);
}

template<class... Ts, size_t dim, size_t last_dim>
constexpr auto
volume_rec(const Rect<Ts...>&            rect,
           InclusiveIndex<dim, last_dim> index) noexcept
{
  const auto r = range<dim>(rect);

  return ((r.lower < r.upper)
            ? ((r.upper - r.lower) * volume_rec(rect, ++index))
            : 0);
}

} // namespace detail

template<class... Ts>
constexpr auto
volume(const Rect<Ts...>& rect) noexcept
{
  return detail::volume_rec(rect, ibegin_inclusive<Ts...>());
}

template<class... Ts>
constexpr ProductOf<Ts...>
volume(const Point<Ts...>&) noexcept
{
  return 0;
}

} // namespace spaix

#endif // SPAIX_VOLUME_HPP
