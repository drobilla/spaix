// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_HETEROX_POINT_HPP
#define SPAIX_HETEROX_POINT_HPP

#include <spaix/detail/Index.hpp>
#include <spaix/heterox/detail/meta.hpp>
#include <spaix/types.hpp>

#include <cstddef>
#include <iosfwd>
#include <ostream>
#include <tuple>
#include <utility>

namespace spaix::heterox {

/// A multi-dimension point which supports heterogeneous types
template<class... Ts>
class Point
{
public:
  using Tuple = std::tuple<Ts...>;

  constexpr explicit Point(Ts... values) noexcept
    : _values{std::move(values)...}
  {}

  constexpr explicit Point(Tuple values) noexcept
    : _values{std::move(values)}
  {}

  constexpr Point(const Point&) noexcept            = default;
  constexpr Point& operator=(const Point&) noexcept = default;

  constexpr Point(Point&&) noexcept            = default;
  constexpr Point& operator=(Point&&) noexcept = default;

  ~Point() noexcept = default;

  [[nodiscard]] constexpr auto ibegin() const
  {
    return detail::ibegin<Ts...>();
  }

  [[nodiscard]] constexpr size_t       size() const { return sizeof...(Ts); }
  [[nodiscard]] constexpr const Tuple& tuple() const { return _values; }

private:
  Tuple _values;
};

template<class... Ts>
constexpr bool
operator==(const Point<Ts...>& lhs, const Point<Ts...>& rhs) noexcept
{
  return lhs.tuple() == rhs.tuple();
}

template<class... Ts>
constexpr bool
operator!=(const Point<Ts...>& lhs, const Point<Ts...>& rhs) noexcept
{
  return !(lhs == rhs);
}

template<size_t dim, class... Ts>
constexpr const detail::Nth<dim, Ts...>&
get(const Point<Ts...>& point) noexcept
{
  return std::get<dim>(point.tuple());
}

template<size_t dim, class... Ts>
constexpr DimRange<detail::Nth<dim, Ts...>>
range(const Point<Ts...>& point) noexcept
{
  return {get<dim>(point), get<dim>(point)};
}

namespace detail {

template<class... Ts, size_t n_dims>
constexpr auto
point_ranges_rec(const Point<Ts...>&,
                 ::spaix::detail::EndIndex<n_dims>) noexcept
{
  return std::make_tuple();
}

template<class... Ts, size_t dim, size_t n_dims>
constexpr auto
point_ranges_rec(Point<Ts...>                        point,
                 ::spaix::detail::Index<dim, n_dims> index) noexcept
{
  return std::tuple_cat(
    std::make_tuple(make_dim_range(get<dim>(point), get<dim>(point))),
    point_ranges_rec(point, ++index));
}

} // namespace detail

template<class... Ts>
constexpr auto
ranges(const Point<Ts...>& point) noexcept
{
  return detail::point_ranges_rec(point, detail::ibegin<Ts...>());
}

template<size_t dim, class... Ts>
constexpr detail::Nth<dim, Ts...>
span(const Point<Ts...>&) noexcept
{
  return {};
}

namespace detail {

template<class... Ts, size_t n_dims>
void
print_rec(std::ostream&,
          const Point<Ts...>&,
          ::spaix::detail::EndIndex<n_dims>) noexcept
{}

template<class... Ts, size_t dim, size_t n_dims>
void
print_rec(std::ostream&                       os,
          const Point<Ts...>&                 point,
          ::spaix::detail::Index<dim, n_dims> index)
{
  os << ((dim > 0) ? ", " : "") << get<dim>(point);
  print_rec(os, point, ++index);
}

} // namespace detail

template<class... Ts>
inline std::ostream&
operator<<(std::ostream& os, const Point<Ts...>& point)
{
  os << '[';
  detail::print_rec(os, point, point.ibegin());
  return os << ']';
}

} // namespace spaix::heterox

#endif // SPAIX_HETEROX_POINT_HPP
