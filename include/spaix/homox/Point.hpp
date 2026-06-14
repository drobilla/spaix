// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_HOMOX_POINT_HPP
#define SPAIX_HOMOX_POINT_HPP

#include <array>
#include <cstddef>
#include <iosfwd>
#include <utility>

namespace spaix::homox {

template<class T, size_t D>
class Point
{
public:
  using Array = std::array<T, D>;

  template<class... Ts>
  explicit constexpr Point(Ts... values)
    : _values{{std::move(values)...}}
  {}

  [[nodiscard]] constexpr size_t       size() const { return _values.size(); }
  [[nodiscard]] constexpr const Array& array() const { return _values; }

  [[nodiscard]] constexpr const T& operator[](const size_t i) const
  {
    return _values[i];
  }

private:
  Array _values;
};

template<class T, size_t D>
constexpr bool
operator==(const Point<T, D>& lhs, const Point<T, D>& rhs)
{
  for (size_t i = 0U; i < D; ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }

  return true;
}

template<class T, size_t D>
constexpr bool
operator!=(const Point<T, D>& lhs, const Point<T, D>& rhs)
{
  return !operator==(lhs, rhs);
}

template<size_t dim, class T, size_t D>
constexpr const T&
get(const Point<T, D>& point)
{
  return point[dim];
}

template<class CharT, class Traits, class T, size_t D>
inline std::ostream&
operator<<(std::basic_ostream<CharT, Traits>& os, const Point<T, D>& point)
{
  os << '[';
  for (size_t i = 0; i < D; ++i) {
    os << ((i > 0) ? ", " : "") << point[i];
  }
  return os << ']';
}

} // namespace spaix::homox

#endif // SPAIX_HOMOX_POINT_HPP
