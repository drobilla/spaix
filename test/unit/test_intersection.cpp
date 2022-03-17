// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include "spaix_test/Scalar.hpp"
#include "spaix_test/TestRect.hpp"
#include "spaix_test/check.hpp"

#include "spaix/Rect.hpp"
#include "spaix/intersection.hpp"

namespace spaix::test {

static void
test_intersection()
{
  constexpr auto rect = TestRect{{1, 3}, {2.0f, 5.0f}};

  STATIC_CHECK(((rect & TestRect{{2, 4}, {1.0f, 4.0f}}) ==
                TestRect{{2, 3}, {2.0f, 4.0f}}));
  STATIC_CHECK(((rect & TestRect{{5, 6}, {1.0f, 4.0f}}) == TestRect{}));

  STATIC_CHECK(((rect & TestPoint{1, 2.0f}) == TestRect{{1, 1}, {2.0f, 2.0f}}));
  STATIC_CHECK(((TestPoint{1, 2.0f} & rect) == TestRect{{1, 1}, {2.0f, 2.0f}}));

  STATIC_CHECK(((TestPoint{1, 2.0f} & TestPoint{1, 2.0f}) ==
                TestRect{{1, 1}, {2.0f, 2.0f}}));

  STATIC_CHECK(((TestPoint{1, 2.0f} & TestPoint{1, 3.0f}) == TestRect{}));
}

} // namespace spaix::test

int
main()
{
  spaix::test::test_intersection();
  return 0;
}
