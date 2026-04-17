// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_HOMOX_RECT_HPP
#define SPAIX_HOMOX_RECT_HPP

#include <spaix/homox/Point.hpp>
#include <spaix/types.hpp>

#include <array>
#include <cstddef>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <utility>

namespace spaix::homox {

/// A multi-dimensional rectangle
template<class T, size_t D>
class Rect
{
public:
  using Array = std::array<DimRange<T>, D>;

  /// Construct an empty rectangle
  explicit constexpr Rect()
    : _ranges{}
  {
    for (size_t i = 0; i < D; ++i) {
      _ranges[i] = DimRange<T>{std::numeric_limits<T>::max(),
                               std::numeric_limits<T>::lowest()};
    }
  }

  /// Construct a rectangle from a single point
  explicit constexpr Rect(const Point<T, D>& point)
  {
    for (size_t i = 0; i < D; ++i) {
      _ranges[i] = DimRange<T>{point[i], point[i]};
    }
  }

  /// Construct a rectangle for the given ranges in each dimension
  template<class... Rest>
  explicit constexpr Rect(DimRange<T> r, Rest&&... ranges)
    : _ranges{{r, std::forward<Rest>(ranges)...}}
  {}

  /// Construct a rectangle for the given ranges in each dimension
  explicit constexpr Rect(Array ranges)
    : _ranges{std::move(ranges)}
  {}

  [[nodiscard]] static constexpr size_t size() { return D; }

  [[nodiscard]] constexpr const Array& array() const { return _ranges; }

  [[nodiscard]] constexpr const DimRange<T>& operator[](const size_t i) const
  {
    return _ranges[i];
  }

  [[nodiscard]] constexpr DimRange<T>& operator[](const size_t i)
  {
    return _ranges[i];
  }

private:
  Array _ranges;
};

template<class T, size_t D>
constexpr bool
operator==(const Rect<T, D>& lhs, const Rect<T, D>& rhs)
{
  for (size_t i = 0U; i < D; ++i) {
    if (!(lhs[i] == rhs[i])) {
      return false;
    }
  }

  return true;
}

template<class T, size_t D>
constexpr bool
operator!=(const Rect<T, D>& lhs, const Rect<T, D>& rhs)
{
  return !(lhs == rhs);
}

template<size_t dim, class T, size_t D>
constexpr const DimRange<T>&
get(const Rect<T, D>& rect)
{
  return rect.array()[dim];
}

template<size_t dim, class T, size_t D>
constexpr const DimRange<T>&
range(const Rect<T, D>& rect)
{
  return rect.array()[dim];
}

template<size_t dim, class T, size_t D>
constexpr T
span(const Rect<T, D>& rect)
{
  const auto& range = get<dim>(rect);
  return (range.second < range.first) ? 0 : range.second - range.first;
}

template<class T, size_t D>
inline std::ostream&
operator<<(std::ostream& os, const Rect<T, D>& rect)
{
  os << '[';
  for (size_t i = 0; i < D; ++i) {
    os << ((i > 0) ? ", " : "") << "[" << rect[i].lower << ", " << rect[i].upper
       << ']';
  }
  return os << ']';
}

} // namespace spaix::homox

#endif // SPAIX_HOMOX_RECT_HPP
