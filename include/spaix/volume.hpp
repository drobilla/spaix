/*
  Copyright 2013-2020 David Robillard <d@drobilla.net>

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SPAIX_VOLUME_HPP
#define SPAIX_VOLUME_HPP

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

namespace spaix {
namespace detail {

template <class... Ts, size_t last_dim>
constexpr auto
volume_rec(const Rect<Ts...>& rect, LastIndex<last_dim>)
{
  return span<last_dim>(rect);
}

template <class... Ts, size_t dim, size_t last_dim>
constexpr ProductOf<Ts...>
volume_rec(const Rect<Ts...>& rect, InclusiveIndex<dim, last_dim> index)
{
  const auto r = range<dim>(rect);

  return ((r.first < r.second)
              ? ((r.second - r.first) * volume_rec(rect, ++index))
              : 0);
}

} // namespace detail

template <class... Ts>
constexpr ProductOf<Ts...>
volume(const Rect<Ts...>& rect)
{
  return detail::volume_rec(rect, ibegin_inclusive<Ts...>());
}

template <class... Ts>
constexpr ProductOf<Ts...>
volume(const Point<Ts...>&)
{
  return 0;
}

} // namespace spaix

#endif // SPAIX_VOLUME_HPP
