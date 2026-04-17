// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include <spaix_test/Scalar.hpp>
#include <spaix_test/TestRect.hpp>
#include <spaix_test/check.hpp>

#include <spaix/Operations.hpp>
#include <spaix/Rect.hpp>
#include <spaix/types.hpp>

namespace spaix::test {
namespace {

template<typename Ops, typename Rect, typename Point>
constexpr void
test_unify()
{
  using OpRect = decltype(Rect{{0_xc, 0_xc}, {0.0_yc, 0.0_yc}});

  constexpr auto rect = OpRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}};

  CHECK((Ops::unify(rect, OpRect{}) == rect));

  CHECK((Ops::unify(rect, Rect{{2_xc, 4_xc}, {1.0_yc, 5.0_yc}}) ==
         Rect{{1_xc, 4_xc}, {1.0_yc, 5.0_yc}}));

  CHECK((Ops::unify(rect, Point{0_xc, 2.0_yc}) ==
         Rect{{0_xc, 3_xc}, {2.0_yc, 5.0_yc}}));
  CHECK((Ops::unify(rect, Point{4_xc, 2.0_yc}) ==
         Rect{{1_xc, 4_xc}, {2.0_yc, 5.0_yc}}));
  CHECK((Ops::unify(rect, Point{1_xc, 1.0_yc}) ==
         Rect{{1_xc, 3_xc}, {1.0_yc, 5.0_yc}}));
  CHECK((Ops::unify(rect, Point{1_xc, 6.0_yc}) ==
         Rect{{1_xc, 3_xc}, {2.0_yc, 6.0_yc}}));

  auto mut = Rect{{1_xc, 3_xc}, {1.0_yc, 5.0_yc}};
  Ops::expand(mut, Rect{{2_xc, 4_xc}, {2.0_yc, 6.0_yc}});
  CHECK((mut == Rect{{1_xc, 4_xc}, {1.0_yc, 6.0_yc}}));
}

constexpr void
run()
{
  test_unify<Operations<XCoord, YCoord>, TestRect, TestPoint>();
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::run();
  return 0;
}
