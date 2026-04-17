// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include <spaix_test/check.hpp>
#include <spaix_test/hetero2.hpp>

#include <limits>
#include <sstream>
#include <string>

namespace spaix::test {
namespace {

constexpr void
test_empty()
{
  using heterox::Rect;

  STATIC_CHECK(
    (Rect<int, int>{} ==
     Rect<int, int>{
       {std::numeric_limits<int>::max(), std::numeric_limits<int>::lowest()},
       {std::numeric_limits<int>::max(), std::numeric_limits<int>::lowest()}}));

  STATIC_CHECK((Rect<float, float>{} ==
                Rect<float, float>{{std::numeric_limits<float>::max(),
                                    std::numeric_limits<float>::lowest()},
                                   {std::numeric_limits<float>::max(),
                                    std::numeric_limits<float>::lowest()}}));
}

template<typename Ops, typename OpRect, typename OpPoint>
void
test_rect()
{
  constexpr auto rect = OpRect{{1, 3}, {2, 5}};

  STATIC_CHECK((OpRect{OpPoint{1, 2}} == OpRect{{1, 1}, {2, 2}}));

  // Comparison
  STATIC_CHECK((rect == OpRect{{1, 3}, {2, 5}}));
  STATIC_CHECK((rect != OpRect{{2, 3}, {2, 5}}));
  STATIC_CHECK((rect != OpRect{{1, 3}, {4, 5}}));

  // Basic access
  STATIC_CHECK((rect.size() == 2));
  STATIC_CHECK((Ops::template lower<0>(rect) == 1));
  STATIC_CHECK((Ops::template upper<0>(rect) == 3));
  STATIC_CHECK((Ops::template lower<1>(rect) == 2));
  STATIC_CHECK((Ops::template upper<1>(rect) == 5));

  // Printing
  std::ostringstream ss;
  ss << rect;
  CHECK((ss.str() == "[[1, 3], [2, 5]]"));
}

void
run()
{
  test_empty();
  test_rect<hetero2::Ops, hetero2::Rect, hetero2::Point>();
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::run();
  return 0;
}
