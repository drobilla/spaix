// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include "Scalar.hpp"
#include "TestRect.hpp"
#include "check.hpp"

#include "spaix/volume.hpp"

namespace spaix {
namespace test {

static void
test_volume()
{
  STATIC_CHECK((volume(TestRect{{1, 3}, {2, 5}}) == 6));
  STATIC_CHECK((volume(TestRect{{1, 1}, {2, 5}}) == 0));
  STATIC_CHECK((volume(TestRect{{1, 3}, {2, 2}}) == 0));
}

} // namespace test
} // namespace spaix

int
main()
{
  spaix::test::test_volume();
  return 0;
}
