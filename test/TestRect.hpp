// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef TEST_TESTRECT_HPP
#define TEST_TESTRECT_HPP

#include "Scalar.hpp"

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/types.hpp"

namespace spaix {
namespace test {

using XCoord = Scalar<struct X, int>;
using YCoord = Scalar<struct Y, float>;

using XRange = Range<XCoord>;
using YRange = Range<YCoord>;

using TestRect  = Rect<XCoord, YCoord>;
using TestPoint = Point<XCoord, YCoord>;

constexpr auto
operator*(const XCoord& lhs, const YCoord& rhs)
{
  return lhs.value() * rhs.value();
}

} // namespace test
} // namespace spaix

#endif // TEST_TESTRECT_HPP
