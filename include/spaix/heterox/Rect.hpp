// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_HETEROX_RECT_HPP
#define SPAIX_HETEROX_RECT_HPP

#include <spaix/detail/Index.hpp>
#include <spaix/heterox/Point.hpp>
#include <spaix/heterox/detail/meta.hpp>
#include <spaix/types.hpp>

#include <cstddef>
#include <iosfwd>
#include <limits>
#include <tuple>
#include <utility>

namespace spaix::heterox {

namespace detail {

template<class... Ts, size_t n_dims>
constexpr auto empty_ranges_rec(::spaix::detail::EndIndex<n_dims>) noexcept
{
  return std::make_tuple();
}

template<class... Ts, size_t dim, size_t n_dims>
constexpr auto
empty_ranges_rec(::spaix::detail::Index<dim, n_dims> index) noexcept
{
  using T = detail::Nth<dim, Ts...>;

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
  using Tuple = std::tuple<DimRange<T0>, DimRange<Ts>...>;

  /// Construct an empty rectangle
  constexpr explicit Rect() noexcept
    : _ranges{detail::empty_ranges_rec<T0, Ts...>(ibegin())}
  {}

  /// Construct a rectangle from a single point
  constexpr explicit Rect(const Point<T0, Ts...>& point) noexcept
    : _ranges{ranges(point)}
  {}

  /// Construct a rectangle for the given ranges in each dimension
  constexpr explicit Rect(DimRange<T0> first, DimRange<Ts>... rest) noexcept
    : Rect{std::make_tuple(std::move(first), std::move(rest)...)}
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

  static constexpr auto ibegin() { return detail::ibegin<T0, Ts...>(); }

  static constexpr size_t size() { return 1 + sizeof...(Ts); }

  [[nodiscard]] constexpr const Tuple& tuple() const { return _ranges; }
  [[nodiscard]] constexpr Tuple&       tuple() { return _ranges; }

private:
  Tuple _ranges{};
};

template<class... Ts>
constexpr bool
operator==(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs) noexcept
{
  return lhs.tuple() == rhs.tuple();
}

template<class... Ts>
constexpr bool
operator!=(const Rect<Ts...>& lhs, const Rect<Ts...>& rhs) noexcept
{
  return !(lhs == rhs);
}

template<size_t dim, class... Ts>
constexpr const DimRange<detail::Nth<dim, Ts...>>&
get(const Rect<Ts...>& rect) noexcept
{
  return std::get<dim>(rect.tuple());
}

template<size_t dim, class... Ts>
constexpr const DimRange<detail::Nth<dim, Ts...>>&
range(const Rect<Ts...>& rect) noexcept
{
  return std::get<dim>(rect.tuple());
}

template<size_t dim, class... Ts>
DimRange<detail::Nth<dim, Ts...>>&
range(Rect<Ts...>& rect) noexcept
{
  return std::get<dim>(rect.tuple());
}

template<class... Ts>
constexpr const auto&
ranges(const Rect<Ts...>& rect) noexcept
{
  return rect.tuple();
}

namespace detail {

template<class CharT, class Traits, class... Ts, size_t n_dims>
void
print_rec(std::basic_ostream<CharT, Traits>&,
          const Rect<Ts...>&,
          ::spaix::detail::EndIndex<n_dims>) noexcept
{}

template<class CharT, class Traits, class... Ts, size_t dim, size_t n_dims>
void
print_rec(std::basic_ostream<CharT, Traits>&  os,
          const Rect<Ts...>&                  rect,
          ::spaix::detail::Index<dim, n_dims> index)
{
  const auto [lower, upper] = range<dim>(rect);

  os << ((dim > 0) ? ", " : "") << "[" << lower << ", " << upper << ']';

  print_rec(os, rect, ++index);
}

} // namespace detail

template<class CharT, class Traits, class... Ts>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const Rect<Ts...>& rect)
{
  os << '[';
  detail::print_rec(os, rect, rect.ibegin());
  return os << ']';
}

} // namespace spaix::heterox

#endif // SPAIX_HETEROX_RECT_HPP
