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

  constexpr auto rect = TestRect{{1, 3}, {2, 5}};

  STATIC_CHECK((contains(rect, rect)));
  STATIC_CHECK((!contains(rect, TestRect{{0, 3}, {2, 5}})));
  STATIC_CHECK((!contains(rect, TestRect{{1, 4}, {2, 5}})));
  STATIC_CHECK((!contains(rect, TestRect{{1, 3}, {1, 5}})));
  STATIC_CHECK((!contains(rect, TestRect{{1, 3}, {2, 6}})));

  STATIC_CHECK((contains(rect, TestPoint{1, 2})));
  STATIC_CHECK((contains(rect, TestPoint{2, 5})));
  STATIC_CHECK((!contains(rect, TestPoint{0, 2})));
  STATIC_CHECK((!contains(rect, TestPoint{1, 0})));
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
