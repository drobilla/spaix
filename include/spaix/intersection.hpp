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

#ifndef SPAIX_INTERSECTION_HPP
#define SPAIX_INTERSECTION_HPP

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

#include <algorithm>

namespace spaix {
namespace detail {

template <class Lhs, class Rhs, size_t num_dims>
constexpr auto
intersection_rec(const Lhs&, const Rhs&, EndIndex<num_dims>)
{
  return std::make_tuple();
}

template <class Lhs, class Rhs, size_t dim, size_t num_dims>
constexpr auto
intersection_rec(const Lhs& lhs, const Rhs& rhs, Index<dim, num_dims> index)
{
  const auto l = range<dim>(lhs);
  const auto r = range<dim>(rhs);

  return std::tuple_cat(
      std::make_tuple(std::make_pair(std::max(l.first, r.first),
                                     std::min(l.second, r.second))),
      intersection_rec(lhs, rhs, ++index));
}

} // namespace detail

/// Return the geometric intersection of `lhs` and `rhs`
template <class... Ts>
constexpr Rect<Ts...>
operator&(const Point<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric intersection of `lhs` and `rhs`
template <class... Ts>
constexpr Rect<Ts...>
operator&(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric intersection of `lhs` and `rhs`
template <class... Ts>
constexpr Rect<Ts...>
operator&(const Rect<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric intersection of `lhs` and `rhs`
template <class... Ts>
constexpr Rect<Ts...>
operator&(const Point<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::intersection_rec(lhs, rhs, ibegin<Ts...>())};
}

} // namespace spaix

#endif // SPAIX_INTERSECTION_HPP
