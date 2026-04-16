// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include <spaix_test/TestRect.hpp>
#include <spaix_test/check.hpp>

#include <spaix/expansion.hpp>

namespace spaix::test {
namespace {

int
test_expansion()
{
  // No expansion
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestPoint{1_xc, 2.0_yc}) == 0));
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}}) == 0));

  // Expansion without volume increase
  STATIC_CHECK((expansion(TestRect{{1_xc, 1_xc}, {2.0_yc, 3.0_yc}},
                          TestPoint{1_xc, 5.0_yc}) == 2));
  STATIC_CHECK((expansion(TestRect{{2_xc, 3_xc}, {1.0_yc, 1.0_yc}},
                          TestPoint{5_xc, 1.0_yc}) == 2));

  // Unit expansion by point in each dimension/direction
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestPoint{0_xc, 2.0_yc}) == 1));
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestPoint{1_xc, 6.0_yc}) == 1));

  // Unit expansion by rect in each dimension/direction
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestRect{{0_xc, 3_xc}, {2.0_yc, 5.0_yc}}) == 1));
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestRect{{1_xc, 4_xc}, {2.0_yc, 5.0_yc}}) == 1));
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestRect{{1_xc, 3_xc}, {1.0_yc, 5.0_yc}}) == 1));
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestRect{{1_xc, 3_xc}, {2.0_yc, 6.0_yc}}) == 1));

  // Multi-dimensional expansion
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestRect{{1_xc, 3_xc}, {2.0_yc, 7.0_yc}}) == 2));
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestRect{{1_xc, 5_xc}, {2.0_yc, 7.0_yc}}) == 4));
  STATIC_CHECK((expansion(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}},
                          TestRect{{1_xc, 5_xc}, {2.0_yc, 8.0_yc}}) == 6));

  return 0;
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::test_expansion();
  return 0;
}
