// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TEST_HETERO2_HPP
#define SPAIX_TEST_HETERO2_HPP

#include "Scalar.hpp" // IWYU pragma: export

#include <spaix/heterox/Operations.hpp>
#include <spaix/heterox/Point.hpp> // IWYU pragma: export
#include <spaix/heterox/Queries.hpp>
#include <spaix/heterox/Rect.hpp> // IWYU pragma: export
#include <spaix/types.hpp>

// IWYU pragma: no_include "spaix/heterox/Point.hpp"
// IWYU pragma: no_include "spaix/heterox/Rect.hpp"
// IWYU pragma: no_include "spaix_test/Scalar.hpp"

#include <type_traits>

namespace spaix::test::hetero2 {

using XCoord = Scalar<struct X, int>;
using YCoord = Scalar<struct Y, float>;

using XRange = DimRange<XCoord>;
using YRange = DimRange<YCoord>;

using Rect  = heterox::Rect<XCoord, YCoord>;
using Point = heterox::Point<XCoord, YCoord>;

using Ops     = heterox::Operations<XCoord, YCoord>;
using Queries = heterox::Queries<XCoord, YCoord>;

} // namespace spaix::test::hetero2

namespace std {

template<>
struct common_type<spaix::test::hetero2::XCoord, spaix::test::hetero2::YCoord> {
  using type = float;
};

} // namespace std

#endif // SPAIX_TEST_HETERO2_HPP
