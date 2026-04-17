// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include <spaix_test/TestRect.hpp>
#include <spaix_test/check.hpp>

#include <spaix/Queries.hpp>

namespace spaix::test {
namespace {

template<typename Queries, typename TestRect, typename TestPoint>
constexpr void
test_intersects()
{
  auto intersects = [](const auto& l, const auto& r) {
    return Queries::intersects(l, r);
  };

  constexpr auto rect = TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}};

  STATIC_CHECK((intersects(rect, TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}})));
  STATIC_CHECK((intersects(rect, TestRect{{0_xc, 3_xc}, {2.0_yc, 5.0_yc}})));
  STATIC_CHECK((intersects(rect, TestRect{{1_xc, 4_xc}, {2.0_yc, 5.0_yc}})));
  STATIC_CHECK((intersects(rect, TestRect{{1_xc, 3_xc}, {1.0_yc, 5.0_yc}})));
  STATIC_CHECK((intersects(rect, TestRect{{1_xc, 3_xc}, {2.0_yc, 6.0_yc}})));

  STATIC_CHECK((!intersects(rect, TestRect{{0_xc, 0_xc}, {2.0_yc, 5.0_yc}})));
  STATIC_CHECK((!intersects(rect, TestRect{{4_xc, 5_xc}, {2.0_yc, 5.0_yc}})));
  STATIC_CHECK((!intersects(rect, TestRect{{1_xc, 3_xc}, {0.0_yc, 1.0_yc}})));
  STATIC_CHECK((!intersects(rect, TestRect{{1_xc, 3_xc}, {6.0_yc, 7.0_yc}})));

  STATIC_CHECK((intersects(rect, TestPoint{1_xc, 2.0_yc})));
  STATIC_CHECK((intersects(rect, TestPoint{2_xc, 5.0_yc})));
  STATIC_CHECK((!intersects(rect, TestPoint{0_xc, 2.0_yc})));
  STATIC_CHECK((!intersects(rect, TestPoint{1_xc, 0.0_yc})));
}

constexpr void
run()
{
  test_intersects<Queries<XCoord, YCoord>, TestRect, TestPoint>();
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::run();
  return 0;
}
