// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TEST_TESTRECT_HPP
#define SPAIX_TEST_TESTRECT_HPP

#include <spaix_test/Scalar.hpp>

#include <spaix/heterox/Point.hpp>
#include <spaix/heterox/Rect.hpp>
#include <spaix/types.hpp>

namespace spaix::test {

using XCoord = Scalar<struct X, int>;
using YCoord = Scalar<struct Y, float>;

using XRange = DimRange<XCoord>;
using YRange = DimRange<YCoord>;

using TestRect  = heterox::Rect<XCoord, YCoord>;
using TestPoint = heterox::Point<XCoord, YCoord>;

} // namespace spaix::test

#endif // SPAIX_TEST_TESTRECT_HPP
