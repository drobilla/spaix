// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include "TestRect.hpp"
#include "check.hpp"

#include "spaix/intersects.hpp"

namespace spaix::test {

static void
test_intersects()
{
  constexpr auto rect = TestRect{{1, 3}, {2.0f, 5.0f}};

  STATIC_CHECK((intersects(rect, TestRect{{1, 3}, {2.0f, 5.0f}})));
  STATIC_CHECK((intersects(rect, TestRect{{0, 3}, {2.0f, 5.0f}})));
  STATIC_CHECK((intersects(rect, TestRect{{1, 4}, {2.0f, 5.0f}})));
  STATIC_CHECK((intersects(rect, TestRect{{1, 3}, {1.0f, 5.0f}})));
  STATIC_CHECK((intersects(rect, TestRect{{1, 3}, {2.0f, 6.0f}})));

  STATIC_CHECK((!intersects(rect, TestRect{{0, 0}, {2.0f, 5.0f}})));
  STATIC_CHECK((!intersects(rect, TestRect{{4, 5}, {2.0f, 5.0f}})));
  STATIC_CHECK((!intersects(rect, TestRect{{1, 3}, {0.0f, 1.0f}})));
  STATIC_CHECK((!intersects(rect, TestRect{{1, 3}, {6.0f, 7.0f}})));

  STATIC_CHECK((intersects(rect, TestPoint{1, 2.0f})));
  STATIC_CHECK((intersects(rect, TestPoint{2, 5.0f})));
  STATIC_CHECK((!intersects(rect, TestPoint{0, 2.0f})));
  STATIC_CHECK((!intersects(rect, TestPoint{1, 0.0f})));

  STATIC_CHECK((intersects(TestPoint{1, 2.0f}, rect)));
  STATIC_CHECK((intersects(TestPoint{2, 5.0f}, rect)));
  STATIC_CHECK((!intersects(TestPoint{0, 2.0f}, rect)));
  STATIC_CHECK((!intersects(TestPoint{1, 0.0f}, rect)));
}

} // namespace spaix::test

int
main()
{
  spaix::test::test_intersects();
  return 0;
}
