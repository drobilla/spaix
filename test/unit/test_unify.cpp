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
  using OpRect = decltype(Rect{{0, 0}, {0, 0}});

  constexpr auto rect = OpRect{{1, 3}, {2, 5}};

  CHECK((Ops::unify(rect, OpRect{}) == rect));

  CHECK((Ops::unify(rect, Rect{{2, 4}, {1, 5}}) == Rect{{1, 4}, {1, 5}}));

  CHECK((Ops::unify(rect, Point{0, 2}) == Rect{{0, 3}, {2, 5}}));
  CHECK((Ops::unify(rect, Point{4, 2}) == Rect{{1, 4}, {2, 5}}));
  CHECK((Ops::unify(rect, Point{1, 1}) == Rect{{1, 3}, {1, 5}}));
  CHECK((Ops::unify(rect, Point{1, 6}) == Rect{{1, 3}, {2, 6}}));

  auto mut = Rect{{1, 3}, {1, 5}};
  Ops::expand(mut, Rect{{2, 4}, {2, 6}});
  CHECK((mut == Rect{{1, 4}, {1, 6}}));
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
