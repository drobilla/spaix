// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include "spaix_test/TestRect.hpp"
#include "spaix_test/check.hpp"

#include "spaix/Point.hpp"
#include "spaix/types.hpp"

#include <sstream>
#include <string>
#include <tuple>

namespace spaix::test {
namespace {

void
test_point()
{
  constexpr auto point = make_point(XCoord{1}, YCoord{2.0f});

  STATIC_CHECK((TestPoint{std::make_tuple(XCoord{1}, YCoord{2.0f})} == point));

  // Comparison
  STATIC_CHECK((point == TestPoint{1_xc, 2.0_yc}));
  STATIC_CHECK((point != TestPoint{2_xc, 2.0_yc}));
  STATIC_CHECK((point != TestPoint{1_xc, 3.0_yc}));

  // Basic access
  STATIC_CHECK((ranges(point) ==
                std::make_tuple(XRange{1_xc, 1_xc}, YRange{2.0_yc, 2.0_yc})));
  STATIC_CHECK((point.size() == 2));
  STATIC_CHECK((get<0>(point) == 1_xc));
  STATIC_CHECK((get<1>(point) == 2.0_yc));
  STATIC_CHECK((range<0>(point) == XRange{1_xc, 1_xc}));
  STATIC_CHECK((range<1>(point) == YRange{2.0_yc, 2.0_yc}));
  STATIC_CHECK((span<0>(point).value() == 0));
  STATIC_CHECK((span<1>(point).value() == 0.0f));

  std::ostringstream ss;
  ss << make_point(1, 2);
  CHECK((ss.str() == "[1, 2]"));
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::test_point();

  return 0;
}
