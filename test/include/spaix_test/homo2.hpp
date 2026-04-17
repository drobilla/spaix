// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TEST_HOMO2_HPP
#define SPAIX_TEST_HOMO2_HPP

#include <spaix/homox/Operations.hpp>
#include <spaix/homox/Point.hpp> // IWYU pragma: export
#include <spaix/homox/Queries.hpp>
#include <spaix/homox/Rect.hpp> // IWYU pragma: export
#include <spaix/types.hpp>

// IWYU pragma: no_include "spaix/homox/Point.hpp"
// IWYU pragma: no_include "spaix/homox/Rect.hpp"

namespace spaix::test::homo2 {

using Coord = double;

using Range = DimRange<Coord>;

class Point : public homox::Point<Coord, 2U>
{
public:
  template<typename T>
  explicit constexpr Point(const T x, const T y) noexcept
    : homox::Point<Coord, 2U>{static_cast<double>(x), static_cast<double>(y)}
  {}
};

class Rect : public homox::Rect<Coord, 2U>
{
public:
  constexpr Rect() noexcept = default;

  template<typename T>
  explicit constexpr Rect(const T left,
                          const T right,
                          const T top,
                          const T bottom) noexcept
    : homox::Rect<Coord, 2U>{
        Range{static_cast<double>(left), static_cast<double>(right)},
        Range{static_cast<double>(top), static_cast<double>(bottom)}}
  {}

  explicit constexpr Rect(const Range x, const Range y) noexcept
    : homox::Rect<Coord, 2U>{x, y}
  {}

  explicit constexpr Rect(const Point& point) noexcept
    : homox::Rect<Coord, 2U>{
        Range{static_cast<double>(point[0]), static_cast<double>(point[0])},
        Range{static_cast<double>(point[1]), static_cast<double>(point[1])}}
  {}
};

using Ops     = homox::Operations<double, 2U>;
using Queries = homox::Queries<double, 2U>;

} // namespace spaix::test::homo2

#endif // SPAIX_TEST_HOMO2_HPP
