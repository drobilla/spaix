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

#ifndef SPAIX_CONTAINS_HPP
#define SPAIX_CONTAINS_HPP

#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

namespace spaix {
namespace detail {

template <class Parent, class Child, size_t n_dims>
constexpr bool
contains_rec(const Parent&, const Child&, EndIndex<n_dims>)
{
  return true;
}

template <class Parent, class Child, size_t dim, size_t n_dims>
constexpr bool
contains_rec(const Parent& parent, const Child& child, Index<dim, n_dims> index)
{
  return (!(min<dim>(child) < min<dim>(parent)) &&
          !(max<dim>(parent) < max<dim>(child)) &&
          contains_rec(parent, child, ++index));
}

} // namespace detail

/// Return true iff `parent` contains `child`
template <class... Ts>
constexpr bool
contains(const Rect<Ts...>& parent, const Rect<Ts...>& child)
{
  return detail::contains_rec(parent, child, ibegin<Ts...>());
}

/// Return true iff `parent` contains `child`
template <class... Ts>
constexpr bool
contains(const Rect<Ts...>& parent, const Point<Ts...>& child)
{
  return detail::contains_rec(parent, child, ibegin<Ts...>());
}

/// Return true iff `parent` contains `child`
template <class... Ts>
constexpr bool
contains(const Point<Ts...>& parent, const Rect<Ts...>& child)
{
  return detail::contains_rec(parent, child, ibegin<Ts...>());
}

/// Return true iff `parent` contains `child`
template <class... Ts>
constexpr bool
contains(const Point<Ts...>& parent, const Point<Ts...>& child)
{
  return parent == child;
}

} // namespace spaix

#endif // SPAIX_CONTAINS_HPP
