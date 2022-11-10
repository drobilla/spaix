// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include "spaix_test/TestRect.hpp"
#include "spaix_test/check.hpp"

#include "spaix/expansion.hpp"

namespace spaix::test {
namespace {

int
test_expansion()
{
  STATIC_CHECK((expansion(TestRect{{1, 3}, {2, 5}}, TestPoint{1, 2}) == 0));
  STATIC_CHECK((expansion(TestRect{{1, 3}, {2, 5}}, TestPoint{0, 2}) == 1));
  STATIC_CHECK((expansion(TestRect{{1, 3}, {2, 5}}, TestPoint{1, 6}) == 1));

  STATIC_CHECK(
    (expansion(TestRect{{1, 3}, {2, 5}}, TestRect{{1, 3}, {2, 5}}) == 0));

  STATIC_CHECK(
    (expansion(TestRect{{1, 3}, {2, 5}}, TestRect{{0, 3}, {2, 5}}) == 1));
  STATIC_CHECK(
    (expansion(TestRect{{1, 3}, {2, 5}}, TestRect{{1, 4}, {2, 5}}) == 1));
  STATIC_CHECK(
    (expansion(TestRect{{1, 3}, {2, 5}}, TestRect{{1, 3}, {1, 5}}) == 1));
  STATIC_CHECK(
    (expansion(TestRect{{1, 3}, {2, 5}}, TestRect{{1, 3}, {2, 6}}) == 1));

  STATIC_CHECK(
    (expansion(TestRect{{1, 3}, {2, 5}}, TestRect{{1, 3}, {2, 7}}) == 2));
  STATIC_CHECK(
    (expansion(TestRect{{1, 3}, {2, 5}}, TestRect{{1, 5}, {2, 7}}) == 4));

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
