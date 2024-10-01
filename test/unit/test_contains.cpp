// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include "spaix_test/TestRect.hpp"
#include "spaix_test/check.hpp"

#include "spaix/contains.hpp"

namespace spaix::test {
namespace {

void
test_contains()
{
  constexpr auto rect = TestRect{{1, 3}, {2.0f, 5.0f}};

  STATIC_CHECK((contains(rect, rect)));
  STATIC_CHECK((!contains(rect, TestRect{{0, 3}, {2.0f, 5.0f}})));
  STATIC_CHECK((!contains(rect, TestRect{{1, 4}, {2.0f, 5.0f}})));
  STATIC_CHECK((!contains(rect, TestRect{{1, 3}, {1.0f, 5.0f}})));
  STATIC_CHECK((!contains(rect, TestRect{{1, 3}, {2.0f, 6.0f}})));

  STATIC_CHECK((contains(rect, TestPoint{1, 2.0f})));
  STATIC_CHECK((contains(rect, TestPoint{2, 5.0f})));
  STATIC_CHECK((!contains(rect, TestPoint{0, 2.0f})));
  STATIC_CHECK((!contains(rect, TestPoint{1, 0.0f})));

  STATIC_CHECK((!contains(TestPoint{1, 2.0f}, rect)));
  STATIC_CHECK((!contains(TestPoint{1, 2.0f}, TestPoint{2, 5.0f})));
  STATIC_CHECK((contains(TestPoint{1, 2.0f}, TestPoint{1, 2.0f})));
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::test_contains();
  return 0;
}
