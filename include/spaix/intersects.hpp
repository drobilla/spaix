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

#ifndef SPAIX_INTERSECTS_HPP
#define SPAIX_INTERSECTS_HPP

#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

namespace spaix {
namespace detail {

template <class Lhs, class Rhs, size_t num_dims>
constexpr bool
intersects_rec(const Lhs&, const Rhs&, EndIndex<num_dims>)
{
  return true;
}

template <class Lhs, class Rhs, size_t dim, size_t num_dims>
constexpr bool
intersects_rec(const Lhs& lhs, const Rhs& rhs, Index<dim, num_dims> index)
{
  return ((max<dim>(rhs) < min<dim>(lhs) || max<dim>(lhs) < min<dim>(rhs))
              ? false
              : intersects_rec(lhs, rhs, ++index));
}

} // namespace detail

/// Return true iff `lhs` has a non-empty intersection with `rhs`
template <class... Ts>
constexpr bool
intersects(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return detail::intersects_rec(lhs, rhs, ibegin<Ts...>());
}

/// Return true iff `lhs` has a non-empty intersection with `rhs`
template <class... Ts>
constexpr bool
intersects(const Rect<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return detail::intersects_rec(lhs, rhs, ibegin<Ts...>());
}

/// Return true iff `lhs` has a non-empty intersection with `rhs`
template <class... Ts>
constexpr bool
intersects(const Point<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return detail::intersects_rec(rhs, lhs, ibegin<Ts...>());
}

} // namespace spaix

#endif // SPAIX_INTERSECTS_HPP
