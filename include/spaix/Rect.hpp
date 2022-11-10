// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_RECT_HPP
#define SPAIX_RECT_HPP

#include "spaix/Point.hpp"
#include "spaix/detail/meta.hpp"
#include "spaix/types.hpp"

#include <cstddef>
#include <limits>
#include <ostream>
#include <tuple>
#include <utility>

namespace spaix {

namespace detail {

template<class... Ts, size_t n_dims>
constexpr auto
empty_ranges_rec(EndIndex<n_dims>) noexcept
{
  return std::make_tuple();
}

template<class... Ts, size_t dim, size_t n_dims>
constexpr auto
empty_ranges_rec(Index<dim, n_dims> index) noexcept
{
  using T = Nth<dim, Ts...>;

  static_assert(std::numeric_limits<T>::lowest() <
                std::numeric_limits<T>::max());

  return std::tuple_cat(
    std::make_tuple(DimRange<T>{std::numeric_limits<T>::max(),
                                std::numeric_limits<T>::lowest()}),
    empty_ranges_rec<Ts...>(++index));
}

} // namespace detail

/// A multi-dimensional rectangle which suports heterogeneous types
template<class T0, class... Ts>
class Rect
{
public:
  using Tuple   = std::tuple<DimRange<T0>, DimRange<Ts>...>;
  using Scalars = std::tuple<T0, Ts...>;

  /// Construct an empty rectangle
  constexpr explicit Rect() noexcept
    : _ranges{detail::empty_ranges_rec<T0, Ts...>(ibegin())}
  {}

  /// Construct a rectangle from a single point
  constexpr explicit Rect(const Point<T0, Ts...>& point) noexcept
    : _ranges{ranges(point)}
  {}

  /// Construct a rectangle for the given ranges in each dimension
  constexpr explicit Rect(DimRange<T0>&& first, DimRange<Ts>&&... rest) noexcept
    : Rect{std::make_tuple(std::forward<DimRange<T0>>(first),
                           std::forward<DimRange<Ts>>(rest)...)}
  {}

  /// Construct a rectangle for the given ranges in each dimension
  constexpr explicit Rect(Tuple ranges) noexcept
    : _ranges{std::move(ranges)}
  {}

  Rect(const Rect& rect) noexcept            = default;
  Rect& operator=(const Rect& rect) noexcept = default;

  Rect(Rect&& rect) noexcept            = default;
  Rect& operator=(Rect&& rect) noexcept = default;

  ~Rect() noexcept = default;

  static constexpr auto   ibegin() { return spaix::ibegin<T0, Ts...>(); }
  static constexpr size_t size() { return 1 + sizeof...(Ts); }

  constexpr const Tuple& tuple() const { return _ranges; }
  constexpr Tuple&       tuple() { return _ranges; }

private:
  Tuple _ranges{};
};

template<class... Ts>
constexpr Rect<Ts...>
make_rect(DimRange<Ts>&&... ranges)
{
  return Rect<Ts...>{std::forward<DimRange<Ts>>(ranges)...};
}

template<class... Ts>
constexpr bool
operator==(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return lhs.tuple() == rhs.tuple();
}

template<class... Ts>
constexpr bool
operator!=(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs)
{
  return !(lhs == rhs);
}

template<size_t dim, class... Ts>
constexpr const DimRange<Nth<dim, Ts...>>&
get(const Rect<Ts...>& rect)
{
  return std::get<dim>(rect.tuple());
}

template<size_t dim, class... Ts>
constexpr const DimRange<Nth<dim, Ts...>>&
range(const Rect<Ts...>& rect)
{
  return std::get<dim>(rect.tuple());
}

template<size_t dim, class... Ts>
DimRange<Nth<dim, Ts...>>&
range(Rect<Ts...>& rect)
{
  return std::get<dim>(rect.tuple());
}

template<class... Ts>
constexpr const auto&
ranges(const Rect<Ts...>& rect)
{
  return rect.tuple();
}

template<size_t dim, class... Ts>
constexpr DifferenceOf<Nth<dim, Ts...>, Nth<dim, Ts...>>
span(const Rect<Ts...>& rect)
{
  const auto& dim_range = get<dim>(rect);
  return (dim_range.upper < dim_range.lower)
           ? 0
           : dim_range.upper - dim_range.lower;
}

namespace detail {

template<class... Ts, size_t n_dims>
void
print_rec(std::ostream&, const Rect<Ts...>&, EndIndex<n_dims>)
{}

template<class... Ts, size_t dim, size_t n_dims>
void
print_rec(std::ostream& os, const Rect<Ts...>& rect, Index<dim, n_dims> index)
{
  const auto [lower, upper] = range<dim>(rect);

  os << ((dim > 0) ? ", " : "") << "[" << lower << ", " << upper << ']';

  print_rec(os, rect, ++index);
}

} // namespace detail

template<class... Ts>
inline std::ostream&
operator<<(std::ostream& os, const Rect<Ts...>& rect)
{
  os << '[';
  detail::print_rec(os, rect, rect.ibegin());
  return os << ']';
}

} // namespace spaix

#endif // SPAIX_RECT_HPP
