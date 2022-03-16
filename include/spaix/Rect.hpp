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
empty_ranges_rec(EndIndex<n_dims>)
{
  return std::make_tuple();
}

template<class... Ts, size_t dim, size_t n_dims>
constexpr auto
empty_ranges_rec(Index<dim, n_dims> index)
{
  using T = Nth<dim, Ts...>;

  static_assert(std::numeric_limits<T>::lowest() <
                std::numeric_limits<T>::max());

  return std::tuple_cat(
    std::make_tuple(std::make_pair(std::numeric_limits<T>::max(),
                                   std::numeric_limits<T>::lowest())),
    empty_ranges_rec<Ts...>(++index));
}

template<class Tuple, size_t n_dims>
constexpr bool
ranges_are_empty(const Tuple&, EndIndex<n_dims>)
{
  return false;
}

template<class Tuple, size_t dim, size_t n_dims>
constexpr bool
ranges_are_empty(const Tuple& tuple, Index<dim, n_dims> index)
{
  return (std::get<dim>(tuple).second < std::get<dim>(tuple).first) ||
         ranges_are_empty(tuple, ++index);
}

} // namespace detail

/// A multi-dimensional rectangle which suports heterogeneous types
template<class T0, class... Ts>
class Rect
{
public:
  using Tuple   = std::tuple<DimRange<T0>, DimRange<Ts>...>;
  using Scalars = std::tuple<T0, Ts...>;

  Rect(const Rect& rect)     = default;
  Rect(Rect&& rect) noexcept = default;
  Rect& operator=(const Rect& rect) = default;
  Rect& operator=(Rect&& rect) noexcept = default;

  ~Rect() = default;

  /// Construct an empty rectangle
  constexpr explicit Rect()
    : _ranges{detail::empty_ranges_rec<T0, Ts...>(ibegin())}
  {}

  /// Construct a rectangle from a single point
  constexpr explicit Rect(const Point<T0, Ts...>& point)
    : _ranges{ranges(point)}
  {}

  /// Construct a rectangle for the given ranges in each dimension
  constexpr explicit Rect(DimRange<T0>&& first, DimRange<Ts>&&... rest)
    : Rect{std::make_tuple(std::forward<DimRange<T0>>(first),
                           std::forward<DimRange<Ts>>(rest)...)}
  {}

  /// Construct a rectangle for the given ranges in each dimension
#if 1
  constexpr explicit Rect(Tuple ranges)
    : _ranges{detail::ranges_are_empty(ranges, ibegin())
                ? detail::empty_ranges_rec<T0, Ts...>(ibegin())
                : std::move(ranges)}
  {}
#else
  constexpr explicit Rect(Tuple ranges)
    : _ranges{std::move(ranges)}
  {}
#endif

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
constexpr const Nth<dim, Ts...>&
min(const Rect<Ts...>& rect)
{
  return std::get<dim>(rect.tuple()).first;
}

template<size_t dim, class... Ts>
constexpr const Nth<dim, Ts...>&
max(const Rect<Ts...>& rect)
{
  return std::get<dim>(rect.tuple()).second;
}

template<size_t dim, class... Ts>
constexpr Nth<dim, Ts...>
span(const Rect<Ts...>& rect)
{
  const auto& dim_range = get<dim>(rect);
  return (dim_range.second < dim_range.first)
           ? 0
           : dim_range.second - dim_range.first;
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
  os << ((dim > 0) ? ", " : "") << "[" << min<dim>(rect) << ", "
     << max<dim>(rect) << ']';

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
