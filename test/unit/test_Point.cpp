// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#include <spaix_test/hetero2.hpp>
#include <spaix_test/homo2.hpp>

#undef NDEBUG

#include <spaix_test/check.hpp>

#include <sstream>
#include <string>

namespace spaix::test {
namespace {

template<typename Ops, typename TestPoint>
void
test_point()
{
  constexpr auto point = TestPoint{1, 2};

  STATIC_CHECK(((TestPoint{1, 2}) == point));

  // Comparison
  STATIC_CHECK((point == TestPoint{1, 2}));
  STATIC_CHECK((point != TestPoint{2, 2}));
  STATIC_CHECK((point != TestPoint{1, 3}));

  // Basic access
  STATIC_CHECK((point.size() == 2));
  STATIC_CHECK((Ops::template lower<0>(point) == 1));
  STATIC_CHECK((Ops::template upper<0>(point) == 1));
  STATIC_CHECK((Ops::template lower<1>(point) == 2));
  STATIC_CHECK((Ops::template upper<1>(point) == 2));

  STATIC_CHECK((Ops::volume(point) == 0));

  std::ostringstream ss;
  ss << point;
  CHECK((ss.str() == "[1, 2]"));
}

void
run()
{
  test_point<hetero2::Ops, hetero2::Point>();
  test_point<homo2::Ops, homo2::Point>();
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::run();
  return 0;
}
