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
test_contains()
{
  auto contains = [](const auto& l, const auto& r) {
    return Queries::contains(l, r);
  };

  constexpr auto rect = TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}};

  STATIC_CHECK((contains(rect, rect)));
  STATIC_CHECK((!contains(rect, TestRect{{0_xc, 3_xc}, {2.0_yc, 5.0_yc}})));
  STATIC_CHECK((!contains(rect, TestRect{{1_xc, 4_xc}, {2.0_yc, 5.0_yc}})));
  STATIC_CHECK((!contains(rect, TestRect{{1_xc, 3_xc}, {1.0_yc, 5.0_yc}})));
  STATIC_CHECK((!contains(rect, TestRect{{1_xc, 3_xc}, {2.0_yc, 6.0_yc}})));

  STATIC_CHECK((contains(rect, TestPoint{1_xc, 2.0_yc})));
  STATIC_CHECK((contains(rect, TestPoint{2_xc, 5.0_yc})));
  STATIC_CHECK((!contains(rect, TestPoint{0_xc, 2.0_yc})));
  STATIC_CHECK((!contains(rect, TestPoint{1_xc, 0.0_yc})));
}

constexpr void
run()
{
  test_contains<Queries<XCoord, YCoord>, TestRect, TestPoint>();
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::run();
  return 0;
}
