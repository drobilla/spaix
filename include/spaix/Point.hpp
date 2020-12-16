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

#ifndef SPAIX_POINT_HPP
#define SPAIX_POINT_HPP

#include "spaix/detail/meta.hpp"
#include "spaix/types.hpp"

#include <cstddef>
#include <iosfwd>
#include <ostream>
#include <tuple>
#include <utility>

namespace spaix {

/// A multi-dimension point which supports heterogeneous types
template <class... Ts>
class Point
{
public:
  using Tuple = std::tuple<Ts...>;

  explicit constexpr Point(Ts... values) : _values{std::move(values)...} {}
  explicit constexpr Point(Tuple values) : _values{std::move(values)} {}

  constexpr auto         ibegin() const { return spaix::ibegin<Ts...>(); }
  constexpr size_t       size() const { return sizeof...(Ts); }
  constexpr const Tuple& tuple() const { return _values; }

private:
  Tuple _values;
};

template <class... Ts>
constexpr auto
make_point(Ts&&... values)
{
  return Point<Ts...>{std::forward<Ts>(values)...};
}

template <class... Ts>
constexpr bool
operator==(const Point<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return lhs.tuple() == rhs.tuple();
}

template <class... Ts>
constexpr bool
operator!=(const Point<Ts...>& lhs, const Point<Ts...>& rhs)
{
  return !(lhs == rhs);
}

template <size_t dim, class... Ts>
constexpr const Nth<dim, Ts...>&
get(const Point<Ts...>& point)
{
  return std::get<dim>(point.tuple());
}

template <size_t dim, class... Ts>
constexpr Range<Nth<dim, Ts...>>
range(const Point<Ts...>& point)
{
  return std::make_pair(get<dim>(point), get<dim>(point));
}

namespace detail {

template <class... Ts, size_t n_dims>
constexpr auto
point_ranges_rec(const Point<Ts...>&, EndIndex<n_dims>)
{
  return std::make_tuple();
}

template <class... Ts, size_t dim, size_t n_dims>
constexpr auto
point_ranges_rec(Point<Ts...> point, Index<dim, n_dims> index)
{
  return std::tuple_cat(
      std::make_tuple(std::make_pair(get<dim>(point), get<dim>(point))),
      point_ranges_rec(point, ++index));
}

} // namespace detail

template <class... Ts>
constexpr auto
ranges(const Point<Ts...>& point)
{
  return detail::point_ranges_rec(point, ibegin<Ts...>());
}

template <size_t dim, class... Ts>
constexpr Nth<dim, Ts...>
min(const Point<Ts...>& point)
{
  return std::get<dim>(point.tuple());
}

template <size_t dim, class... Ts>
constexpr Nth<dim, Ts...>
max(const Point<Ts...>& point)
{
  return std::get<dim>(point.tuple());
}

template <size_t dim, class... Ts>
constexpr Nth<dim, Ts...>
span(const Point<Ts...>&)
{
  return 0;
}

namespace detail {

template <class... Ts, size_t n_dims>
void
print_rec(std::ostream&, const Point<Ts...>&, EndIndex<n_dims>)
{
}

template <class... Ts, size_t dim, size_t n_dims>
void
print_rec(std::ostream& os, const Point<Ts...>& point, Index<dim, n_dims> index)
{
  os << ((dim > 0) ? ", " : "") << get<dim>(point);
  print_rec(os, point, ++index);
}

} // namespace detail

template <class... Ts>
inline std::ostream&
operator<<(std::ostream& os, const Point<Ts...>& point)
{
  os << '[';
  detail::print_rec(os, point, point.ibegin());
  return os << ']';
}

} // namespace spaix

#endif // SPAIX_POINT_HPP
