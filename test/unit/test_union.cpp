// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include <spaix_test/TestRect.hpp>
#include <spaix_test/check.hpp>

#include <spaix/Rect.hpp>
#include <spaix/union.hpp>

namespace spaix::test {
namespace {

void
test_union()
{
  constexpr auto rect = TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}};

  STATIC_CHECK(((rect | TestRect{}) == rect));

  STATIC_CHECK(((rect | TestRect{{2_xc, 4_xc}, {1.0_yc, 5.0_yc}}) ==
                TestRect{{1_xc, 4_xc}, {1.0_yc, 5.0_yc}}));

  STATIC_CHECK(((rect | TestPoint{0_xc, 2.0_yc}) ==
                TestRect{{0_xc, 3_xc}, {2.0_yc, 5.0_yc}}));
  STATIC_CHECK(((rect | TestPoint{4_xc, 2.0_yc}) ==
                TestRect{{1_xc, 4_xc}, {2.0_yc, 5.0_yc}}));
  STATIC_CHECK(((rect | TestPoint{1_xc, 1.0_yc}) ==
                TestRect{{1_xc, 3_xc}, {1.0_yc, 5.0_yc}}));
  STATIC_CHECK(((rect | TestPoint{1_xc, 6.0_yc}) ==
                TestRect{{1_xc, 3_xc}, {2.0_yc, 6.0_yc}}));

  STATIC_CHECK(((TestPoint{0_xc, 2.0_yc} | rect) ==
                TestRect{{0_xc, 3_xc}, {2.0_yc, 5.0_yc}}));
  STATIC_CHECK(((TestPoint{4_xc, 2.0_yc} | rect) ==
                TestRect{{1_xc, 4_xc}, {2.0_yc, 5.0_yc}}));
  STATIC_CHECK(((TestPoint{1_xc, 1.0_yc} | rect) ==
                TestRect{{1_xc, 3_xc}, {1.0_yc, 5.0_yc}}));
  STATIC_CHECK(((TestPoint{1_xc, 6.0_yc} | rect) ==
                TestRect{{1_xc, 3_xc}, {2.0_yc, 6.0_yc}}));

  STATIC_CHECK(((TestPoint{1_xc, 2.0_yc} | TestPoint{3_xc, 5.0_yc}) == rect));

  auto mut = TestRect{{1_xc, 3_xc}, {1.0_yc, 5.0_yc}};
  mut |= TestRect{{2_xc, 4_xc}, {2.0_yc, 6.0_yc}};
  CHECK((mut == TestRect{{1_xc, 4_xc}, {1.0_yc, 6.0_yc}}));
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::test_union();
  return 0;
}
