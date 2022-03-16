// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TEST_TESTRECT_HPP
#define SPAIX_TEST_TESTRECT_HPP

#include "Scalar.hpp"

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/types.hpp"

namespace spaix::test {

using XCoord = Scalar<struct X, int>;
using YCoord = Scalar<struct Y, float>;

using XRange = DimRange<XCoord>;
using YRange = DimRange<YCoord>;

using TestRect  = Rect<XCoord, YCoord>;
using TestPoint = Point<XCoord, YCoord>;

constexpr auto
operator*(const XCoord& lhs, const YCoord& rhs)
{
#ifdef _MSC_VER
  __pragma(warning(push, 0))
  __pragma(warning(disable : 5219))
#endif

  return lhs.value() * rhs.value(); // NOLINT

#ifdef _MSC_VER
  __pragma(warning(pop));
#endif
}

} // namespace spaix::test

#endif // SPAIX_TEST_TESTRECT_HPP
