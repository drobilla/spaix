// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#include <spaix_test/hetero2.hpp>
#include <spaix_test/homo2.hpp>

#undef NDEBUG

#include <spaix_test/check.hpp>

namespace spaix::test {
namespace {

template<typename Comparisons, typename TestRect, typename TestPoint>
constexpr void
test_intersects()
{
  auto intersects = [](const auto& l, const auto& r) {
    return Comparisons::intersects(l, r);
  };

  constexpr auto rect = TestRect{{1, 3}, {2, 5}};

  STATIC_CHECK((intersects(rect, TestRect{{1, 3}, {2, 5}})));
  STATIC_CHECK((intersects(rect, TestRect{{0, 3}, {2, 5}})));
  STATIC_CHECK((intersects(rect, TestRect{{1, 4}, {2, 5}})));
  STATIC_CHECK((intersects(rect, TestRect{{1, 3}, {1, 5}})));
  STATIC_CHECK((intersects(rect, TestRect{{1, 3}, {2, 6}})));

  STATIC_CHECK((!intersects(rect, TestRect{{0, 0}, {2, 5}})));
  STATIC_CHECK((!intersects(rect, TestRect{{4, 5}, {2, 5}})));
  STATIC_CHECK((!intersects(rect, TestRect{{1, 3}, {0, 1}})));
  STATIC_CHECK((!intersects(rect, TestRect{{1, 3}, {6, 7}})));

  STATIC_CHECK((intersects(rect, TestPoint{1, 2})));
  STATIC_CHECK((intersects(rect, TestPoint{2, 5})));
  STATIC_CHECK((!intersects(rect, TestPoint{0, 2})));
  STATIC_CHECK((!intersects(rect, TestPoint{1, 0})));
}

constexpr void
run()
{
  test_intersects<hetero2::Comparisons, hetero2::Rect, hetero2::Point>();
  test_intersects<homo2::Comparisons, homo2::Rect, homo2::Point>();
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::run();
  return 0;
}
