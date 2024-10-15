// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TEST_TESTRECT_HPP
#define SPAIX_TEST_TESTRECT_HPP

#include "Scalar.hpp" // IWYU pragma: export

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

constexpr XCoord
operator""_xc(const unsigned long long value)
{
  return XCoord{static_cast<int>(value)};
}

// NOLINTNEXTLINE(google-runtime-int])
constexpr YCoord
operator""_yc(const long double value)
{
  return YCoord{static_cast<float>(value)};
}

} // namespace spaix::test

#endif // SPAIX_TEST_TESTRECT_HPP
