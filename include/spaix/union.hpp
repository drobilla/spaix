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

#ifndef SPAIX_UNION_HPP
#define SPAIX_UNION_HPP

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

#include <algorithm>

namespace spaix {
namespace detail {

template <class Lhs, class Rhs, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr auto
union_rec(const Lhs&, const Rhs&, EndIndex<n_dims>)
{
  return std::make_tuple();
}

template <class Lhs, class Rhs, size_t dim, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr auto
union_rec(const Lhs& lhs, const Rhs& rhs, Index<dim, n_dims> index)
{
  const auto l = range<dim>(lhs);
  const auto r = range<dim>(rhs);

  return std::tuple_cat(
      std::make_tuple(std::make_pair(std::min(l.first, r.first),
                                     std::max(l.second, r.second))),
      union_rec(lhs, rhs, ++index));
}

template <class Lhs, class Rhs, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr auto
expand_rec(Lhs&, const Rhs&, EndIndex<n_dims>)
{
}

template <class Lhs, class Rhs, size_t dim, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr auto
expand_rec(Lhs& lhs, const Rhs& rhs, Index<dim, n_dims> index)
{
  auto&      l = range<dim>(lhs);
  const auto r = range<dim>(rhs);

  l.first  = std::min(l.first, r.first);
  l.second = std::max(l.second, r.second);

  expand_rec(lhs, rhs, ++index);
}

} // namespace detail

/// Return the geometric union of `lhs` and `rhs`
template <class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>
operator|(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric union of `lhs` and `rhs`
template <class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>
operator|(const Rect<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Expand `lhs` to include `rhs`
template <class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>&
operator|=(Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  detail::expand_rec(lhs, rhs, ibegin<Ts...>());
  return lhs;
}

/// Expand `lhs` to include `rhs`
template <class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>
operator|=(Rect<Ts...>& lhs, const Point<Ts...>& rhs)
{
  detail::expand_rec(lhs, rhs, ibegin<Ts...>());
  return lhs;
}

/// Return the geometric union of `lhs` and `rhs`
template <class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>
operator|(const Point<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, ibegin<Ts...>())};
}

/// Return the geometric union of `lhs` and `rhs`
template <class... Ts>
SPAIX_ALWAYS_INLINE constexpr Rect<Ts...>
operator|(const Point<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return Rect<Ts...>{detail::union_rec(lhs, rhs, rhs.ibegin())};
}

} // namespace spaix

#endif // SPAIX_UNION_HPP
