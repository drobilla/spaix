// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include <spaix_test/Scalar.hpp>
#include <spaix_test/TestRect.hpp>
#include <spaix_test/check.hpp>

#include <spaix/Rect.hpp>
#include <spaix/types.hpp>

#include <limits>
#include <sstream>
#include <string>
#include <tuple>

namespace spaix::test {
namespace {

void
test_rect()
{
  constexpr auto rect = make_rect(XRange{1_xc, 3_xc}, YRange{2.0_yc, 5.0_yc});

  STATIC_CHECK(
    (Rect<int, int>{} ==
     Rect<int, int>{
       {std::numeric_limits<int>::max(), std::numeric_limits<int>::lowest()},
       {std::numeric_limits<int>::max(), std::numeric_limits<int>::lowest()}}));

  STATIC_CHECK((Rect<float, float>{} ==
                Rect<float, float>{{std::numeric_limits<float>::max(),
                                    std::numeric_limits<float>::lowest()},
                                   {std::numeric_limits<float>::max(),
                                    std::numeric_limits<float>::lowest()}}));

  STATIC_CHECK((TestRect{TestPoint{1_xc, 2.0_yc}} ==
                TestRect{{1_xc, 1_xc}, {2.0_yc, 2.0_yc}}));

  STATIC_CHECK((TestRect{std::make_tuple(XRange{1_xc, 3_xc},
                                         YRange{2.0_yc, 5.0_yc})} == rect));

  // Comparison
  STATIC_CHECK((rect == make_rect(XRange{1_xc, 3_xc}, YRange{2.0_yc, 5.0_yc})));
  STATIC_CHECK((rect != make_rect(XRange{2_xc, 3_xc}, YRange{2.0_yc, 5.0_yc})));
  STATIC_CHECK((rect != make_rect(XRange{1_xc, 3_xc}, YRange{4.0_yc, 5.0_yc})));

  // Basic access
  STATIC_CHECK((ranges(rect) ==
                std::make_tuple(XRange{1_xc, 3_xc}, YRange{2.0_yc, 5.0_yc})));
  STATIC_CHECK((rect.size() == 2));
  STATIC_CHECK((get<0>(rect) == XRange{1_xc, 3_xc}));
  STATIC_CHECK((get<1>(rect) == YRange{2.0_yc, 5.0_yc}));
  STATIC_CHECK((range<0>(rect) == XRange{1_xc, 3_xc}));
  STATIC_CHECK((range<1>(rect) == YRange{2.0_yc, 5.0_yc}));
  STATIC_CHECK((span<0>(rect) == 2));
  STATIC_CHECK((span<1>(rect) == 3.0f));

  // Empty ranges
  STATIC_CHECK(
    (span<0>(make_rect(XRange{2_xc, 1_xc}, YRange{1.0_yc, 2.0_yc})) == 0));

  // Printing
  std::ostringstream ss;
  ss << make_rect(make_dim_range(1, 2), make_dim_range(3, 4));
  CHECK((ss.str() == "[[1, 2], [3, 4]]"));
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::test_rect();
  return 0;
}
